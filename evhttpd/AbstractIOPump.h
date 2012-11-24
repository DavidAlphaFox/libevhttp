/* 
 * File:   AbstractIOPump.h
 * Author: try
 *
 * Created on 2011年7月14日, 下午3:43
 */

#ifndef ABSTRACTIOPUMP_H
#define	ABSTRACTIOPUMP_H

#include "events.h"
#include "event/EventDispatcher.h"
#include "IIOPump.h"



/**
 * IIOPump接口的常规方法实现，以及装配了EventDispatcher的相关方法
 */

class AbstractIOPump: public EventDispatcher, public IIOPump{
public:
    AbstractIOPump();
    virtual ~AbstractIOPump();
    
      /**
       * @see IEventDispatcher#addListener(EventType type, IEventListener* l, int pos = -1)
       */
      virtual void addListener(EventType type, IEventListener* l, int pos = -1){
          EventDispatcher::addListener(type, l, pos);
      }

      /**
       * @see IEventDispatcher#addListener(IEventListener* l, int pos=-1)
       */
      virtual void addListener(IEventListener* l, int pos=-1){
          EventDispatcher::addListener(l, pos);
      }

      /**
       * @see IEventDispatcher#removeListener(IEventListener* l)
       */
      virtual void removeListener(IEventListener* l){
          EventDispatcher::removeListener(l);
      }

      /**
       * @see IEventDispatcher#fire(const Event* e, bool asyn = false)
       */
      virtual bool fire(const Event* e, bool asyn = false){
          return EventDispatcher::fire(e, asyn);
      }

      /**
       * @see IEventDispatcher#handleAll()
       */
      virtual void handleAll(){
          EventDispatcher::handleAll();
      }
    
    /**
     * 检查是否已经被关闭
     */
    virtual bool isClosed(){
        return status == CLOSED;
    }
      
    /**
     * 检查是否正运行中
     */
    virtual bool isRunning(){
        return status == RUNNING;
    }
    
    /**
     * 检查是否正准备状态
     */
    virtual bool isReady(){
        return status == NEW;
    }
    
    /**
     * 设置从什么位置开始读, 默认为:-1, 从IO当前位置开始读，
     * 如果是打开的文件，从文件第0位置开始读
     * 
     */
    virtual void setOffset(off_t offset){
        this->offset = offset;
    }
    
    /**
     * 设置最大传输长度，默认为：0， 表示将所有数据传完或出错为止
     * 
     * @param len 最大传输数据量
     */
    virtual void setLength(size_t len){
        this->length = len;
    };
    
protected:
    /** 状态 */
    enum STATUS{
        //准备状态
        NEW = 1,
        //正运行状态
        RUNNING = 2,
        //已经停止状态
        CLOSED=3
    };
      
protected:
    //定义传输事件
    IOTransportEvent transportEvent;
    
    // 管道状态
    STATUS status;
    
    //读位置
    off_t offset;
    
    //预传输的最大数据长度
    size_t length;
    
protected:
    
    /**
     * 发送管道传输事件
     * 
     * @param readEvent IO读完成事件
     * @param writeEvent IO写完成事件
     * @return 
     */
    virtual bool fireTransportEvent(const IOReadCompletionEvent* readEvent, const IOWriteCompletionEvent* writeEvent);   

    /**
     *  根据计划传输速度计算读数据频率和每次读数据量
     * 
     * @param speed 计划传输速度, 如果值小于等于0表示无限制
     * @param timeoutInterval 在此返回读数据频率
     * @param readBytesPerEach 在此返回每次读多少数据量
     * @param buffSize 数据缓冲区大小
     */
    virtual void calculateSpeed(double speed, double* timeoutInterval, size_t* readBytesPerEach, size_t buffSize);

};

#endif	/* ABSTRACTIOPUMP_H */

