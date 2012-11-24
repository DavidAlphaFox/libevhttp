/* 
 * File:   AbstractIOReader.h
 * Author: try
 *
 * Created on 2011年7月4日, 下午10:03
 */

#ifndef ABSTRACTIOREADER_H
#define	ABSTRACTIOREADER_H

#include "resources.h"
#include "IIOReader.h"
#include "event/EventDispatcher.h"
#include "events.h"

/**
 * 主要装配了EventDispatcher的相关方法
 */
class AbstractIOReader  : public IIOReader, public EventDispatcher{
public:
    AbstractIOReader():readCompletionEvent(this) {}
    virtual ~AbstractIOReader(){}
    
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
    //定义IO读完成事件
    IOReadCompletionEvent readCompletionEvent;
    
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
    bool fireReadCompletionEvent(char* buff, ssize_t rsize, off_t offset){
        readCompletionEvent.dataBuff = buff;
        readCompletionEvent.length = rsize;
        readCompletionEvent.offset = offset;        
        if(onReadCompletionEvent(&readCompletionEvent)){
           //事件已经被处理，返回
           return true; 
        }
        //把事件交给其它监听器处理
        if(fire(&readCompletionEvent)){
           //事件已经被处理，返回
           return true; 
        }
        LOG_INFO("IOReadCompletionEvent have not been processed");
        return false;
    } 
    
};

#endif	/* ABSTRACTIOREADER_H */

