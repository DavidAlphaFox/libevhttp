/* 
 * File:   HttpHandler.cpp
 * Author: try
 * 
 * Created on 2011年5月28日, 下午3:55
 */

#include "HttpHandler.h"
#include "HttpProcess.h"
#include "SocketUtils.h"
#include <iostream>
#include <queue>
#include <deque>
#include "libev.h"
#include <errno.h>
#include "MemoryPool.h"
#include "HttpServletManager.h"
#include "HttpHandlerManager.h"


using namespace std;

HttpHandler::HttpHandler():status(NEW), fd(-1), 
        idleTimeout(false), httpProcess(NULL), readBuff(NULL), request(requestData), response(NULL), httpCode(0),
        servlet(NULL),
        bodyDataBytes(0), completeFlag(false)
    {
}

HttpHandler::~HttpHandler() {
    if(status != STOP){
        stop();
    }
    if(readBuff){
        MemoryPool::instance.free(readBuff);
    }
    if(response) delete response;
    
}

void HttpHandler::init(HttpProcess* p, FD fd){
    if(!readBuff){
        readBuff = (char*)MemoryPool::instance.malloc(p->config->requestBufferSize);
    }
    if(!response){
        response = new Response(*this, request, p->config);
    }
    
    this->status = NEW;
    this->httpProcess = p;
    this->fd = fd;
    this->servlet = NULL;
    this->completeFlag = false;
    this->setIdleTimeout(true);

}

void HttpHandler::callServletOnWriteQueueEmptyEvent(){
    if(servlet){
        servlet->onWriteQueueEmptyEvent();
    }
}


HttpHandler::STATUS HttpHandler::getStatus(){
    return status;
}

void HttpHandler::start(){
    if(isClosed()){
        LOG_ERROR("isClosed()");
        return;
    } 
    LOG_DEBUG("fd:%d", fd);
    status = RUNNING;
    
    //初始化IOWriter, 设置FD, 设置事件监听器
    iowriter.setFD(fd);
    iowriter.addListener(EVENT_TYPE_IO_WRITE_COMPLETION, this);
    
    //初始化IOReader, 设置FD, 设置事件监听器
    ioreader.setFD(fd);
    ioreader.addListener(EVENT_TYPE_IO_READ_COMPLETION, this);
    prepareRead(); //准备读Socket数据
    
    evtimer.set<HttpHandler, &HttpHandler::timeoutCallback>(this);
}

bool HttpHandler::isRunning(){
    return status == RUNNING;
}

bool HttpHandler::isClosed(){
    return fd < 0;
}

void HttpHandler::setIdleTimeout(bool b){
    if(idleTimeout != b){
        if(idleTimeout && evtimer.is_active()){
            evtimer.stop();
        }
        idleTimeout = b;
    }
}

void HttpHandler::startTimeoutEv(){
    if(idleTimeout){
        if(evtimer.is_active()){
            evtimer.stop();
        }
        evtimer.start(httpProcess->config->idleTimeout, 0);
    }
}

void HttpHandler::stopTimeoutEv(){
    if(idleTimeout){
        if(evtimer.is_active()){
            evtimer.stop();
        }
    }
}
            

void HttpHandler::error(){
    iowriter.close();
    ioreader.close();
    stop();
}

void HttpHandler::notifyClose(){
    //Servlet不存在或onNotifiedEvent()返回true, 就停止服务
    if(!servlet || servlet->onNotifiedCloseEvent()){
        stop();
    }
}

/**
 * 强制关闭
 */
void HttpHandler::close(){
    error();
}

void HttpHandler::stop(){
    LOG_DEBUG("iowriter.waitingQueueSize():%d, status:%d", iowriter.waitingQueueSize(), status);
    if(status == STOP){
        LOG_WARN("already is STOP,  fd:%d, status:%d", this->fd, status);
        return;
    }
    //LOG_DEBUG("1 fd:%d, status:%d", this->fd, status);
    if(iowriter.waitingQueueSize() > 0){
        status = STOPPING;
        //如果responseDataQueue不为空，说明还有没发送完的数据
        return;
    }
    //LOG_DEBUG("2 fd:%d, status:%d", this->fd, status);
    if(status != STOP){
        status = STOP;
        
        iowriter.close();
        ioreader.close();
        
        //必须在status = STOP;之后
        clean();
        
//        ::shutdown(fd, SHUT_RDWR);
        ::close(fd);
        fd = -1;
        
        //回收HttpHandler
        HttpHandlerManager::instance.recycle(this);
        
        stopTimeoutEv();
    }
}

