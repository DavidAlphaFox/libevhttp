/*
 * Event.h
 *
 *  Created on: 2010-8-16
 *      Author: wenjian
 */

#ifndef EVENT_H_
#define EVENT_H_
#include "stdlib.h"

//namespace Try {
  /**
   * 事件类型, 100000以内的事件类型被系统保留
   */
  #define EventType int

  /**
   * 事件
   */
  class Event {
    public:
      Event(EventType t = NULL, void* source=NULL):type(t), source(source){

      }
      virtual ~Event(){}

      /**
       * 返回事件类型
       */
      virtual EventType getType() const{
        return type;
      }

      /**
       * 返回事件源
       */
      virtual void* getSource() const{
        return source;
      }

      /**
       * 设置事件源
       */
      virtual void setSource(void* s){
        source = s;
      }

    private:
      //事件类型
      EventType type;
      //事件源
      void* source;
  };

//}

#endif /* EVENT_H_ */
