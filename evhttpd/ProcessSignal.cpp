/* 
 * File:   ProcessSignal.cpp
 * Author: dell
 * 
 * Created on 2012年4月6日, 上午9:05
 */

#include <bits/stl_pair.h>

#include "ProcessSignal.h"

using namespace ev;

ProcessSignal::ProcessSignal():processSignalEvent(this) {
    
}

ProcessSignal::~ProcessSignal() {
    EVSignals::iterator it = evSignals.begin();
    for(; it != evSignals.end(); it++){
        ev::sig* sig = it->first;
//        evSignals.erase(it++);
        if(sig->is_active()){
            sig->stop();
        }
        delete sig;
    }
    evSignals.clear();
}

void ProcessSignal::addSignal(int sigval){
    sig* s = new sig();
    s->set(sigval);
    s->set<ProcessSignal, &ProcessSignal::sigCallback>(this);
    evSignals.insert(EVSignals::value_type(s, sigval));
}

void ProcessSignal::sigCallback(ev::sig &ev, int revents){
    LOG_DEBUG("");
    EVSignals::iterator it = evSignals.find(&ev);
    if(it == evSignals.end()){
        LOG_WARN("undefined signal");
        return;
    }
    //发送事件
    fireProcessSignalEvent(it->second);
    
}


void ProcessSignal::start(){
    EVSignals::iterator it = evSignals.begin();
    for(; it != evSignals.end(); it++){
        if(!(it->first->is_active())){
            it->first->start();
        }
    }
}

void ProcessSignal::stop(){
    EVSignals::iterator it = evSignals.begin();
    for(; it != evSignals.end(); it++){
        if(it->first->is_active()){
            it->first->stop();
        }
    }
}


bool  ProcessSignal::fireProcessSignalEvent(int signal){
    processSignalEvent.signal = signal;

    if(onProcessSignalEvent(&processSignalEvent)){
        //事件已经被处理，返回
        return true; 
    }
    //把事件交给其它监听器处理
    if(fire(&processSignalEvent)){
        //事件已经被处理，返回
        return true; 
    }
    LOG_INFO("ProcessSignalEvent have not been processed");
    return false;
}   


