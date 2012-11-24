/* 
 * File:   Dispatcher.cpp
 * Author: try
 * 
 * Created on 2011年5月27日, 上午9:14
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <wait.h>

#include <iostream>
#include "Dispatcher.h"
#include "Config.h"
#include "HttpServer.h"
#include "CleanerTimer.h"
#include "SocketUtils.h"

using namespace std;

Dispatcher::Dispatcher():httpServer(NULL), evioSocket(NULL), config(NULL), processCount(0){

}


Dispatcher::~Dispatcher() {
    CleanerTimer::instance.remove(this);   
    cleanup();
}


bool Dispatcher::start(){
    CleanerTimer::instance.add(this);
    return createProcess(config->workProcessCount) == config->workProcessCount;
}

int Dispatcher::createProcess(int n){
    LOG_DEBUG("n:%d", n);
    int count = 0;
    for(int i=0; i<n; i++){
        HttpProcess* p = new HttpProcess(this);
        p->httpServer = httpServer;
        p->evioSocket = evioSocket;
        p->config = config;
        if(config->eventListener){
            p->addListener(config->eventListener); //添加事件监听器
        }
        if(!p->start()){
            delete p;
            break;
        }
        count++;
        allProcesses.push_back(p);
    }
    processCount = allProcesses.size();
    return count;
}


void Dispatcher::addToDestroyedProcesses(HttpProcess* p){
    LOG_DEBUG("allProcesses.size:%d", allProcesses.size());
    HttpProcessVector::iterator it = allProcesses.begin();
    for(; it != allProcesses.end(); it++){
        if((*it)->getId() == p->getId()){
            allProcesses.erase(it);
            break;
        }
    }    
    LOG_DEBUG("allProcesses.size:%d", allProcesses.size());
    
    destroyedProcesses.push_back(p);
}


void Dispatcher::cleanup(){
    LOG_DEBUG("destroyedProcesses.size:%d", allProcesses.size());
    HttpProcessVector::iterator it = destroyedProcesses.begin();
    for(; it != destroyedProcesses.end(); it++){
        HttpProcess* p =  *it;
        if(p->isAlive()){
            if(!p->isExiting()){
                LOG_WARN("kill process, pid:%d, p->isExiting():%d", p->getId(), p->isExiting());
            }
            p->close();
            kill(p->getId(), SIGKILL);
        }
        delete p;
    }
    destroyedProcesses.clear();
}


//@TODO 返回一个空闲进程，以后可以改为更快速的算法来查找空闲进程，当前这样用着
HttpProcess* Dispatcher::getIdleProcess(){
    if(allProcesses.size() < config->workProcessCount){
        createProcess(config->workProcessCount-allProcesses.size());
    }
    if(allProcesses.empty()){
        LOG_ERROR("have not idle process");
        return NULL;
    }
    return allProcesses[rand()%allProcesses.size()];
}

bool Dispatcher::dispatch(FD fd){
    HttpProcess* p = getIdleProcess();
    if(p){
        return p->addTask(fd);
    }
    ::close(fd);
    return false;
}

std::vector<HttpProcess*>& Dispatcher::getHttpProcesses(std::vector<HttpProcess*>& httpProcesses){
    HttpProcessVector::iterator it = allProcesses.begin();
    for(; it != allProcesses.end(); it++){
        httpProcesses.push_back(*it);
    }
    return httpProcesses;
}


void Dispatcher::sendMessage(MSG_TYPE type, void* msg, size_t len){
    HttpProcessVector::iterator it = allProcesses.begin();
    for(; it != allProcesses.end(); it++){
        (*it)->sendMessage(type, msg, len);
    }
}


/**
 * 通知所有子进程退出
 */    
void Dispatcher::notifyCloseProcesses(){
    LOG_DEBUG("allProcesses.size:%d", allProcesses.size());
    HttpProcessVector::iterator it = allProcesses.begin();
    for(; it != allProcesses.end(); it++){
        LOG_DEBUG("isAlive:%d", (*it)->isAlive());
        (*it)->notifyClose();
    }
}

/**
 * 强行关闭所有子进程 
 */
void Dispatcher::killProcesses(){
    LOG_DEBUG("allProcesses.size:%d", allProcesses.size());
    if(allProcesses.empty()){
        return;
    }
    HttpProcessVector::iterator it = allProcesses.begin();
    for(; it != allProcesses.end(); it++){
        HttpProcess* p =  *it;
        LOG_DEBUG("%d, isAlive:%d, isExit:%d", (*it)->getId(), p->isAlive(), p->isExit());
        if(p->isAlive()){
            if(!p->isExiting()){
                LOG_WARN("kill process, pid:%d", p->getId());
            }
            kill(p->getId(), SIGKILL);
            p->close();
        }
        delete p;
    }
    allProcesses.clear();
}

int Dispatcher::processesCount(){
    return allProcesses.size();
}

//清除从父进程中带来的资源
void Dispatcher::removeProcesses(){
    LOG_DEBUG("allProcesses.size:%d", allProcesses.size());
    if(allProcesses.empty()){
        return;
    }
    HttpProcessVector::iterator it = allProcesses.begin();
    for(; it != allProcesses.end(); it++){
        delete *it;
    }
    allProcesses.clear();
}