void HttpHandler::clean(){
    LOG_DEBUG("iowriter.waitingQueueSize():%d, status:%d", iowriter.waitingQueueSize(), status);
    if(iowriter.waitingQueueSize() > 0){
        status = CLEANING;
        //如果responseDataQueue不为空，说明还有没发送完的数据
        return;
    }
    
    stopWrite();
    
    if(servlet){
        //发起请求完成事件
        servlet->onCompletionEvent();
        //回收HttpServlet
        HttpServletManager::instance.recycle(servlet);
        servlet = NULL;        
    }
    
//    if(requestData.data.size() > 0 || httpCode > 0){
//        //请求计数
//        httpProcess->totalRequestCount++;
//    }else{
//        LOG_DEBUG("HttpHandler::clean, requestData.data.size():%d, httpCode:%d", requestData.data.size(), httpCode);
//    }
    
    httpProcess->totalRequestCount++;
    
    //复位HTTP Code
    httpCode = 0;

    bodyDataBytes = 0;
    
    idleTimeout = true;
    
    completeFlag = false;

    //清理请求数据
    request.clean();
    //清理响应数据
    response->clean();
    
//    LOG_DEBUG("HttpHandler::clean, 2 fd:%d, status:%d", fd, status);
    
    if(status != STOP){
        LOG_DEBUG("3 fd:%d, status:%d", fd, status);
        status = RUNNING;
        //开始超时事件
       //startTimeoutEv();
        prepareRead();
    }
}

bool HttpHandler::prepareRequestData(){
    //准备请求行
    if(!requestData.existReqLine){
        if(!prepareRequestLine()){
            return false;
        }
        if(requestData.existReqLine){
            //LOG_DEBUG("onRequestLineEvent start");
            //发送请求行准备好事件
            onRequestLineEvent();
            //LOG_DEBUG("onRequestLineEvent end, httpCode:%d", httpCode);
            if(httpCode != 0){
                return false;
            }
        }
    }

    //准备请求头
    if(requestData.existReqLine && !requestData.existReqHeaders){
        if(!prepareRequestHeaders()){
            return false;
        }
        if(requestData.existReqHeaders){
            //发送消息头准备好事件
            onRequestHeadersEvent();
            if(httpCode != 0){
                return false;
            }
        }
    }

    return true;
}


bool HttpHandler::prepareRequestLine(){
    int size = requestData.data.size();
    const char* data = requestData.data.data();
    int begin = requestData.notTraversePos;

    //准备请求方式位置
    if(requestData.methodEndPos == 0){
        for(; begin < size; begin++){
            if(data[begin] == CHAR_SP){
                requestData.methodEndPos = begin;
                requestData.notTraversePos = ++begin;
                requestData.pathStartPos = begin;
                break;
            }else if(data[begin] == CHAR_LF){
                httpCode = HTTP_CODE_400; //请求错误
                return false;
            }
        }
    }

    //准备请求URI路径
    if(requestData.methodEndPos > 0 && requestData.pathEndPos == 0){
        for(; begin < size; begin++){
            if(data[begin] == CHAR_QUESTION){
                 requestData.pathEndPos = begin;
                 requestData.notTraversePos = ++begin;
                 requestData.parameterStartPos = begin;
                 break;
            }else if(data[begin] == CHAR_SP){
                requestData.pathEndPos = begin;
                //没有请求参数, 设置开始位置和结束位置相等
                requestData.parameterStartPos = begin;
                requestData.parameterEndPos = begin;
                requestData.notTraversePos = ++begin;
                requestData.protocolStartPos = requestData.notTraversePos;
                break;
            }else if(data[begin] == CHAR_LF){
                int pos = begin-1;
                requestData.pathEndPos = pos;
                requestData.parameterStartPos = pos;
                requestData.parameterEndPos = pos;

                requestData.protocolStartPos = pos;
                requestData.protocolEndPos = pos;
                requestData.notTraversePos = ++begin;

                requestData.reqLineEndPos = pos;
                requestData.headersStartPos = requestData.notTraversePos;
                requestData.currentHeaderStartPos = requestData.notTraversePos;
                requestData.existReqLine = true;

                break;
            }
        }
    }

    //准备请求参数
    if(requestData.pathEndPos > 0 && requestData.parameterEndPos == 0){
        for(; begin < size; begin++){
            if(data[begin] == CHAR_SP){
                requestData.parameterEndPos = begin;
                requestData.notTraversePos = ++begin;
                requestData.protocolStartPos = requestData.notTraversePos;
                break;
            }else if(data[begin] == CHAR_LF){

                int pos = begin-1;
                requestData.parameterEndPos = pos;

                requestData.protocolStartPos = pos;
                requestData.protocolEndPos = pos;
                requestData.notTraversePos = ++begin;

                requestData.reqLineEndPos = pos;
                requestData.headersStartPos = requestData.notTraversePos;
                requestData.currentHeaderStartPos = requestData.notTraversePos;
                requestData.existReqLine = true;

            }
        }
    }

    //准备请求协议信息
    if(requestData.parameterEndPos > 0 && requestData.protocolEndPos == 0){
        for(; begin < size; begin++){
            if(data[begin] == CHAR_LF){
                int pos = begin-1;
                requestData.protocolEndPos = pos;
                requestData.notTraversePos = ++begin;
                requestData.reqLineEndPos = pos;
                requestData.headersStartPos = requestData.notTraversePos;
                requestData.currentHeaderStartPos = requestData.notTraversePos;
                requestData.existReqLine = true;
                break;
            }
        }
    }
    
    if(requestData.existReqLine){
        //构建请求行
        requestData.buildRequestLine();
    }

    return true;
}


