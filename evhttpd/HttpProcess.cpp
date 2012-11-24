/* 
 * File:   HttpProcess.cpp
 * Author: try
 * 
 * Created on 2011年5月27日, 上午10:34
 */

#include <signal.h>
#include <fcntl.h>
#include <wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>

#include "events.h"
#include "HttpProcess.h"
#include "SocketUtils.h"
#include "Dispatcher.h"
#include "HttpServer.h"
#include <errno.h>
#include "HttpServletManager.h"
#include "HttpHandlerManager.h"
#include "CleanerTimer.h"
#include "SocketUtils.h"

using namespace std;


HttpProcess::HttpProcess(Dispatcher* dispatcher): 
        activeHandlerCount(0), totalRequestCount(0), httpHandlerCount(0),
        dispatcher(dispatcher), httpServer(NULL), evioSocket(NULL), config(NULL), 
        parentEvioStarted(false){
        
}


HttpProcess::~HttpProcess() {

}


void HttpProcess::executeClose(){
    LOG_DEBUG("childProcess:%d, getAllCount:%d, getActiveCount:%d", childProcess, HttpHandlerManager::instance.getAllCount(), HttpHandlerManager::instance.getActiveCount());
    HttpHandlerManager::instance.notifyCloseAll();
//    LOG_DEBUG("childProcess:%d, getAllCount:%d, getActiveCount:%d", childProcess, HttpHandlerManager::instance.getAllCount(), HttpHandlerManager::instance.getActiveCount());
    LOG_DEBUG("childProcess:%d, getAllCount:%d, getActiveCount:%d", childProcess, HttpHandlerManager::instance.getAllCount(), HttpHandlerManager::instance.getActiveCount());
    //如果所有Handler都关闭了，就退出进程
    if(HttpHandlerManager::instance.getAllCount() <= 0){
        CleanerTimer::instance.cleanupAll();
        this->close();
        exit(0);
    }
}

/**
 * 关闭相关资源
 */
void HttpProcess::notifyClose(){
    Process::notifyClose();
}

/**
 * 关闭相关资源
 */
void HttpProcess::close(){
    fdReadEvio.stop();
    if(childProcess){
        HttpHandlerManager::instance.notifyCloseAll();
    }
    Process::close();
}

bool HttpProcess::addTask(FD fd){
    bool retrs = sendFD(fd);
    if(!retrs){
        LOG_WARN("send FD error, fd is %d", fd);
//        SocketUtils::closeSocketFD(fd);
        ::close(fd);
    }
    return retrs;
}


//结束子进程
void HttpProcess::exitProcess(){
    LOG_DEBUG("childProcess:%d", childProcess);
    
    if(childProcess){
        this->close(); //关闭打开的资源
        ::exit(1);
    }else{//在父进程中
        LOG_DEBUG("isAlive():%d, isExit():%d", isAlive(), isExit());
        if(isAlive()){//如果子进程还是活动的, 向子进程发送退出事件
            kill(getId(), SIGTERM);
        }
        //将此进程加入到待销毁进程列表
        dispatcher->addToDestroyedProcesses(this); //将进程加入等销毁队列
        this->close();
    }
}

/**
 * 处理从父进程中读到的消息
 */
bool HttpProcess::handleMessageInChild(ProcessMessage_t* message){
    LOG_DEBUG("readsize:%d, command:%d", message->length, message->type);
    switch(message->type){
        case MSG_TYPE_QUERY_ACTIVE_HANDLERS:
            activeHandlerCount = HttpHandlerManager::instance.getActiveCount();
            return sendMessage(MSG_TYPE_QUERY_ACTIVE_HANDLERS, (char*)(&activeHandlerCount), INT_SIZE);
        case MSG_TYPE_QUERY_HTTP_HANDLERS:
            httpHandlerCount = HttpHandlerManager::instance.getAllCount();
            return sendMessage(MSG_TYPE_QUERY_HTTP_HANDLERS, (char*)(&httpHandlerCount), INT_SIZE);
        case MSG_TYPE_QUERY_TOTAL_REQUESTS:
            return sendMessage(MSG_TYPE_QUERY_TOTAL_REQUESTS, (char*)(&totalRequestCount), LONG_SIZE);
        case MSG_TYPE_EXIT:
            LOG_DEBUG("type:MSG_TYPE_EXIT, childProcess:%d, exiting:%d", childProcess, exiting);
            executeClose();
            return true;
            
    }
    return false;
}

/**
 * 处理从子进程中读到的消息
 */
