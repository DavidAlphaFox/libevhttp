/*
 * EventDispatcher.h
 *
 *  Created on: 2010-8-16
 *      Author: wenjian
 */

#ifndef EVENTDISPATCHER_H_
#define EVENTDISPATCHER_H_

#include <ext/hash_map>
#include "Event.h"
#include "IEventDispatcher.h"


//namespace Try {
  class IEventListener;

  /**
   * 事件Dispatcher, 比较简单的事件处理, 只支持同步分发处理事件
   */
  class EventDispatcher : public IEventDispatcher{
    public:
      EventDispatcher();
      virtual ~EventDispatcher();

      /**
       * 注册事件监听器, type为事件类型, 此监听器将监听类型为type的事件。 pos为监听器顺序位置
       */
      virtual void addListener(EventType type, IEventListener* l, int pos=-1);

      /**
       * 注册事件监听器, 此监听器将监听所有事件
       */
      virtual void addListener(IEventListener* l, int pos=-1);

      /**
       * 移除监听器
       */
      virtual void removeListener(IEventListener* l);

      /**
       * 移除所有事件监听器
       */
      virtual void removeAllListeners();
      
      /**
       * 触发一个事件, 在触发事件时默认同步执行事件监听器。 asyn为true将异步
       * 处理事件，此时需要调用handleAll方法来处理事件,异步事件返回值无意义，
       * 同步事件返回值为true说明至少有一个监听器响应了事件
       */
      virtual bool fire(const Event* e, bool asyn = false);

      /**
       * 处理所有异步事件, 处理完成后将清除事件
       */
      virtual void handleAll();

    protected:
      //所有异步事件
      std::vector<const Event*> allAsynEvents;

      //所有具体事件类型相关监听器
      __gnu_cxx::hash_map<int, std::vector<IEventListener*>*> allTypeListeners;
      typedef __gnu_cxx::hash_map<int, std::vector<IEventListener*>*>::iterator ListenerMapIterator;
      typedef std::vector<IEventListener*>::iterator ListenerIterator;

      //监听所有事件的事件监听器
      std::vector<IEventListener*> allEventListeners;

      //异步事件锁
//      CriticalLock aeLock;

    private:

      /**
       * 处理一个事件
       */
      bool handle(const Event* e);
  };

//}

#endif /* EVENTDISPATCHER_H_ */
