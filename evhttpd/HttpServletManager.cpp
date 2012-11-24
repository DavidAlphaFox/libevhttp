/* 
 * File:   HttpServletManager.cpp
 * Author: try
 * 
 * Created on 2011年7月11日, 上午8:58
 */

#include <stdlib.h>
#include "HttpServletManager.h"
#include "HttpServletFactory.h"
#include "CleanerTimer.h"

HttpServletManager HttpServletManager::instance;

HttpServletManager::HttpServletManager():factory(NULL) {
}


HttpServletManager::~HttpServletManager() {
}

void HttpServletManager::init(HttpServletFactory* factory){
    HttpServletManager::instance.factory = factory;
    CleanerTimer::instance.add(&HttpServletManager::instance);
}
    
    
HttpServlet* HttpServletManager::create(const char* path){
    if(factory){
        return factory->create(path);
    }
    LOG_ERROR("factory is NULL");
    return NULL;
}


void HttpServletManager::recycle(HttpServlet* servlet){
    if(factory){
        freeQueue.push(new Entry(servlet, time(NULL)));
    }
}


void HttpServletManager::cleanup(){
    LOG_DEBUG("freeQueue.size():%d", freeQueue.size());
    time_t ct = time(NULL);
    while(!freeQueue.empty()){
         Entry* e = freeQueue.front();
         if(ct > e->t + HTTP_SERVLET_FREE_TIMEOUT){
             factory->free(e->servlet);
             freeQueue.pop();
             delete e;
         }else{
             break;
         }
    }
}



