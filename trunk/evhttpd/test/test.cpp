
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <signal.h>

#include "../HttpServer.h"
#include "../Config.h"
#include "../SocketUtils.h"
#include "../http_1.1/Http11HandlerFactory.h"
#include "../HttpServlet.h"
#include "../HttpServletFactory.h"
#include "../ProcessSignal.h"
#include "../CleanerTimer.h"
#include "../Timer.h"
#include "../HttpHandlerManager.h"

#include "HelloHttpServlet.h"
#include "ReadFormBodyHttpServlet.h"
#include "SendfileIOPumpHttpServlet.h"
#include "RWIOPumpHttpServlet.h"
#include "FileServerHttpServlet.h"



#define SERVER_PORT 3080

HttpServer* httpServer;

static void mainSignalHandler(int sno) {
    LOG_WARN("main pid is %d, signal id is %d", getpid(), sno);
    httpServer->getDispatcher()->notifyCloseProcesses();
    exit(sno);
}

static void childSignalHandler(int sno) {
    LOG_WARN("child pid is %d, signal id is %d", getpid(), sno);
    HttpHandlerManager::instance.notifyCloseAll();
    LOG_INFO("ChildProcessSignal, getAllCount:%d, getActiveCount:%d",  HttpHandlerManager::instance.getAllCount(), HttpHandlerManager::instance.getActiveCount());
    exit(sno);
}


void setMainSignalHandler(){
//    signal(SIGPIPE, SIG_IGN);
    
    signal(SIGABRT, mainSignalHandler);
    signal(SIGFPE, mainSignalHandler);
    signal(SIGILL, mainSignalHandler);
    signal(SIGINT, mainSignalHandler);
    //signal(SIGHUP, mainSignalHandler);
    signal(SIGSEGV, mainSignalHandler);
    signal(SIGTERM, mainSignalHandler);
    signal(SIGSTOP, mainSignalHandler);
    signal(SIGTSTP, mainSignalHandler);
    /* 处理kill信号 */
    signal(SIGKILL, mainSignalHandler);
    signal(SIGQUIT, mainSignalHandler);
    signal(SIGHUP, mainSignalHandler);
    /* 处理定时更新修改的数据到磁盘信号 */
    signal(SIGALRM, mainSignalHandler);
}


void setChildSignalHandler(){
//    signal(SIGPIPE, SIG_IGN);
    
    signal(SIGABRT, childSignalHandler);
    signal(SIGFPE, childSignalHandler);
    signal(SIGILL, childSignalHandler);
    signal(SIGINT, childSignalHandler);
    //signal(SIGHUP, childSignalHandler);
    signal(SIGSEGV, childSignalHandler);
    signal(SIGTERM, childSignalHandler);
    signal(SIGSTOP, childSignalHandler);
    signal(SIGTSTP, childSignalHandler);
    /* 处理kill信号 */
    signal(SIGKILL, childSignalHandler);
    signal(SIGQUIT, childSignalHandler);
    signal(SIGHUP, childSignalHandler);
    /* 处理定时更新修改的数据到磁盘信号 */
    signal(SIGALRM, childSignalHandler);
}


class TestHttpServletFactory : public HttpServletFactory{
public:
    TestHttpServletFactory(){
    
    }
    virtual ~TestHttpServletFactory(){}

    virtual HttpServlet* create(const char* path){
        if(strcmp(path, "/hello") == 0){
            return new HelloHttpServlet();
            
        }else if(strcmp(path, "/form") == 0){
            return new ReadFormBodyHttpServlet();
            
        }else if(strcmp(path, "/upload") == 0){
            return new ReadFormBodyHttpServlet();
            
        }else if(strncmp(path, "/sf/", 4) == 0){
            return new SendfileIOPumpHttpServlet();
            
        }else if(strncmp(path, "/rw/", 4) == 0){
            return new RWIOPumpHttpServlet();
            
        }else if(strncmp(path, "/file/", 6) == 0){
            return new FileServerHttpServlet();
            
        }
        
        

        return new HelloHttpServlet();
    }
    
    void free(HttpServlet* servlet){
        delete servlet;
    }
    
};

#include <stdio.h>
#include <iostream>

TestHttpServletFactory servletFactory;


