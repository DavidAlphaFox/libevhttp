/* 
 * File:   HttpProcess.h
 * Author: try
 *
 * Created on 2011年5月27日, 上午10:34
 */

#ifndef _HTTPPROCESS_H
#define	_HTTPPROCESS_H

#include "libev.h"
#include <ext/hash_map>
#include <queue>

#include "resources.h"
#include "Request.h"
#include "Response.h"
#include "Process.h"
#include "Config.h"
#include "HttpHandler.h"
//#include "MessageListener.h"
#include "SocketNIOReader.h"
#include "SocketNIOWriter.h"


/**
 * Http进程
 */

class Dispatcher;
class HttpServer;
class HttpProcess : public Process{
friend class HttpHandler;
friend class Dispatcher;
public:
    HttpProcess(Dispatcher* dispatcher);
    virtual ~HttpProcess();

    /* 下面是在父进程中使用的方法 */
public:
    
    /**
     * 查询当前正在处理的请求数量, 当在事件监听器中收到查询响应后可调用
     * getActiveHandlerCount方法获取值
     */
    bool queryActiveHandlerCount();
    
    /**
     * 查询所有已经处理的请求数量, 当在事件监听器中收到查询响应后可调用
     * getTotalRequestCount方法获取值
     */
    bool queryTotalRequestCount();    
    
    /**
     * 查询所有HttpHandler数量, 当在事件监听器中收到查询响应后可调用
     * getHttpHandlerCount方法获取值
     */
    bool queryHttpHandlerCount();    
    
    /**
     * 返回近似活动请求数量
     */
    unsigned int getActiveHandlerCount(){
        return activeHandlerCount;
    }
    
    /**
     * 返回已经完成的请求总数
     */
    unsigned long getTotalRequestCount(){
        return totalRequestCount;
    }
    
    /**
     * 返回近似所有HttpHander数量
     */
    unsigned int getHttpHandlerCount(){
        return httpHandlerCount;
    }
    
    /** 添加一个任务, 向子进程发送一个Http连接请求 */
    bool addTask(FD fd);    

    
    /**
     * 关闭相关资源
     */
    virtual void close(); 
    
    /**
     * 通知关闭
     */
    virtual void notifyClose();
    
    
    
public:
    /** 指向Dispatcher */
    Dispatcher* dispatcher;
    /** 指向HttpServer */
    HttpServer* httpServer;
    /** 指向主socket事件 */
    ev::io** evioSocket;
    /** 指向Config */
    Config* config;
    
protected:
    /** 
     * 子进程从这里开始执行 
     * 
     * @see Process#run(void* param=NULL)
     */
    virtual int run(void* param=NULL);
    
protected:
    
    /**
     * 发送消息完成事件
     */
    virtual bool onSendMessageEvent(const ProcessSendMessageEvent* e);
    
    /**
     * 接收消息完成事件
     */
    virtual bool onReceiveMessageEvent(const ProcessReceiveMessageEvent* e);     
    
    
/* 仅用于父进程的属性 */
private:
    //活动Handler数量， 当调用了queryActiveHandlerCount方法后会刷新此数据
    //活动handlerCount数量
    unsigned int activeHandlerCount;
    //已经完成的所有请求数量
    unsigned long totalRequestCount;
    //近似所有HttpHander数量， 当调用了queryHttpHandlerCount方法后会刷新此数据
    unsigned int httpHandlerCount; 
    
private:
    //用于监听从父进程中发送来的描述符FD
    ev::io fdReadEvio;

private:
    bool parentEvioStarted; //标识监听子进程事件的ioev是否被启动
    
private:

    //读父进程发送来的FD
    void fdReadCallback(ev::io &evio, int revents);
    
    bool handleMessageInChild(ProcessMessage_t* message);
    
    bool handleMessageInParent(ProcessMessage_t* message);
    
    
    //退出进程
    void exitProcess();
  
    virtual void executeClose();

};

#endif	/* _HTTPPROCESS_H */