bool HttpProcess::handleMessageInParent(ProcessMessage_t* message){
    LOG_DEBUG("readsize:%d, command:%d", message->length, message->type);

    switch(message->type){
        case MSG_TYPE_QUERY_ACTIVE_HANDLERS:
            if(message->length != INT_SIZE){
                LOG_WARN("MSG_TYPE_QUERY_ACTIVE_HANDLERS, getpid:%d, rsize:%d", getpid(), message->length)
            }else{
                activeHandlerCount = *((int*)message->data);
                return true;
            }
            break;
        case MSG_TYPE_QUERY_HTTP_HANDLERS:
            if(message->length != INT_SIZE){
                LOG_WARN("MSG_TYPE_QUERY_HTTP_HANDLERS, getpid:%d, rsize:%d", getpid(), message->length)
            }else{             
                httpHandlerCount = *((int*)message->data);
                return true;
            }
            break;
        case MSG_TYPE_QUERY_TOTAL_REQUESTS:
            if(message->length != LONG_SIZE){
                LOG_WARN("MSG_TYPE_QUERY_TOTAL_REQUESTS, getpid:%d, rsize:%d", getpid(), message->length)
            }else{              
                totalRequestCount = *((long*)message->data);
                return true;
            }
            break;
        case MSG_TYPE_EXIT:
            LOG_DEBUG("type:MSG_TYPE_EXIT, childProcess:%d, exiting:%d", childProcess, exiting);
            //子进程准备退出
            if(isExit()){//检查子进程如果已经退出了，就移除进程
                exitProcess();
            }
            return true;            
    }            
    return false;
}


bool HttpProcess::queryActiveHandlerCount(){
   return sendMessage(MSG_TYPE_QUERY_ACTIVE_HANDLERS);
}

bool HttpProcess::queryTotalRequestCount(){
    return sendMessage(MSG_TYPE_QUERY_TOTAL_REQUESTS);
}

bool HttpProcess::queryHttpHandlerCount(){
    return sendMessage(MSG_TYPE_QUERY_HTTP_HANDLERS);
}


void HttpProcess::fdReadCallback(ev::io &evio, int revents){
    LOG_DEBUG("evio.fd:%d", evio.fd);
    FD fd = -1; //在当前进程中的socket fd
    if(!recvFD(&fd)){
        if(!ERRNO_IS_AGAIN()){//errno != EAGAIN){
            LOG_WARN("recvFD error, ppid:%d, pid:%d, errno:%d", getppid(), getpid(), errno);
            exitProcess();
        }
        return;
    }
    LOG_DEBUG("recvFD:%d", fd)
    SocketUtils::setNonblock(fd);
    HttpHandler* handler = HttpHandlerManager::instance.get(this, fd);
    
    //取客户端地址信息
    if(getpeername(fd, (struct sockaddr *)&(handler->requestData.remoteAddr), &SOCKET_ADDR_LEN) != 0){
        //@TODO 写日志
        LOG_WARN("get remote ip addr error, fd:%d", fd);
        memset(&(handler->requestData.remoteAddr), 0, SOCKET_ADDR_LEN);
    }    
    handler->start();
    
}

int HttpProcess::run(void* param /* =NULL */){
    LOG_DEBUG("");
    //忽略一些会让程序中止的信号
    signal(SIGPIPE, SIG_IGN);
    
    //初始化
    HttpHandlerManager::init(config->handlerFactory, config->poolMaxHttpHandlerCount);
    HttpServletManager::init(httpServer->getHttpServletFactory());
    
    //使用libev框架监听Socket事件
    ev::loop_ref loop = ev::get_default_loop();
    loop.post_fork();
    
    LOG_DEBUG("dispatcher.processesCount():%d", dispatcher->processesCount());
    //清理从父进程带过来的资源
    httpServer->stop();
    dispatcher->removeProcesses();
    CleanerTimer::instance.remove(dispatcher);
//    dispatcher = NULL;
    
    //设置Socket模式，设置为非阻塞模式
//    SocketUtils::setNonblock(msgfd);
    SocketUtils::setNonblock(fdfd);

    //接收父进程发送过来的FD
    fdReadEvio.set<HttpProcess, &HttpProcess::fdReadCallback>(this);
    fdReadEvio.set(fdfd, EV_READ);
    fdReadEvio.start();
    
    loop.run(0);

    return 0;
}


    /**
     * 发送消息完成事件
     */
    bool HttpProcess::onSendMessageEvent(const ProcessSendMessageEvent* e){
        LOG_DEBUG("succeed:%d, childProcess:%d", e->succeed, e->childProcess);
        if(!e->succeed){
            exitProcess();
            return true;
        }
        return true;
    }
    
    /**
     * 接收消息完成事件
     */
    bool HttpProcess::onReceiveMessageEvent(const ProcessReceiveMessageEvent* e){
        LOG_DEBUG("succeed:%d, childProcess:%d", e->succeed, e->childProcess);
        if(!e->succeed){
            LOG_DEBUG("succeed:%d, childProcess:%d", e->succeed, e->childProcess);
            exitProcess();
            return true;
        }

        if(Process::onReceiveMessageEvent(e)){
            return true;
        }
        
        if(e->childProcess){
            return handleMessageInChild(e->message);
        }else{
            return handleMessageInParent(e->message);
        }
    }  

