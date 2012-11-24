/* 
 * File:   CleanerTimer.h
 * Author: try
 *
 * Created on 2011年7月11日, 上午9:39
 */

#ifndef CLEANERTIMER_H
#define	CLEANERTIMER_H

#include <vector>
#include <ext/hash_set>
#include "libev.h"
#include "resources.h"
#include "ICleaner.h"

/**
 * 资源清理定时器
 */
class CleanerTimer {
private:
    CleanerTimer();
    virtual ~CleanerTimer();
    
public:
    static CleanerTimer instance;
    
    /**
     * 添加一个 Cleaner
     * 
     * @param c 实现了Cleaner接口的类型实例
     */
    void add(ICleaner* c);
    
    /**
     * 手动执行清理工作
     */
    void cleanupAll();
    
    /**
     * 移除Cleaner
     */
    void remove(ICleaner* c);
    
    /**
     * 停止 
     */
    void stop();
    
//    /**
//     * 清除定时器
//     */
//    void clear(){
//        cleaners.clear();
//    }
    
private:
    typedef __gnu_cxx::hash_set<ICleaner*, PointerHash, PointerCompare> CleanerSet;
//    typedef std::vector<ICleaner*> Cleaners;
    CleanerSet cleaners;
    
private:
    ev::timer timeoutTimer;
    
    void timerCallback(ev::timer& timer, int events);    
    
};

#endif	/* CLEANERTIMER_H */

