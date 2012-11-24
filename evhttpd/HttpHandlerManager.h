/* 
 * File:   HttpHandlerManager.h
 * Author: try
 *
 * Created on 2011年7月14日, 上午10:47
 */

#ifndef HTTPHANDLERMANAGER_H
#define	HTTPHANDLERMANAGER_H

#include "resources.h"
#include "libev.h"
#include <ext/hash_set>
#include "http_1.1/Http11HandlerFactory.h"
#include "ICleaner.h"
#include "HttpHandler.h"

/**
 * 管理HttpHandler
 */
class HttpHandlerManager : public ICleaner{
public:
    typedef __gnu_cxx::hash_set<HttpHandler*, PointerHash, PointerCompare> HttpHandlerSet;    
    typedef std::vector<HttpHandler*> HttpHandlerVector;
    
private:
    HttpHandlerManager();
    virtual ~HttpHandlerManager();
    
public:
    static HttpHandlerManager instance;
    
    static void init(HttpHandlerFactory* factory, int maxHttpHandlerCount);
    
    /**
     *  返回一个空闲的HttpHandler
     *  
     * @param httpProcess http进程
     * @param fd Http请求连接描述符
     */
    HttpHandler* get(HttpProcess* httpProcess, FD fd);

    /**
     * 返回所有HttpHandler
     */
    const HttpHandlerSet* getAllHttpHandlers(){
        return &allHttpHandlers;
    }
    
    /** 回收HttpHandler */
    void recycle(HttpHandler* handler);

    /** 返回活动Handler数量 */
    int getActiveCount();
    
    /** 返回所有Handler数量 */
    int getAllCount();
    
    /** 清除多余或超时的Handler，超时功能暂未实现 */
    void cleanup();    
    
    /** 清除所有空闲Handler */
    void cleanupAll();   
    
    /** 通知所有Handler关闭s */
    void notifyCloseAll();
    
    /** 强行关闭未结束的Handler */
    void closeAll();
    
private:
    void cleanup(int keepCount);        
    
private:
    HttpHandlerSet allHttpHandlers;
    HttpHandlerSet idleHttpHandlers;    
    
private:
    Http11HandlerFactory defaultHttpHandlerFactory;
    HttpHandlerFactory* httpHandlerFactory;    
    int maxHttpHandlerCount;
};

#endif	/* HTTPHANDLERMANAGER_H */

