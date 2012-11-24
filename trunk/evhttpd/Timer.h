/* 
 * File:   Timer.h
 * Author: dell
 *
 * Created on 2012年4月7日, 下午5:18
 */

#ifndef OUTTIMER_H
#define	OUTTIMER_H

#include "resources.h"
#include "libev.h"
#include "event/EventDispatcher.h"
#include "events.h"

/**
 * 定时器, 其实只是对ev::timer的再次包装，使用更方便些
 */
class Timer : public EventDispatcher{
public:
    Timer();
    virtual ~Timer();
    
    /**
     * @param after 多少时间之后开始, 单位：秒
     * @param repeat 重复间隔，如果为0，不重复， 单位：秒
     */
    void set(double after, double repeat = 0.);
    
    void start(double after, double repeat = 0.);
    
    void start();
    
    void stop();
    
protected:
    virtual bool onTimerEvent(const TimerEvent* e){
        return false;
    }
    
private:
//    double after;
//    double repeat;
    TimerEvent timerEvent;
    ev::timer timer;
    void timerCallback(ev::timer& timer, int revents);
    
};

#endif	/* OUTTIMER_H */

