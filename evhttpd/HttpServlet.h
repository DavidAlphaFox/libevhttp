/* 
 * File:   HttpServlet.h
 * Author: try
 *
 * Created on 2011年5月28日, 下午3:53
 */

#ifndef _HTTPSERVLET_H
#define	_HTTPSERVLET_H

#include "Request.h"
#include "Response.h"
#include <iostream>
#include "log.h"
/**
 * 实现此类型来处理HTTP请求
 */
class HttpHandler;
class HttpServlet {
public:
    HttpServlet():request(NULL),response(NULL),httpHandler(NULL){}
    virtual ~HttpServlet(){}

    /**
     * 初始化此HttpServlet，框架主动调用此方法。如果要在子类中覆盖此方法，需要在子类
     * 的init方法中调用此方法，HttpServlet::init(handler, req, resp)
     * 
     * @param handler HttpHandler
     * @param req 收到的HTTP请求，包括请求行，请求消息头和主体数据等
     * @param resp 用于向客户端输出响应数据
     */
    virtual void init(HttpHandler* handler, Request& req, Response& resp){
        request = &req;
        response = &resp;
        httpHandler = handler;
    }
    
    /** 
     * 当读Body数据时会调用此方法，在子类型中覆盖此方法来处理读到的主体数据, 如果
     * Body数据已经全部读完，就返回true，否则返回false
     * 
     * @param buff 读到的主体数据
     * @param n 本次读到的数据量
     * 
     * @return true, 已经处理此事件，false, 未处理此事件
     */
    virtual bool onReadBodyEvent(const char* buff, size_t n){
        return true;
    }
    
    /**
     * 当写队列为空时，触发此事件
     * 
     * @return true, 已经处理此事件，false, 未处理此事件
     */
    virtual bool onWriteQueueEmptyEvent(){
        return false;
    }    
    
    /**
     * 向客户端响应完所有数据后，触发此事件，主要用于异步向客户端写数据时，得知
     * 请求处理完成情况
     * 
     * @return true, 已经处理此事件，false, 未处理此事件
     */
    virtual bool onCompletionEvent(){
        return false;
    }
    
    /** 
     * 主要是被父进程通知需要关闭系统了, 然后可以此事件中处理一些后续工作，
     * 如果返回true, 说明已经准备好关闭系统了，此时HttpHandler将调用停止(HttpHandler.stop())
     * 方法
     * 
     */
    virtual bool onNotifiedCloseEvent(){
        return false;
    }
    
    /**
     * 在此方法中处理HTTP请求, 如果异步响应请求，请使用response.setAsynAnswerMode(true)设置,
     * 当设置异步处理请求后，需要调用response.complete()方法通知。请求处理完成，会自动调用
     * onCompletionEvent()方法，不管是成功响应还是出错(比如客户端断开连接等)，可在此方法中
     * 做一些清理工作
     * 
     * @param req HTTP请求
     * @param resp HTTP Response
     * 
     */
    virtual void service(Request& req, Response& resp){
    }


protected:
    Request* request;
    Response* response;
    HttpHandler* httpHandler;
};

#endif	/* _HTTPSERVLET_H */

