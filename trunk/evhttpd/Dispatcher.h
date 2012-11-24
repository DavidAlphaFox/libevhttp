/* 
 * File:   Dispatcher.h
 * Author: try
 *
 * Created on 2011年5月27日, 上午9:14
 */

#ifndef _DISPATCHER_H
#define	_DISPATCHER_H


#include "libev.h"
#include <vector>
#include <queue>
#include <ext/hash_map>
#include "Process.h"
#include "HttpProcess.h"
#include "resources.h"
#include "Config.h"
#include "ICleaner.h"


/**
 * 用于管理子进程和分发HTTP请求
 */
class HttpServer;
class Dispatcher : public ICleaner{
    friend class HttpProcess;
public:
    Dispatcher();
    virtual ~Dispatcher();

    /** 
     * 启动子进程
     */
    bool start();    
    
    /**
     * 客户端连接成功，调用此方法向子进程派发连接FD
     * 
     * @param fd 客户端连接FD
     * 
     * @return true, 操作成功
     */
    bool dispatch(FD fd);

    /**
     * 向子所有进程发送消息
     */
    void sendMessage(MSG_TYPE type, void* msg, size_t len);
    
    /**
     * 通知所有子进程退出
     */    
    void notifyCloseProcesses();

    /**
     * 强行关闭所有子进程
     */
    void killProcesses();
    
    /**
     * 在httpProcesses中返回所有子进程
     */
    std::vector<HttpProcess*>& getHttpProcesses(std::vector<HttpProcess*>& httpProcesses);
    
    
    /**
     * 移除所有进程，主要用于在子进程移除从父进程中带过来的进程其它进程
     */
    void removeProcesses();    
    
    /**
     * 返回活动进程数量
     */
    int processesCount();
    
    /**
     * @see Cleaner#cleanup()
     */
    void cleanup();    
    
public:
    /** 指向HttpServer */
    HttpServer* httpServer;
    /** 指向监听Socket的ev::io */
    ev::io** evioSocket;
    /** 指向Config */
    Config* config;
    
private:
    //定义进程容器类型
    typedef std::vector<HttpProcess*> HttpProcessVector;
    typedef __gnu_cxx::hash_set<HttpProcess*, PointerHash, PointerCompare> HttpProcessSet;

    //进程总数
    int processCount;
    //所有工作进程
//    HttpProcessSet allProcesses;
    HttpProcessVector allProcesses;

//    //用于空闲和忙进程切换的容器
//    HttpProcessVector firstProcesses;
//    HttpProcessVector secondProcesses;
//    
    //将要被销毁的进程
    HttpProcessVector destroyedProcesses;
//    
//    //指向比较空闲的工作进程
//    HttpProcessVector* idleProcesses;
//    //指向比较忙的工作进程
//    HttpProcessVector* busyProcesses;
    
    
private:

    //返回一个比较空闲的进程, 并同时从空闲进程池中移除，并添加到比较忙的进程池中
    HttpProcess* getIdleProcess();
    
    //创建新进程, 返回实际成功创建数量
    int createProcess(int n);
    
    void addToDestroyedProcesses(HttpProcess* p);
    
};

#endif	/* _DISPATCHER_H */

