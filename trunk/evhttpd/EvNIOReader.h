/* 
 * File:   EvNIOReader.h
 * Author: try
 *
 * Created on 2011年7月2日, 下午4:12
 */

#ifndef EVNIOREADER_H
#define	EVNIOREADER_H

#include "libev.h"
#include <queue>
#include "AbstractIOReader.h"

/**
 * 通过libev库实现的异步非阻塞IO读
 */
class EvNIOReader : public AbstractIOReader{
public:
    EvNIOReader();
    EvNIOReader(FD fd);
    virtual ~EvNIOReader();
    
    /**
     * 设置FD
     */
    virtual void setFD(FD fd);  
    
    /**
     * 关闭
     * 
     * @see IIOReader#close()
     */
    virtual void close();
    
    /**
     * 检查是否已经被关闭
     * 
     * @see IIOReader#isClosed()
     */
    virtual bool isClosed();
    
    /**
     * 提交读请求
     * 
     * @see IIOReader#read(char* buff, size_t n, bool full = false, off_t offset = -1)
     */    
    virtual bool read(char* buff, size_t n, bool full = false, off_t offset = -1);
    
    /**
     * 返回还未处理的读请求数量
     * 
     * @see IIOReader#waitingQueueSize()
     */
    virtual size_t waitingQueueSize();

    /**
     * 复位
     * 
     * @see IIOReader#reset()
     */
    virtual void reset();   
    
protected:
    /**
     * 真正读操作
     */
    virtual ssize_t realRead(char* buff, size_t n, off_t offset = -1);    
    
    /**
     * ev::io读事件回调方法
     */
    virtual void readCallback(ev::io &evio, int revents);    
    
    
    //向EV分发读事件
    virtual void feedReadEvio();
    
    //停止EVIO
    virtual void stopReadEvio();
    
protected:
    FD fd; //IO描述符

protected:
    //用于读数据的EVIO事件
    ev::io readevio;

protected:
    //用于包装读请求
    typedef struct Entry{
        char* buff;  //读数据缓冲区
        bool mustfree; //如果值为true，说明buff需要手动释放
        size_t planLength; //预读数据长度
        size_t actualLength; //实际读出数据长度
        off_t pointer;   //FD位置指针，从什么位置开始读数据，如果值为-1, 表式顺序读,否则从指定位置读数据
        bool full; //指示是否要将buff读满时才触发读完成事件
        public:
        Entry()
                :buff(NULL), mustfree(false), planLength(0), actualLength(0), pointer(-1), full(false) {}
        Entry(char* buff, size_t planLength, off_t pointer, bool full, bool mustfree=false)
                :buff(buff), mustfree(mustfree), planLength(planLength), actualLength(0), pointer(pointer), full(full) {}
    }Entry;
    
    //预读的缓冲队列
    typedef std::queue<Entry*> EntryQueue;
    EntryQueue entryQueue;
    
private:
    void popEntry();
    
};

#endif	/* EVNIOREADER_H */

