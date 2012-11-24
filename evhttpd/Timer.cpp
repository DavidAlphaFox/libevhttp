/* 
 * File:   Timer.cpp
 * Author: dell
 * 
 * Created on 2012年4月7日, 下午5:18
 */

#include "Timer.h"
#include "log.h"


Timer::Timer():timerEvent(this) {
    timer.set<Timer, &Timer::timerCallback>(this);
}

Timer::~Timer() {
    timer.stop();
//    removeAllListeners();
}

void Timer::set(double after, double repeat){
    timer.set(after, repeat);
}

void Timer::start(double after, double repeat){
    timer.start(after, repeat);
}

void Timer::start(){
    timer.start();
}

void Timer::stop(){
    if(timer.is_active()){
        timer.stop();
    }
}

void Timer::timerCallback(ev::timer& timer, int revents){
    if(onTimerEvent(&timerEvent)){
        return;
    }
    if(fire(&timerEvent)){
        return;
    }
    LOG_INFO("TimerEvent have not been processed");
}


