/* 
 * File:   AbstractIOWriter.h
 * Author: try
 *
 * Created on 2011年7月8日, 下午3:21
 */

#ifndef ABSTRACTIOWRITER_H
#define	ABSTRACTIOWRITER_H

#include "resources.h"
#include "IIOWriter.h"
#include "event/EventDispatcher.h"
#include "events.h"

/**
 * 主要装配了EventDispatcher的相关方法
 */
class AbstractIOWriter  : public IIOWriter, public EventDispatcher{
public:
    AbstractIOWriter():writeCompletionEvent(this){}
    virtual ~AbstractIOWriter(){}
    
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
      
      
protected:
    //定义IO写完成事件
    IOWriteCompletionEvent writeCompletionEvent;

protected:
    
    virtual bool fireWriteCompletionEvent(const char* buff, ssize_t wsize, off_t offset){
        return fireWriteCompletionEvent(buff, wsize, offset, NULL);
    }
    
    /**
     * 触发写完成事件
     * 
     * @param buff 数据区
     * @param wsize 本次写入的数据量, 如果等于-1，发生错误
     * @param offset 提交读请求时的写偏移位置
     * 
     * @return true, 事件被成功处理; false, 未被处理
     */    
    virtual bool fireWriteCompletionEvent(const char* buff, ssize_t wsize, off_t offset, IIOPump* ioPumper){
        writeCompletionEvent.length = wsize;
        writeCompletionEvent.offset = offset;
        writeCompletionEvent.dataBuff = buff;
        writeCompletionEvent.ioPumper = ioPumper;
        writeCompletionEvent.isSendfile = ioPumper?true:false;
        if(onWriteCompletionEvent(&writeCompletionEvent)){
           //事件已经被处理，返回
           return true; 
        }
        //把事件交给其它监听器处理
        if(fire(&writeCompletionEvent)){
           //事件已经被处理，返回
           return true; 
        }
        LOG_INFO("IOWriteCompletionEvent have not been processed");
        return false;
    } 
    
    
private:

};

#endif	/* ABSTRACTIOWRITER_H */

