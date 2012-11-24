/* 
 * File:   Response.cpp
 * Author: try
 * 
 * Created on 2011年6月2日, 下午11:19
 */

#include <sys/stat.h>
#include <time.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "Response.h"
#include "HttpHandler.h"
#include "SocketUtils.h"
#include "Config.h"
#include "MemoryPool.h"
#include "SendfileIOPump.h"

using namespace std;

Response::Response(HttpHandler& httpHandler, Request& request, Config* config):httpHandler(httpHandler), request(request),
        status(0), msg("OK"), headerLength(0), headerWriteFlag(false), isChunkedTransferEncoding(false),
        contentLength(-1), bufferSize(config->responseBufferSize), bufferPointer(0), buffer(NULL), asynAnswerMode(false)
{
        buffer = (char*)MemoryPool::instance.malloc(bufferSize);
}

Response::~Response() {
    //LOG_DEBUG("")
    clean();
    if(buffer){
        MemoryPool::instance.free(buffer);
    }
    //清理ioPumpers
    for(IOPumpers::iterator it = ioPumpers.begin(); it != ioPumpers.end(); it++){
        delete (*it);
    }
    ioPumpers.clear();    
}

/** 清理 */
void Response::clean(){

    status = 0; //HTTP状态码, 默认为:0
    msg = "OK";
    headerLength = 0;          //响应头长度，默认为:0
    headerWriteFlag = false;        //如果已经输出响应头，设置此标记
    isChunkedTransferEncoding = false;  //内容传输方式是否：chunked方式
    contentLength = -1;         //内容长度，默认为:-1
    bufferPointer = 0;    //当前写指针位置，默认为:0
    asynAnswerMode = false;
    
    //清除消息头
    headers.clear();
    //释放组内存池
    memGroup.freeAll();
    //清除消息头写缓冲
    headerBuffer.clear();
    

}



void Response::addHeader(const char* name, const char* value){
    unsigned int vlen = strlen(value);
    char* nval = (char*)memGroup.copy(value, vlen+1, vlen);
    nval[vlen] = 0;
    const char* v = headers.get(name);
    if(v){
        headers.add(name, nval);
    }else{
        int klen = strlen(name);
        char* nkey = (char*)memGroup.copy(name, klen+1, klen);
        nkey[klen] = 0;
        headers.add(nkey, nval);
    }
}

void Response::setHeader(const char* name, const char* value){
    unsigned int vlen = strlen(value);
    char* nval = (char*)memGroup.copy(value, vlen+1, vlen);
    nval[vlen] = 0;
    const char* v = headers.get(name);
    if(v){
        headers.set(name, nval);
    }else{
        int klen = strlen(name);
        char* nkey = (char*)memGroup.copy(name, klen+1, klen);
        nkey[klen] = 0;
        headers.set(nkey, nval);
    }
}

/** 新增或追加响应头, 值为int类型 */
void Response::setHeader(const char* name, int intValue){
    char value[32];
    sprintf(value,"%d",intValue);
    setHeader(name, value);
}

/** 新增或追加响应头, 值为int类型 */
void Response::addHeader(const char* name, int intValue){
    char value[32];
    sprintf(value,"%d",intValue);
    addHeader(name, value);
}

void Response::addDateHeader(const char* name, long int value){
    char timestr[32];
    timestr[strftime(timestr,32,"%a, %d %b %Y %H:%M:%S GMT",gmtime(&value))]='\0';
    addHeader(name, timestr);
}

void Response::setDateHeader(const char* name, long int value){
    char timestr[32];
    timestr[strftime(timestr,32,"%a, %d %b %Y %H:%M:%S GMT",gmtime(&value))]='\0';
    setHeader(name, timestr);
} 

void Response::setContentLength(long len){
    contentLength = len;
    setHeader("Content-Length", len);
}

void Response::setContentType(const char* type){
    setHeader("Content-Type", type);
}

void Response::setStatus(unsigned short int status, const char* msg){
    this->status = status;
    httpHandler.httpCode = status;
    if(msg){
        this->msg = msg;
    }
}

/** 返回状态码 */
unsigned short int Response::getStatusCode(){
    return status;
}

/** 返回状态消息 */
const char* Response::getStatusMessage(){
    return msg.c_str();
}