class TestProcessSignal : public ProcessSignal , public Timer{
public:
    TestProcessSignal(){
            addSignal(SIGINT);
            addSignal(SIGQUIT);
//            addSignal(SIGKILL);
            addSignal(SIGABRT);
            addSignal(SIGFPE);
            addSignal(SIGILL);
            addSignal(SIGSEGV);
            addSignal(SIGTERM);
//            addSignal(SIGSTOP);
            addSignal(SIGTSTP);
            addSignal(SIGHUP);
            addSignal(SIGALRM);
    };
    virtual ~TestProcessSignal(){
        ProcessSignal::stop();
        Timer::stop();
    };
    
    void start(){
        ProcessSignal::start();
    }
    
    void stop(){
        ProcessSignal::stop();
    }    
    
};

class MainProcessSignal : public TestProcessSignal{
public:    
    MainProcessSignal():repeat(0){
    }
    virtual ~MainProcessSignal(){

    }
    
protected:    
    virtual bool onProcessSignalEvent(const ProcessSignalEvent* e){
        LOG_INFO("main, stop server, signal:%d, repeat:%d", e->signal, repeat);

        if(repeat == 0){
            if(httpServer){
                httpServer->getDispatcher()->notifyCloseProcesses();
                ProcessSignal::stop();
//                exit(1);                
                Timer::start(0.25); 
            }
        }
        return true;
    }    
    
    
    virtual bool onTimerEvent(const TimerEvent* e){
        if(!httpServer)return true;
        LOG_INFO("");
        int processesCount = httpServer->getDispatcher()->processesCount();
        LOG_INFO("processCount:%d, repeat:%d", processesCount, repeat);
        if(processesCount <= 0 || repeat > 40){
            if(processesCount > 0){
                httpServer->getDispatcher()->killProcesses();
            }
            ProcessSignal::stop();
            Timer::stop();
            CleanerTimer::instance.stop();
            httpServer->stop();
            exit(0);
            return true;
        }else{
            repeat++;
            httpServer->getDispatcher()->notifyCloseProcesses();
            Timer::start(0.25);
            return true;
        }
    }    
    
private:
    int repeat;
};



class ChildProcessSignal : public TestProcessSignal{
public:    
    ChildProcessSignal():repeat(0){
    }
    virtual ~ChildProcessSignal(){

    };
    
protected:    
    bool onProcessSignalEvent(const ProcessSignalEvent* e){
        LOG_INFO("child, stop server, signal:%d", e->signal);
        if(repeat == 0){
            executeClose();
        }
        return true;
    }      
    
    virtual bool onTimerEvent(const TimerEvent* e){
        executeClose();
        return true;
    }
    
    void executeClose(){
        HttpHandlerManager::instance.notifyCloseAll();
        LOG_INFO("ChildProcessSignal, getAllCount:%d, getActiveCount:%d, repeat:%d",  HttpHandlerManager::instance.getAllCount(), HttpHandlerManager::instance.getActiveCount(), repeat);
        if(HttpHandlerManager::instance.getAllCount() <= 0 || repeat > 20){
            Timer::stop();
            CleanerTimer::instance.stop();
            ProcessSignal::stop();
            exit(0);
            return;
        }
        repeat++;
        Timer::start(0.25);
        
    }
private:
    int repeat;
};

MainProcessSignal mainProcessSignal;
ChildProcessSignal childProcessSignal;


class TestEventListener : public IEventListener, public Timer{
public:
    TestEventListener(){
    
    };
    virtual ~TestEventListener(){

    };

    virtual bool handle(const Event* e){
        switch(e->getType()){
            case EVENT_TYPE_PROCESS_STARTED:
                return onProcessStartedEvent((const ProcessStartedEvent*)e);
        }
        return false;
    }
    
    
    bool onProcessStartedEvent(const ProcessStartedEvent* e){
        LOG_INFO("e->inChildProcess:%d", e->inChildProcess);
        //因为子进程会继承父进程的资源，所以在子进程启动后需要清理不需要的
        if(e->inChildProcess){
            mainProcessSignal.stop();
            childProcessSignal.start();
            //setChildSignalHandler();
        }else{
            mainProcessSignal.start();
            //setMainSignalHandler();
        }
        
        return true;
    }
    
private:
};

TestEventListener testEventListener;

int main(int argc, char** argv) {
    
    Config config;
    config.workProcessCount = 3;
    config.poolMaxHttpHandlerCount = 0;
//    config.requestBufferSize = 20;
//    config.responseBufferSize = 10;
    config.eventListener = &testEventListener;
 
    //配置HttpServer
    
    httpServer = new HttpServer(SERVER_PORT, &servletFactory, &config);
    //httpServer = &httpServer;
    
    //启动HttpServer
    if(!httpServer->start()){
        return 1;
    }
    
    //进入EV事件循环
    HttpServer::loop();

}
