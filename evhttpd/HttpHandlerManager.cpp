/* 
 * File:   HttpHandlerManager.cpp
 * Author: try
 * 
 * Created on 2011年7月14日, 上午10:47
 */

#include "HttpHandlerManager.h"
#include "CleanerTimer.h"

HttpHandlerManager HttpHandlerManager::instance;

HttpHandlerManager::HttpHandlerManager():httpHandlerFactory(&defaultHttpHandlerFactory),
        maxHttpHandlerCount(POOL_MAX_HTTP_HANDLER_COUNT) {
}

HttpHandlerManager::~HttpHandlerManager() {
    //CleanerTimer::instance.remove(&HttpHandlerManager::instance);
    cleanupAll();
}

void HttpHandlerManager::init(HttpHandlerFactory* factory, int maxHttpHandlerCount){
    if(factory){
        HttpHandlerManager::instance.httpHandlerFactory = factory;
    }
    HttpHandlerManager::instance.maxHttpHandlerCount = maxHttpHandlerCount;
    CleanerTimer::instance.add(&HttpHandlerManager::instance);
}


HttpHandler* HttpHandlerManager::get(HttpProcess* httpProcess, FD fd){
    HttpHandler* handler = NULL;
    HttpHandlerSet::iterator it = idleHttpHandlers.begin();
    if(it == idleHttpHandlers.end()){
        handler = httpHandlerFactory->create();
        allHttpHandlers.insert(handler);
    }else{
        handler = *it;
        idleHttpHandlers.erase(it);
    }
    handler->init(httpProcess, fd);
    return handler;
}



void HttpHandlerManager::recycle(HttpHandler* handler){
    //LOG_DEBUG("idleHttpHandlers.size():%d", idleHttpHandlers.size());
    if(idleHttpHandlers.find(handler) == idleHttpHandlers.end()){
        idleHttpHandlers.insert(handler);
    }
}

/** 强行关闭未结束的Handler */
void HttpHandlerManager::closeAll(){
    const HttpHandlerManager::HttpHandlerSet* allHandlers = HttpHandlerManager::instance.getAllHttpHandlers();
    HttpHandlerManager::HttpHandlerSet::iterator it = allHandlers->begin();
    for(; it != allHandlers->end(); it++){
        if(!((*it)->isClosed())){
            (*it)->close();
        }
    }
    cleanupAll();
}

void HttpHandlerManager::notifyCloseAll(){
    LOG_DEBUG("AllCount:%d",  HttpHandlerManager::instance.getAllCount());
    const HttpHandlerManager::HttpHandlerSet* allHandlers = HttpHandlerManager::instance.getAllHttpHandlers();
    HttpHandlerManager::HttpHandlerSet::iterator it = allHandlers->begin();
    for(; it != allHandlers->end(); it++){
        if(!((*it)->isClosed())){
            (*it)->notifyClose();
        }
    }
    cleanupAll();
}

void HttpHandlerManager::cleanupAll(){
    cleanup(0);
}

void HttpHandlerManager::cleanup(){
    cleanup(maxHttpHandlerCount);
}

void HttpHandlerManager::cleanup(int keepCount){
    //移除过多的空闲Handlers
    int excessCount = allHttpHandlers.size() - keepCount;
//    LOG_DEBUG("excessCount:%d, keepCount:%d, allHttpHandlers.size():%d, idleHttpHandlers.size():%d", excessCount, keepCount, allHttpHandlers.size(), idleHttpHandlers.size());
    for(HttpHandlerSet::iterator it = idleHttpHandlers.begin(); excessCount>0 && it != idleHttpHandlers.end(); excessCount--){
        HttpHandler* handler = *it;
        HttpHandlerSet::iterator ait = allHttpHandlers.find(handler);
        if(ait != allHttpHandlers.end()){
            allHttpHandlers.erase(ait);
        }
        idleHttpHandlers.erase(it++);
        httpHandlerFactory->free(handler);
    }
//    LOG_DEBUG("excessCount:%d, keepCount:%d, allHttpHandlers.size():%d, idleHttpHandlers.size():%d", excessCount, keepCount, allHttpHandlers.size(), idleHttpHandlers.size());
}



int HttpHandlerManager::getActiveCount(){
    return allHttpHandlers.size() - idleHttpHandlers.size();
}

int HttpHandlerManager::getAllCount(){
    return allHttpHandlers.size();
}


