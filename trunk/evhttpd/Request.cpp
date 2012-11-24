/* 
 * File:   Request.cpp
 * Author: try
 * 
 * Created on 2011年6月2日, 下午11:18
 */

#include "Request.h"
#include "StringUtils.h"

using namespace std;

Request::Request(RequestData& requestData):requestData(requestData), remoteAddr(requestData.remoteAddr), 
    method(requestData.method),path(requestData.path), parameter(requestData.parameter), protocol(requestData.protocol),
    isGet(false), isPost(false), isHead(false), isPut(false), isHttp1_0(false), isHttp1_1(false),
    //body(requestData.body),
    contentType(NULL), contentLength(-1), isMultipartContentType(false), boundary(NULL), isChunkedTransferEncoding(false)
{

}

Request::~Request() {
    clean();
}


void Request::clean(){
    isGet = false;
    isPost = false;
    isHead = false;
    isPut = false;
    isHttp1_0 = false;
    isHttp1_1 = false;
    contentType = NULL;
    contentLength = -1;
    isMultipartContentType = false;
    boundary = NULL;
    isChunkedTransferEncoding = false;
    headerNames.clear();
    parameterNames.clear();
    requestData.clean();
}


const char* Request::getHeader(const char* name){
    return requestData.headers.get(name);
}

vector<const char*>& Request::getHeaders(const char* name, vector<const char*>& rsv){
    return requestData.headers.gets(name, rsv);
}


const vector<const char*>& Request::getHeaderNames(){
    if(requestData.headers.size() > 0 && headerNames.size() == 0){
        requestData.headers.getNames(headerNames);
    }
    return headerNames;
}

const char* Request::getContentType(){
    //return getHeader("Content-Type");
    return contentType;
}

ssize_t Request::getContentLength(){
    return contentLength;
}

const char* Request::getParameter(const char* name){
    if(!requestData.parameterPrepared){
        requestData.buildRequestParameters();
    }
    return requestData.parameters.get(name);
}

vector<const char*>& Request::getParameterValues(const char* name, vector<const char*>& rsv){
    if(!requestData.parameterPrepared){
        requestData.buildRequestParameters();
    }
    requestData.parameters.gets(name, rsv);
    return rsv;
}

const std::vector<const char*>& Request::getParameterNames(){
    if(!requestData.parameterPrepared){
        requestData.buildRequestParameters();
    }

    if(requestData.parameters.size() > 0 && parameterNames.size() == 0){
        requestData.parameters.getNames(parameterNames);
    }

    return parameterNames;
}

