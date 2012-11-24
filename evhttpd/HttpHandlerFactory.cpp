/* 
 * File:   HttpHandlerFactory.cpp
 * Author: try
 * 
 * Created on 2011年6月2日, 上午12:01
 */

#include "HttpHandlerFactory.h"
#include "HttpHandler.h"

HttpHandlerFactory::HttpHandlerFactory() {
}


HttpHandlerFactory::~HttpHandlerFactory() {
}


HttpHandler* HttpHandlerFactory::create(){
    return new HttpHandler();
}


void HttpHandlerFactory::free(HttpHandler* handler){
    delete handler;
}



