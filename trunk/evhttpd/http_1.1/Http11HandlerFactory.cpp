/* 
 * File:   Http11HandlerFactory.cpp
 * Author: try
 * 
 * Created on 2011年6月3日, 上午11:31
 */

#include "Http11HandlerFactory.h"
#include "Http11Handler.h"

Http11HandlerFactory::Http11HandlerFactory() {
}

Http11HandlerFactory::~Http11HandlerFactory() {
}

HttpHandler* Http11HandlerFactory::create(){
    return new Http11Handler();
}


void Http11HandlerFactory::free(HttpHandler* handler){
    delete handler;
}