bool Response::write(long value){
    char buff[64];
    sprintf(buff, "%ld", value);
    return write(buff);
}

bool Response::write(int value){
    char buff[64];
    sprintf(buff, "%d", value);
    return write(buff);
}

/** 向客户端写数据，b不能为NULL, 且strlen(b)必须大于等于len*/
bool Response::write(const char* b, unsigned int len){
    return write(b, 0, len);
}

/** 向客户端写数据，str为以0结尾的字符串 */
bool Response::write(const char* str){
    return write(str, strlen(str));
}

bool Response::write(const char* b, unsigned int off, unsigned int len){
    if(len == 0 || request.isHead){
        return false;
    }
    
    if(!httpHandler.isRunning()){
        return false;
    }
    
    do{
        int freeSize = bufferSize - bufferPointer;
        if(freeSize >= len){
            memcpy(buffer+bufferPointer, b+off, len);
            bufferPointer += len;
            break;
        }
        memcpy(buffer+bufferPointer, b+off, freeSize);
        bufferPointer += freeSize;
        if(!writeBuffer(false)){
            return false;
        }
        len -= freeSize;
        off += freeSize;

    }while(true);
    return true;
}


bool Response::sendfile(FD fd, off_t readOffset, size_t len, unsigned int speed, size_t maxSendSize /* =DEFAULT_DATA_SIZE */){
    //LOG_DEBUG("fd:%d,readOffset:%d,len:%d", fd, readOffset,len);
    readOffset = readOffset < 0 ? 0 : readOffset;
    if(len <= 0){
            LOG_WARN("len <= 0, is %d", len);
            return false;
    }
    
    //LOG_DEBUG("bufferPointer:%d", bufferPointer);
    //刷新缓冲区
    writeBuffer(false);

    //LOG_DEBUG("1 isChunkedTransferEncoding:%d", isChunkedTransferEncoding);
    if(isChunkedTransferEncoding){
        char wsizeValue[32];
        sprintf(wsizeValue,"%x",len);
        int wsizeValueLen = strlen(wsizeValue);
        wsizeValue[wsizeValueLen] = CHAR_CR;
        wsizeValue[wsizeValueLen+1] = CHAR_LF;
        if(!httpHandler.iowriter.write(wsizeValue, wsizeValueLen+2)){//写块头, 包括回车换行符
            return false;
        }
    }
    
    SendfileIOPump* ioPump = new SendfileIOPump(httpHandler.getClientFD(), fd, speed, maxSendSize);
    ioPumpers.push_back(ioPump);
    ioPump->setOffset(readOffset);
    ioPump->setLength(len);
    
    //将ioPump存起来，在销毁Response时将其销毁
    if(!httpHandler.iowriter.transport(ioPump)){
        return false;
    }
    
    if(isChunkedTransferEncoding){
        if(!sendln()){//写回车换行符
           LOG_WARN("write CRLF error");
           return false;
        }
    }
    return true;
}

void Response::complete(){
    LOG_DEBUG("httpHandler.isRunning():%d, httpHandler.status:%d", httpHandler.isRunning(), httpHandler.status);
    if(!httpHandler.isRunning()){
        //LOG_WARN("httpHandler.isRunning():%d, httpHandler.status:%d", httpHandler.isRunning(), httpHandler.status);
        httpHandler.startWrite();
        return;
    }
    
    if(!isChunkedTransferEncoding){
        setContentLength(bufferPointer);
    }
    
    if(bufferPointer > 0){
        if(!writeBuffer(false)){
            httpHandler.startWrite();
            httpHandler.status = HttpHandler::ANSWERING;
            httpHandler.onCompleteEvent();
            return;
        }
    }
    
    if(!writeBuffer(true)){
        httpHandler.startWrite();
        httpHandler.status = HttpHandler::ANSWERING;
        httpHandler.onCompleteEvent();
        return;
    }
    
    httpHandler.startWrite();
    httpHandler.status = HttpHandler::ANSWERING;
    httpHandler.onCompleteEvent();
}

size_t Response::waitingWriteQueueSize(){
    return httpHandler.iowriter.waitingQueueSize();
}

bool Response::headerSended(){
    return headerWriteFlag;
}

bool Response::flushBuffer(){
    bool wrs = writeBuffer(false);
    httpHandler.iowriter.start();
    return wrs;
}

