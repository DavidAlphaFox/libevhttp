/* 
 * File:   HttpServer.cpp
 * Author: try
 * 
 * Created on 2011年6月6日, 下午12:24
 */

#include "SocketUtils.h"
#include "HttpServer.h"
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>

using namespace std;

HttpServer::HttpServer(unsigned short int port, HttpServletFactory* servletFactory, Config* config) :
    address(&defaultAddress), servletFactory(servletFactory), defaultServletFactory(NULL), 
        socketfd(0), evioSocketPt(&evioSocket), config(config?config:&defaultConfig){

    memset(&defaultAddress, 0, sizeof(defaultAddress));
    defaultAddress.sin_family = AF_INET;
    defaultAddress.sin_addr.s_addr = INADDR_ANY;
    defaultAddress.sin_port = htons(port);
    
    init();
}

HttpServer::HttpServer(unsigned short int port, HttpServlet* servlet, Config* config) :
    address(&defaultAddress), socketfd(0), evioSocketPt(&evioSocket), 
        config(config?config:&defaultConfig){
    defaultServletFactory = new DefaultHttpServletFactory(servlet);
    servletFactory = defaultServletFactory;

    memset(&defaultAddress, 0, sizeof(defaultAddress));
    defaultAddress.sin_family = AF_INET;
    defaultAddress.sin_addr.s_addr = INADDR_ANY;
    defaultAddress.sin_port = htons(port);
    
    init();
}

HttpServer::HttpServer(sockaddr_in* address, HttpServletFactory* servletFactory, Config* config) :
    address(address), servletFactory(servletFactory), defaultServletFactory(NULL), 
        socketfd(0), evioSocketPt(&evioSocket), config(config?config:&defaultConfig){

    init();
}

HttpServer::HttpServer(sockaddr_in* address, HttpServlet* servlet, Config* config) :
    address(address), socketfd(0), evioSocketPt(&evioSocket), config(config?config:&defaultConfig){
    defaultServletFactory = new DefaultHttpServletFactory(servlet);
    servletFactory = defaultServletFactory;
    init();
}

HttpServer::~HttpServer() {
    if(defaultServletFactory)delete defaultServletFactory;
}


void HttpServer::init(){

    //设置请求分发程序相关必须参数
    dispatcher.httpServer = this;
    dispatcher.evioSocket = &evioSocketPt;
    dispatcher.config = config;

    //忽略一些会让程序中止的信号
    signal(SIGPIPE, SIG_IGN);
}

void HttpServer::stop(){
    evioSocket.stop();
    if(socketfd > 0){
        ::close(socketfd);
        socketfd = 0;
    }
}


bool HttpServer::start(){

    //生成Socket套节字
    socketfd = socket(AF_INET, SOCK_STREAM, 0); //SOCK_CLOEXEC SOCK_STREAM
    if (socketfd < 0){
        LOG_ERROR("create socket failure, errno:%d", errno)
        return false;
    }
    
    if(!SocketUtils::setReuseaddr(socketfd)){
        LOG_ERROR("set reuseaddr failure, errno:%d", errno)
        return false;
    }

    //将套节字与地址绑定
    if (bind(socketfd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        LOG_ERROR("bind failure, port:%d, errno:%d", ntohs(address->sin_port), errno)
        return false;
    }
    
    //在此套节字上监听
    if (listen(socketfd, 1024) < 0){
        LOG_ERROR("socket listen failure, errno:%d", errno)
        return false;
    }

    //设置Socket模式，设置为非阻塞模式
    if(SocketUtils::setNonblock(socketfd) != 0){
        LOG_ERROR("set nonblock failure, errno:%d", errno)
        return false;
    }

    //使用libev框架监听Socket事件
    //启动socket监听服务
    evioSocket.set<HttpServer, &HttpServer::acceptCallback>(this);
    evioSocket.start(socketfd, EV_READ);

    //启动事件监听
    if(!dispatcher.start()){
        LOG_ERROR("start dispatcher failure, errno:%d", errno)
        return false;
    }

    return true;
}


void HttpServer::acceptCallback(ev::io &evio, int revents){
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    //LOG_WARN("evio.fd:%d", evio.fd);
    FD clientfd = accept(evio.fd,(struct sockaddr *)&clientAddr, &clientLen);
    if (clientfd < 0){
        LOG_WARN("accept failure, socketfd:%d, clientfd:%d, errno:%d", socketfd, clientfd, errno);
        return;
    }

    if(!dispatcher.dispatch(clientfd)){
        LOG_WARN("dispatch failure, errno:%d", errno);
    }
    
}


void  HttpServer::loop(int flags){
    ev::loop_ref loop = ev::get_default_loop();
    loop.run(flags);
} 

void  HttpServer::unloop(ev::how_t how /* = ONE */){
    ev::loop_ref loop = ev::get_default_loop();
    loop.unloop(how);
} 

