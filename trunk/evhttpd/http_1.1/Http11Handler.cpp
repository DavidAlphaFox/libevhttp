/* 
 * File:   Http11Handler.cpp
 * Author: try
 * 
 * Created on 2011年6月3日, 上午11:32
 */

#include "Http11Handler.h"
#include <istream>
#include <string.h>

using namespace std;

Http11Handler::Http11Handler() {
}

Http11Handler::~Http11Handler() {
}

bool Http11Handler::handleRequest(){
    if(!request.isHttp1_1){
        return Http10Handler::handleRequest();
    }
    
    if(Http10Handler::handleRequest()){
        //已经处理完请求
        return true;
    }
    
    if(request.isPut){
        if(!requestData.existReqBody){
            if(!prepareRequestBody()){
                return true;
            }
        }

        if(requestData.existReqBody){
            servlet->service(request, *response);
            if(!response->isAsynAnswerMode()){
                //回应客户端请求
                answer();
                return true;
            }
        }
    }
    
    
    return false;
}

//--事件------------------------------------------------------------------------

bool Http11Handler::onRequestLineEvent(){
    if(strcmp(requestData.protocol, HTTP_1_1)==0){
        request.isHttp1_1 = true;
    }else if(strcmp(request.protocol, HTTP_1_0)==0){
        request.isHttp1_0 = true;
    }

    //检测协议版本是否合法
    if(!(request.isHttp1_1 || request.isHttp1_0)){
        httpCode = HTTP_CODE_505;
        request.isHttp1_1 = true;
        return true;
    }

    //如果是Http 1.0请求，就转到Http10Handler执行
    if(request.isHttp1_0){
        return Http10Handler::onRequestLineEvent();
    }

    //检查是否GET或POST或HEAD
    if(strcmp(request.method, METHOD_GET)==0){
        request.isGet = true;
    }else if(strcmp(request.method, METHOD_POST)==0){
        request.isPost = true;
    }else if(strcmp(request.method, METHOD_HEAD)==0){
        request.isHead = true;
    }else if(strcmp(request.method, METHOD_PUT)==0){        
        request.isPut = true;
    }else{
        httpCode = HTTP_CODE_405;
        return true;
    }
    
    return false;
}


bool Http11Handler::onCompleteEvent(){
    LOG_DEBUG("status:%d", status);
    //如果是Http 1.0请求，就转到Http10Handler执行
    if(!request.isHttp1_1){
        return Http10Handler::onCompleteEvent();
    }

    const char* connection = request.getHeader("Connection");
    if(connection && strcasecmp(connection, "close")==0){
        stop();
    }else{
        clean();
    }
    return true;
}