bool Response::writeBuffer(bool flushEmpty){
    if(request.isHead){
        if(!headerWriteFlag){
            if(!flushHeaders()){
                return false;
            }
        }
        return true;
    }

    int wsize;
    if(!headerWriteFlag){
        if(contentLength == -1){
            setHeader("Transfer-Encoding", "chunked");
            isChunkedTransferEncoding = true;
        }
        if(!flushHeaders()){
            return false;
        }
        wsize = (contentLength != -1 && contentLength<=bufferPointer)?contentLength:bufferPointer;
    }else{
        wsize = bufferPointer;
    }

    if(bufferPointer == 0 && !flushEmpty){
        return true;
    }    

    if(isChunkedTransferEncoding){
        char wsizeValue[32];
        sprintf(wsizeValue,"%x",wsize);
        int wsizeValueLen = strlen(wsizeValue);
        wsizeValue[wsizeValueLen] = CHAR_CR;
        wsizeValue[wsizeValueLen+1] = CHAR_LF;
        if(!httpHandler.iowriter.write(wsizeValue, wsizeValueLen+2)){//写块头, 包括回车换行符
            return false;
        }
        if(wsize != 0){
            if(!httpHandler.iowriter.write(buffer, wsize)){//写块数据
                return false;
            }
        }else{//flushEmpty
            //if(!httpHandler.iowriter.write(wsizeValue+wsizeValueLen, 2)) {//写回车换行符, 所有块都写完了，会写最后一个结束标记块
            if(!sendln()){
                return false;
            }
        }
        //if(!httpHandler.iowriter.write(wsizeValue+wsizeValueLen, 2)){//写回车换行符
        if(!sendln()){
           return false;
        }
    }else{
        if(wsize > 0){
            if(!httpHandler.iowriter.write(buffer, wsize)){
                return false;
            }
        }
    }
    bufferPointer = 0;
    return true;
}

//写响应行和响应头信息
bool Response::flushHeaders(){
    if(headerWriteFlag)return true;
    
    //写响应行
    const char* protocol;
    if(request.isHttp1_1){
        protocol = HTTP_1_1;
    }else if(request.isHttp1_0){
        protocol = HTTP_1_0;
    }else{
        LOG_ERROR("protocol not exist");
        protocol = HTTP_1_0;
    }
    headerBuffer.insert(headerBuffer.end(), protocol, protocol + PROTOCOL_LENGTH);

    headerBuffer.push_back(CHAR_SP);
    char statusBuff[32];
    sprintf(statusBuff,"%d",status==0?HTTP_CODE_200:status);
    headerBuffer.insert(headerBuffer.end(), statusBuff, statusBuff + strlen(statusBuff));
    headerBuffer.push_back(CHAR_SP);
    const char* msgstr = msg.c_str();
    headerBuffer.insert(headerBuffer.end(), msgstr, msgstr + msg.size());
    headerBuffer.push_back(CHAR_CR);
    headerBuffer.push_back(CHAR_LF);
    //写响应头
    const char* key;
    int keylen;
    const char* text;
    Headers::Value* value;
    
    Headers::Data::iterator hit = headers.data.begin();
    for(; hit != headers.data.end(); hit++){
        key = hit->first;
        keylen = strlen(key);
        value = hit->second;
        while(value){
            headerBuffer.insert(headerBuffer.end(), key, key + keylen);
            headerBuffer.push_back(CHAR_COLON);
            headerBuffer.push_back(CHAR_SP);
            text = value->text;
            if(text){
                headerBuffer.insert(headerBuffer.end(), text, text + strlen(text));
                headerBuffer.push_back(CHAR_CR);
                headerBuffer.push_back(CHAR_LF);
            }
            value = value->next;
        }
    }
    headerBuffer.push_back(CHAR_CR);
    headerBuffer.push_back(CHAR_LF);

    if(!httpHandler.iowriter.write(headerBuffer.data(), headerBuffer.size())){
         return false;
    }
    headerWriteFlag = true;
    return true;
}


void Response::setAsynAnswerMode(bool b){
    asynAnswerMode = b;
}

bool Response::isAsynAnswerMode(){
    return asynAnswerMode;
}

bool Response::sendln(){
    return httpHandler.iowriter.write(STR_CRLF, 2, false);
}


