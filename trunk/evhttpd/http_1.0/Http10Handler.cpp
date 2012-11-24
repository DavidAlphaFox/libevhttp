/* 
 * File:   Http10Handler.cpp
 * Author: try
 * 
 * Created on 2011年6月2日, 上午9:39
 */

#include <istream>
#include <string>
#include "Http10Handler.h"
#include "../HttpServletFactory.h"
#include "../SocketUtils.h"
#include "../HttpProcess.h"

using namespace std;

Http10Handler::Http10Handler(){

}

Http10Handler::~Http10Handler() {
}

//bool Http10Handler::prepareRequestBody(){
//    int n = requestData.data.size()-requestData.bodyStartPos;//当前读到的body字节数量
//    if(servlet->onReadBodyEvent(&(requestData.data.data()[requestData.bodyStartPos]), n)){
//        requestData.existReqBody = true;
//    }else{
//        RequestData::DataIterator it = requestData.data.begin() + requestData.bodyStartPos;
//        requestData.data.erase(it, it + n);
//    }
//    
//    //int cttlen = request.getContentLength();
////    int n = requestData.data.size()-requestData.bodyStartPos;//当前读到的body字节数量
////    if(request.isMultipartContentType){
////        bodyDataBytes += n;//累计读到的body字节数量
////        if(n > 0){
////            servlet->onReadBodyEvent(&(requestData.data.data()[requestData.bodyStartPos]), n);
////            RequestData::DataIterator begin = requestData.data.begin();
////            requestData.data.erase(begin+requestData.bodyStartPos, begin+requestData.bodyStartPos+n);
////        }
////        if(bodyDataBytes == request.contentLength){
////            requestData.existReqBody = true;
////        }
////    }else if(request.isChunkedTransferEncoding){
////        bodyDataBytes += n;//累计读到的body字节数量
////        if(n > 0){
////            //如果你认为已经把数据读完了，就返回true吧
////            bool rs = servlet->onReadBodyEvent(&(requestData.data.data()[requestData.bodyStartPos]), n);
////            if(rs){
////                requestData.existReqBody = true;
////            }else{
////                RequestData::DataIterator begin = requestData.data.begin();
////                requestData.data.erase(begin+requestData.bodyStartPos, begin+requestData.bodyStartPos+n);
////            }
////        }
////        
////    }else{
////        if(request.contentLength < 0){
////            httpCode = HTTP_CODE_400;
////        }else if(n >= request.contentLength){
////            servlet->onReadBodyEvent(&(requestData.data.data()[requestData.bodyStartPos]), request.contentLength);
////            requestData.buildRequestBody(request.contentLength);
////        }
////    }
//
//    return httpCode == 0;
//}

bool Http10Handler::handleRequest(){
    if(request.isGet){
        servlet->service(request, *response);
        if(!response->isAsynAnswerMode()){
            //回应客户端请求
            answer();
            return true;
        }
        
    }else if(request.isPost){
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

    }else if(request.isHead){
        servlet->service(request, *response);
        if(!response->isAsynAnswerMode()){
            //回应客户端请求
            answer();
            return true;
        }
    }
    
    return false;
}

ssize_t Http10Handler::getContentLength(){
    const char* ctxlen = request.getHeader("Content-Length");
    if(ctxlen){
        return atol(ctxlen);
    }
    return -1;
}

bool Http10Handler::isMultipartContentType(){
    //检查Content-Type是否为multipart/form-data;
    static const char* multipartContentType = "multipart/form-data; boundary=";
    static int multipartContentTypeLen = strlen(multipartContentType);
    if(request.contentType && memcmp(request.contentType, multipartContentType, multipartContentTypeLen)==0){
        request.boundary = request.contentType + multipartContentTypeLen;
        return true;
    }
    return false;
}

bool Http10Handler::isChunkedTransferEncoding(){
    if(request.contentType && strcmp(request.contentType, "binary/octet-stream") == 0){
        const char* chunked = request.getHeader("Transfer-Encoding");
        return chunked && strcmp(chunked, "chunked") == 0;
    }
    return false;
}


//--事件------------------------------------------------------------------------
bool Http10Handler::onReadEvent(unsigned int n){
    //检查请求长度是否合法, 当还没成功读完请求行时，数据长度已经大于了约定长度时
    //认为此请求非法
    if(!requestData.existReqLine && requestData.data.size() > httpProcess->config->requestLineMaxBytes){ //REQUEST_LINE_MAX_BYTES
        //如果请求行大于REQUEST_LINE_MAX_BYTES字节
        httpCode = HTTP_CODE_414;
        LOG_WARN("httpCode:%d, request line oversize", httpCode);
        return true;
    }
    return false;
}

 bool Http10Handler::onRequestLineEvent(){

     //检查是否GET或POST或HEAD
     if(strcmp(request.method, METHOD_GET)==0){
         request.isGet = true;
     }else if(strcmp(request.method, METHOD_POST)==0){
         request.isPost = true;
     }else if(strcmp(request.method, METHOD_HEAD)==0){
         request.isHead = true;
     }else{
         httpCode = HTTP_CODE_405;
         return true;
     }

     //检查HTTP协议是否HTTP/1.0
     if(strcmp(requestData.protocol, HTTP_1_0)!=0){
         httpCode = HTTP_CODE_505;
         request.isHttp1_0 = true;
         return true;
     }
     request.isHttp1_0 = true;

     return false;
 }

bool Http10Handler::onRequestHeadersEvent(){
    request.contentLength = getContentLength();
    request.contentType = request.getHeader("Content-Type");    
    request.isMultipartContentType = isMultipartContentType();
    request.isChunkedTransferEncoding = isChunkedTransferEncoding();    
    return false;
}

bool Http10Handler::onHttpCodeEvent(){
    LOG_DEBUG("httpCode:%d, status:%d, StatusMessage:%s", httpCode, status, response->getStatusMessage());
    if(response->getStatusCode() == 0){
        if(httpCode > 0){
             response->setStatus(httpCode, "error");
        }else{
             response->setStatus(HTTP_CODE_500, "Internal Server Error");
        }
//    }else{
//        stop();
    }
    //回应客户端请求
    answer();
    return true;
}


bool Http10Handler::onCompleteEvent(){
    //请求处理完成，停止Handler服务
    stop();
    return true;
}



 
