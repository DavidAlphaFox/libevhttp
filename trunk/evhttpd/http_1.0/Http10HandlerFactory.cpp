/* 
 * File:   Http10HandlerFactory.cpp
 * Author: try
 * 
 * Created on 2011年6月2日, 上午9:37
 */

#include "Http10HandlerFactory.h"
#include "Http10Handler.h"

Http10HandlerFactory::Http10HandlerFactory() {
}


Http10HandlerFactory::~Http10HandlerFactory() {
}

HttpHandler* Http10HandlerFactory::create(){
    return new Http10Handler();
}

void Http10HandlerFactory::free(HttpHandler* handler){
    delete handler;
}