bool HttpHandler::prepareRequestHeaders(){
    
    int size = requestData.data.size();
    const char* data = requestData.data.data();
    int begin = requestData.currentHeaderStartPos;

    int keyStartPos = 0;
    int keyEndPos = 0;
    int valueStartPos = 0;
    int valueEndPos = 0;
    char c;
    char lc;
    for(; begin < size; begin++){
        lc = begin>requestData.headersStartPos?data[begin-1]:0;
        c = data[begin];
        
        if(c == CHAR_LF && lc == CHAR_CR && data[begin-2] == CHAR_LF){
            //所有消息头处理完成
                requestData.headersEndPos = begin-1;
                requestData.existReqHeaders = true;
                requestData.notTraversePos = begin+1;
                requestData.bodyStartPos = requestData.notTraversePos;
                //构建消息头
                requestData.buildRequestHeaders();
            break;
        }
        
        if(keyStartPos == 0){
            keyStartPos = lc==0?begin:begin-1; //Key开始位置
        }

        if(keyEndPos == 0 && c == ':'){
            keyEndPos = begin;  //Key结束位置
            continue;
        }

        if(keyEndPos > 0 && valueStartPos == 0){
            valueStartPos = begin; //Value开始位置
        }
        
        //if(c == CHAR_LF){ //c == CHAR_CR
        if(lc == CHAR_LF && (c != CHAR_SP && c != CHAR_HT)){
            int pos = begin-2;

            //一行消息头处理完成
            valueEndPos = pos; //Value结束位置
            if(keyEndPos == 0){
                //说明只有key开始位置，没有=号和值
                keyEndPos = pos;
                valueStartPos = pos; //值开始位置和结束位置相等
            }

            //构建消息头结束
            if(keyStartPos < keyEndPos){
                requestData.saveHeaderPos(keyStartPos, keyEndPos, valueStartPos, valueEndPos);
            }
//            LOG_DEBUG("keyStartPos:%d, keyEndPos:%d, valueStartPos:%d, valueEndPos:%d", keyStartPos, keyEndPos, valueStartPos, valueEndPos);
            
            //初始化下一个消息头位置信息
            keyStartPos = 0;
            keyEndPos = 0;
            valueStartPos = 0;
            valueEndPos = 0;
            requestData.currentHeaderStartPos = begin;
        }
    }

    return true;
}


bool HttpHandler::prepareRequestBody(){
    size_t n = requestData.data.size()-requestData.bodyStartPos;//当前读到的body字节数量
    if(servlet->onReadBodyEvent(&(requestData.data.data()[requestData.bodyStartPos]), n)){
        requestData.existReqBody = true;
    }else{
        RequestData::DataIterator it = requestData.data.begin() + requestData.bodyStartPos;
        requestData.data.erase(it, it + n);
    }
    return httpCode == 0;
}


bool HttpHandler::handleRequest(){
    servlet->service(request, *response);
    if(!response->isAsynAnswerMode()){
        //回应客户端请求
        answer();
        return true;
    }
    return false;
}

//bool HttpHandler::service(Request& req, Response& resp){
//    return true;
//}

void HttpHandler::answer(){
    response->complete();
}


/**
 * 处理读到的数据
 */
