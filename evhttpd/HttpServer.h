/* 
 * File:   HttpServer.h
 * Author: try
 *
 * Created on 2011年6月6日, 下午12:24
 */

#ifndef _HTTPSERVER_H
#define	_HTTPSERVER_H

#include "libev.h"
#include <vector>
#include <netinet/in.h>
#include "resources.h"
#include "Config.h"
#include "Dispatcher.h"
#include "HttpServlet.h"
#include "HttpHandlerFactory.h"
#include "HttpServletFactory.h"

/**
 * 默认HttpServletFactory
 */
class DefaultHttpServletFactory : public HttpServletFactory{
public:
    DefaultHttpServletFactory(HttpServlet* servlet):servlet(servlet){}
    virtual ~DefaultHttpServletFactory(){}

    /**
     * @see HttpServletFactory#create(const char* path)
     */
    virtual HttpServlet* create(const char* path){
        return servlet;
    }

    /**
     * @see HttpServletFactory#free(HttpServlet* servlet)
     */
    void free(HttpServlet* servlet){}

public:
    HttpServlet* servlet;
};

/**
 * 创建此类型来启动一个HTTP服务
 */
class HttpServer {
public:
    /**
     * 
     * @param port 监听端口
     * @param servletFactory HttpServlet工厂，通过servletFactory来创建处理HTTP请求的HttpServlet
     * @param config 配置HTTP服务
     */
    HttpServer(unsigned short int port, HttpServletFactory* servletFactory, Config* config = NULL);
    
    /**
     * 
     * @param port 监听端口
     * @param servlet 指定一个HttpServlet来处理HTTP请求
     * @param config 配置HTTP服务
     */
    HttpServer(unsigned short int port, HttpServlet* servlet, Config* config = NULL);
    
    /**
     * 
     * @param address 服务绑定地址
     * @param servletFactory servletFactory HttpServlet工厂，通过servletFactory来创建处理HTTP请求的HttpServlet
     * @param config 配置HTTP服务
     */
    HttpServer(sockaddr_in* address, HttpServletFactory* servletFactory, Config* config = NULL);
    
    /**
     * 
     * @param address 服务绑定地址
     * @param servlet servletFactory HttpServlet工厂，通过servletFactory来创建处理HTTP请求的HttpServlet
     * @param config 配置HTTP服务
     */
    HttpServer(sockaddr_in* address, HttpServlet* servlet, Config* config = NULL);
    virtual ~HttpServer();

    /** 启动Http服务 */
    bool start();

    /* 停止服务, 向子服务进程发送通知退出消息，如果有子进程的话 */
    void stop();
    
    /** 进入EV事件循环 */
    static void loop(int flags = 0);
    

    static void  unloop(ev::how_t how  = ev::ONE);
    
    /**
     * 返回Dispatcher
     */
    Dispatcher* getDispatcher(){
        return &dispatcher;
    }

    /**
     * 返回HttpServlet工厂
     */
    HttpServletFactory* getHttpServletFactory(){
        return servletFactory;
    }
    
private:
    
    /**
     * 监听Http连接请求
     * @param evio
     * @param revents
     */
    void acceptCallback(ev::io &evio, int revents);
    
private:
    HttpServletFactory* servletFactory;
    DefaultHttpServletFactory* defaultServletFactory;

    Dispatcher dispatcher;
    FD socketfd;
    sockaddr_in* address;
    sockaddr_in defaultAddress;
    ev::io evioSocket;
    ev::io* evioSocketPt;
    
    Config* config;
    Config defaultConfig;
    
private:
    void init();

};

#endif	/* _HTTPSERVER_H */

