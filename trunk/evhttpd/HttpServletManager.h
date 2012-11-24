/* 
 * File:   HttpServletManager.h
 * Author: try
 *
 * Created on 2011年7月11日, 上午8:58
 */

#ifndef HTTPSERVLETMANAGER_H
#define	HTTPSERVLETMANAGER_H

#include <time.h>
#include <queue>
#include "ICleaner.h"
#include "HttpServletFactory.h"
#include "HttpServlet.h"

class HttpServletManager : public ICleaner{
private:
    HttpServletManager();
    virtual ~HttpServletManager();
    
public: 
    static HttpServletManager instance;
    
    
    static void init(HttpServletFactory* factory);
    
    
    HttpServlet* create(const char* path);
    
    
    void recycle(HttpServlet* servlet);
    
    /**
     * @see Cleaner#cleanup()
     */
    void cleanup();
    
private:
    typedef struct Entry{
        HttpServlet* servlet;
        time_t t; //返还时间, 单位：秒
        Entry(HttpServlet* servlet, time_t t):servlet(servlet), t(t){}
    }Entry;
    
private:
    HttpServletFactory* factory;
    std::queue<Entry*> freeQueue;
    
};

#endif	/* HTTPSERVLETMANAGER_H */

