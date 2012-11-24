/* 
 * File:   IEventListener.h
 * Author: try
 *
 * Created on 2011年7月4日, 上午12:24
 */

#ifndef IEVENTLISTENER_H
#define	IEVENTLISTENER_H

//namespace Try {
  class Event;

    class IEventListener {
    public:
        IEventListener(){}
        virtual ~IEventListener(){}


          /**
           * 处理事件, 返回false, 将继续处理事件，否则中止事件处理
           */
          virtual bool handle(const Event* e)=0;


    private:

    };
    
//}
#endif	/* IEVENTLISTENER_H */

