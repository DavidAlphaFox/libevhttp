/* 
 * File:   IEventDispatcher.h
 * Author: try
 *
 * Created on 2011年7月4日, 上午12:09
 */

#ifndef IEVENTDISPATCHER_H
#define	IEVENTDISPATCHER_H

#include "Event.h"

//namespace Try {
    class IEventListener;
    
    class IEventDispatcher {
    public:
        IEventDispatcher(){}
        virtual ~IEventDispatcher(){}

      /**
       * 注册事件监听器, type为事件类型, 此监听器将监听类型为type的事件。 pos为监听器顺序位置
       */
      virtual void addListener(EventType type, IEventListener* l, int pos=-1)=0;

      /**
       * 注册事件监听器, 此监听器将监听所有事件
       */
      virtual void addListener(IEventListener* l, int pos=-1)=0;

      /**
       * 移除监听器
       */
      virtual void removeListener(IEventListener* l)=0;

      /**
       * 触发一个事件, 在触发事件时默认同步执行事件监听器。 asyn为true将异步
       * 处理事件，此时需要调用handleAll方法来处理事件,异步事件返回值无意义，
       * 同步事件返回值为true说明至少有一个监听器响应了事件
       */
      virtual bool fire(const Event* e, bool asyn = false)=0;

      /**
       * 处理所有异步事件, 处理完成后将清除事件
       */
      virtual void handleAll()=0;

    private:

    };
//}
#endif	/* IEVENTDISPATCHER_H */