void HttpHandler::handleReadCompletionEvent(IOReadCompletionEvent* e){
    LOG_DEBUG("isRunning:%d, readsize:%d", isRunning(), e->length);
    
    //停止超时事件
    stopTimeoutEv();
    
    //读数据错误或读到EOF
    if(e->length <= 0){
        //LOG_DEBUG("read size:%d, errno:%d", e->length, errno);
        error();
        return;        
    }
    
    //将读到的数据写入requestData.data
    requestData.data.insert(requestData.data.end(), e->dataBuff, e->dataBuff + e->length);

    
    //在成功读入数据后调用此方法
    onReadEvent(e->length);
    if(httpCode != 0){
        //发生错误, 根据httpCode值进行处理
        onHttpCodeEvent();
        return;
    }

    //准备请求数据
    if(!requestData.existReqHeaders){
        if(!prepareRequestData()){
            //发生错误, 根据httpCode值进行处理
            onHttpCodeEvent();
            return;
        }
    }

    if(requestData.existReqHeaders){
        //第一次进入handle时创建HttpServlet
        LOG_DEBUG("servlet exist:%s", (servlet?"true":"false"));
        if(!servlet){
            servlet = HttpServletManager::instance.create(request.path);
            servlet->init(this, request, *response);
        }
        completeFlag = handleRequest();
        LOG_DEBUG("1, completeFlag:%d, httpCode:%d", completeFlag, httpCode);
        if(httpCode != 0){
            //发生错误, 根据httpCode值进行处理
            onHttpCodeEvent();
            return;
        }
    }
    LOG_DEBUG("2, completeFlag:%d, httpCode:%d", completeFlag, httpCode);
    if(!completeFlag){
        LOG_DEBUG("prepareRead");
        prepareRead();
    }
    
}

/**
 * 处理写完成事件
 */
void HttpHandler::handleWriteCompletionEvent(IOWriteCompletionEvent* e){
    LOG_DEBUG("e.length:%d, iowriter.waitingQueueSize():%d, status:%d", e->length, iowriter.waitingQueueSize(), status)
            
    if(e->length < 0){
        //LOG_WARN("error, e->length:%d", e->length);
        error();
    }else{
//        if(e->length == 0){
//            LOG_DEBUG("e->length == 0, status:%d, iowriter.waitingQueueSize():%d", status, iowriter.waitingQueueSize());
//        }
        
        if(status == STOP){
            LOG_WARN("status is STOP");
            stopWrite();
            return;
        }
        if(isClosed()){
            LOG_WARN("isClosed()");
            stopWrite();
            return;
        }
        
        if(iowriter.waitingQueueSize() == 0){
            //检查是否是STOPPING状态，如果是就停止服务
            if(status == STOPPING){
                stop();
            }else if(status == CLEANING){
                clean();
            }else if(status == ANSWERING){
                LOG_WARN("status is ANSWERING");
            }else{
                callServletOnWriteQueueEmptyEvent();
                startTimeoutEv();
            }
        }
    }
}

//提交读Socket数据请求
bool HttpHandler::prepareRead(){
    if(isClosed())return false;
    startTimeoutEv();
    return ioreader.read(readBuff, httpProcess->config->requestBufferSize);
}

//暂停读Socket数据
void HttpHandler::stopRead(){
    if(isClosed())return;
    ioreader.reset();
}

void HttpHandler::startWrite(){
    if(isClosed())return;
    iowriter.start();
}

void HttpHandler::stopWrite(){
    if(isClosed())return;
    iowriter.reset();
}

bool HttpHandler::handle(const Event* e){
    LOG_DEBUG("e->getType():%d", e->getType())
    switch(e->getType()){
        case EVENT_TYPE_IO_READ_COMPLETION:
            handleReadCompletionEvent((IOReadCompletionEvent*)e);
            return true;
        case EVENT_TYPE_IO_WRITE_COMPLETION:
            handleWriteCompletionEvent((IOWriteCompletionEvent*)e);
            return true;
    }
    return false;
}


void HttpHandler::timeoutCallback(ev::timer &ev, int revents){
    //连接空闲超时, 停止服务
    LOG_DEBUG("fd:%d, status:%d", fd, status);
    error();
}


//--事件------------------------------------------------------------------------
bool HttpHandler::onReadEvent(unsigned int n){
    return false;
}

 bool HttpHandler::onRequestLineEvent(){
     return false;
 }

 bool HttpHandler::onRequestHeadersEvent(){
     return false;
 }

 bool HttpHandler::onHttpCodeEvent(){
     return false;
 }

bool HttpHandler::onCompleteEvent(){
    //请求处理完成，停止Handler服务
    stop();
    return true;
}



