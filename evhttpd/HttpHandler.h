/* 
 * File:   HttpHandler.h
 * Author: wenjian
 *
 * Created on 2011年5月4日, 下午4:06
 */

#ifndef _HTTPHANDLER_H
#define	_HTTPHANDLER_H
#include "libev.h"
#include <sys/types.h>
#include <queue>
#include <deque>
//#include "Queue.h"
#include "resources.h"
#include "RequestData.h"
#include "Request.h"
#include "Response.h"
#include "HttpServlet.h"

#include "SocketNIOReader.h"
#include "SocketNIOWriter.h"
#include "event/IEventListener.h"

/**
 * 接收及分析HTTP请求行，消息头，主体数据等
 */
class HttpProcess;
class HttpServletFactory;
class HttpHandler : protected IEventListener{
    friend class HttpProcess;
    friend class Response;
    friend class HttpHandlerManager;
public:
    //请求状态
    enum STATUS{
        //准备状态
        NEW = 1,
        //正处理请求状态
        RUNNING = 2,
        //正准备停止状态，停止服务过程中...， 主要用于等待发送还未发送完的数据
        STOPPING = 3,
        //正准备Clean状态
        CLEANING=4,
        //正应答过程中
        ANSWERING=5,
        //已经停止
        STOP=6
    };
    
public:
    HttpHandler();
    virtual ~HttpHandler();

    /**
     * 返回Handler当前状态
     */
    virtual HttpHandler::STATUS getStatus();    

    /**
     * 检查是否RUNNING状态
     */
    virtual bool isRunning();
    
    /**
     * 检查HTTP连接是否已经关闭
     */
    virtual bool isClosed();
    
    /**
     * 通知关闭, 然后可通过，isClosed检查, 可通过close强制关闭，
     * 也可以子类型中重载notifyClose来在Handler中拦截通知关闭消息
     */
    virtual void notifyClose();
    
    /**
     * 强制关闭
     */
    virtual void close();
    
    /**
     * 设置是否启用连接空闲超时， 如果是异步响应请求，并且不是调用response的
     * 写方法来发送数据时，比如直接对socket进行操作等，需要调用此方法关闭空闲超时
     * 
     * @param b 是否启用连接空闲超时，ture为启用，false为关闭
     */
    virtual void setIdleTimeout(bool b);
    
    /**
     * 返回客户端Socket描述符
     */
    FD getClientFD(){
        return fd;
    }
    
    /**
     * 返回此Handler所在进程
     */
    HttpProcess* getHttpProcess(){
        return httpProcess;
    }
    
protected:
    virtual void init(HttpProcess* httpProcess, FD fd);

    virtual void start();

    virtual void clean();

    virtual void stop();
    
    virtual void error();

protected:

    /**
     * 处理Socket读完成事件
     */
    virtual void handleReadCompletionEvent(IOReadCompletionEvent* e);
    
    /**
     * 处理iowriter写完成事件
     */
    virtual void handleWriteCompletionEvent(IOWriteCompletionEvent* e);
    
//    /**
//     * 处理iowriter写移除队列元素时事件
//     */    
//    virtual void handleRemoveWriteQueueElementEvent(IOWriteRemoveQueueElementEvent* e);
    
//    /** 向socket写数据，返回写入数据大小 */
//    virtual int writeData(const char* buff, unsigned int n);

    /** 准备请求数据 */
    virtual bool prepareRequestData();

    /** 准备请求行 */
    virtual bool prepareRequestLine();

    /** 准备消息头 */
    virtual bool prepareRequestHeaders();

    /** 准备请求主体数据 */
    virtual bool prepareRequestBody();    
    
    /** 连接空闲事件，等待超时 */
    virtual void timeoutCallback(ev::timer &ev, int revents);

    /** 当请求行和消息头准备好后会进入此方法, 返回true说明请求已经被处理,
     * 否则，说明还未处理完 
     */
    virtual bool handleRequest();

//    /** 最终由客户程序处理请求的入口 */
//    virtual bool service(Request& req, Response& resp);

    /** 向客户端响应数据 */
    virtual void answer();

//--事件-----------------------------------------------------------------------
protected:
    /** 在成功读入数据后调用此方法, 参数为本次读入的数据量
     * 当此事件已经被处理，返回true，如果返回false, 说明此事件还未被处理过 */
    virtual bool onReadEvent(unsigned int n);

    /** 当请求行准备好后，发送此事件
     * 当此事件已经被处理，返回true，如果返回false, 说明此事件还未被处理过 */
     virtual bool onRequestLineEvent();

    /** 当消息头准备好后，发送此事件
     * 当此事件已经被处理，返回true，如果返回false, 说明此事件还未被处理过 */
     virtual bool onRequestHeadersEvent();

    /** 当HttpCode被设置时产生此事件, 即httpCode!=0时 */
    virtual bool onHttpCodeEvent();

    /** 当请求处理完成后产生此事件，在HttpHandler处理完请求后需要需要显示的调用此方法 */
    virtual bool onCompleteEvent();

//    /** 主要是被父进程通知需要关闭系统了, 然后可以此事件中处理一些后续工作 */
//    virtual bool onNotifiedEvent();
    
//--处理事件-----------------------------------------------------------------------
protected:
    /** 处理监听到的事件, 监听的事件有:EVENT_TYPE_IO_READ_COMPLETION */
    virtual bool handle(const Event* e);
    
protected:
    STATUS status;
    //客户端Socket描述符
    FD fd;
    //用于读数据的IIOReader
    SocketNIOReader ioreader;
    //用于写数据的IIOWriter
    SocketNIOWriter iowriter;
    
    //超时处理事件
    ev::timer evtimer;
    //是否启动连接空闲超时标记, 默认:true(启用空闲超时)
    bool idleTimeout;
    //指向HttpProcess
    HttpProcess* httpProcess;
    //请求数据读缓冲
    char* readBuff;
    //请求数据
    RequestData requestData;
    //请求
    Request request;
    //响应
    Response* response;

    //HTTP状态码, 值小于0,说明出错了并且此错误不能返回给客户端，大于0即为HTTP状态码，
    //将根据HTTP状态码值进行处理，等于0说明一切正常，默认为0
    unsigned short int  httpCode;
    //第一进入handle()方法时创建HttpServlet
    HttpServlet* servlet;

    //当前已经被读出的主体数据字节数量
    size_t bodyDataBytes;
    
    //请求处理完成标记
    bool completeFlag;
    
private:
    
    void callServletOnWriteQueueEmptyEvent();
    
    //启动超时EV事件
    void startTimeoutEv();
    //停止超时EV事件
    void stopTimeoutEv();
    
    //提交读Socket数据请求
    bool prepareRead();
    
    //暂停读Socket数据
    void stopRead();    
    
    void startWrite();
    
    void stopWrite();
};

#endif	/* _REQUESTHANDLER_H */

