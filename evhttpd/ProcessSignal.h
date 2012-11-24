/* 
 * File:   ProcessSignal.h
 * Author: dell
 *
 * Created on 2012年4月6日, 上午9:05
 */

#ifndef PROCESSSIGNAL_H
#define	PROCESSSIGNAL_H

#include <vector>
#include "libev.h"
#include "resources.h"
#include "events.h"
#include "event/EventDispatcher.h"

#include <ext/hash_map>
using __gnu_cxx::hash;
using __gnu_cxx::hashtable;
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_multimap;

/**
 * 接收进程信号, 生成信号事件, 可同时监听多种信号
 */
class ProcessSignal : public EventDispatcher{
public:
    ProcessSignal();
    virtual ~ProcessSignal();
    
    /**
     * 添加监听的信号, 可以反复添加信号
     */
    void addSignal(int sig);
    
    void start();
    
    void stop();
       
    
protected:
    //定义IO读完成事件
    ProcessSignalEvent processSignalEvent;
    
protected:
    /**
     * 触发读完成事件
     * 
     * @param buff 数据区
     * @param rsize 本次读出的数据量
     * @param offset 提交读请求时的读偏移位置
     * 
     * @return true, 事件被成功处理; false, 未被处理
     */
    bool fireProcessSignalEvent(int signal);     
    
    /**
     * 可在子类型中使用重载此方法来处理信号
     */
    virtual bool onProcessSignalEvent(const ProcessSignalEvent* e){
        return false;
    }
    
protected:
    typedef hash_map<ev::sig*, int, PointerHash, PointerCompare> EVSignals;
    //所有信号事件
    EVSignals evSignals;
    
    
private:
    void sigCallback(ev::sig &ev, int revents);
    
};

#endif	/* PROCESSSIGNAL_H */

