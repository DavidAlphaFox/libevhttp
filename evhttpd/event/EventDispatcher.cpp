/*
 * EventDispatcher.cpp
 *
 *  Created on: 2010-8-16
 *      Author: wenjian
 */

#include "EventDispatcher.h"
#include "IEventListener.h"
//#include "../thread/Synchronized.h"

using namespace std;

//namespace Try {

  EventDispatcher::EventDispatcher() {
    // TODO Auto-generated constructor stub

  }

  EventDispatcher::~EventDispatcher() {
      removeAllListeners();
  }

  void EventDispatcher::removeAllListeners(){
    for(ListenerMapIterator mit=allTypeListeners.begin(); mit!=allTypeListeners.end(); mit++){
        delete mit->second;
    }
    allTypeListeners.clear();
    allEventListeners.clear();
  }
  
  void EventDispatcher::addListener(EventType type, IEventListener* l, int pos){
    vector<IEventListener*>* ls = allTypeListeners[type];
    if(!ls){
      ls = new vector<IEventListener*>();
      allTypeListeners[type] = ls;
      ls->push_back(l);
    }else{
      int lsSize = ls->size();
      pos = (pos<0 || pos>lsSize)?lsSize:pos;
      ListenerIterator it = std::find(ls->begin(), ls->end(), l);
      ListenerIterator begin = ls->begin();
      if (it != ls->end()){
        if(pos<lsSize && *it == (*ls)[pos]){
          return;
        }
        if((it-begin)<pos){
          pos--;
        }
        ls->erase(it);
      }
      ls->insert(begin+pos, l);
    }
  }

  void EventDispatcher::addListener(IEventListener* l, int pos){
    int lsSize = allEventListeners.size();
    pos = (pos<0 || pos>lsSize)?lsSize:pos;
    ListenerIterator it = std::find(allEventListeners.begin(), allEventListeners.end(), l);
    ListenerIterator begin = allEventListeners.begin();
    if (it != allEventListeners.end()){
      if(pos<lsSize && *it == allEventListeners[pos]){
        return;
      }
      if((it-begin)<pos){
        pos--;
      }
      allEventListeners.erase(it);
    }
    allEventListeners.insert(begin+pos, l);
  }

  void EventDispatcher::removeListener(IEventListener* l){
    ListenerIterator it;
    it = std::find(allEventListeners.begin(), allEventListeners.end(), l);
    if(it != allEventListeners.end()){
      allEventListeners.erase(it);
    }

    for(ListenerMapIterator mit=allTypeListeners.begin(); mit!=allTypeListeners.end(); mit++){
      vector<IEventListener*>* ls = mit->second;
      if(ls){
        it = std::find(ls->begin(), ls->end(), l);
        if(it != ls->end()){
          ls->erase(it);
        }
      }
    }
  }

  bool EventDispatcher::fire(const Event* e, bool asyn){
    if(asyn){
//      Synchronized syn(&aeLock);
      allAsynEvents.push_back(e);
      return false;
    }else{
      return handle(e);
    }
  }

  void EventDispatcher::handleAll(){
//    Synchronized syn(&aeLock);
    vector<const Event*>::iterator it = allAsynEvents.begin();
    for(; it!=allAsynEvents.end(); it++){
      handle(*it);
    }
    allAsynEvents.clear();
  }

  bool EventDispatcher::handle(const Event* e){
    //执行具体类型相关的监听器
    vector<IEventListener*>* ls = allTypeListeners[e->getType()];
    if(ls){
      for(ListenerIterator it=ls->begin(); it!=ls->end(); it++){
        if((*it)->handle(e)){
          return true;
        }
      }
    }
    //执行能监听所有事件的监听器
    ListenerIterator it = allEventListeners.begin();
    for(; it!=allEventListeners.end(); it++){
      if((*it)->handle(e)){
        return true;
      }
    }

    return false;
  }
  
//}
