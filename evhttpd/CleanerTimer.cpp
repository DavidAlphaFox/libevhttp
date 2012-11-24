/* 
 * File:   CleanerTimer.cpp
 * Author: try
 * 
 * Created on 2011年7月11日, 上午9:39
 */

#include "CleanerTimer.h"

CleanerTimer CleanerTimer::instance;

CleanerTimer::CleanerTimer() {
    timeoutTimer.set<CleanerTimer, &CleanerTimer::timerCallback> (this);
    timeoutTimer.start(CLEANER_TIMER_INTERVAL, CLEANER_TIMER_INTERVAL);    
}

CleanerTimer::~CleanerTimer() {
    timeoutTimer.stop();
}

void CleanerTimer::stop(){
    if(timeoutTimer.is_active()){
       timeoutTimer.stop();
    }
}

void CleanerTimer::add(ICleaner* c){
    cleaners.insert(c);
}
    
void CleanerTimer::remove(ICleaner* c){
    CleanerSet::iterator it  = cleaners.find(c);
    if(it != cleaners.end()){
        cleaners.erase(it);
    }
}

void CleanerTimer::cleanupAll(){
    CleanerSet::iterator it  = cleaners.begin();
    for(; it != cleaners.end(); it++){
        (*it)->cleanup();
    }
}

void CleanerTimer::timerCallback(ev::timer& timer, int events){
    cleanupAll();
}



