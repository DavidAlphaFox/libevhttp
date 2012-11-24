/* 
 * File:   Queue.h
 * Author: try
 *
 * Created on 2011年7月11日, 下午3:04
 */

#ifndef QUEUE_H
#define	QUEUE_H
#include <stdlib.h>
#include <sys/types.h>

/**
 * 一个简单的队列, 用法与stl库的queue差不多
 */
template<typename T>
class Queue {
public:
    Queue():qsize(0), frontE(NULL),backE(NULL){
        
    }
    virtual ~Queue(){}
    
    void push(T o){
        if(frontE){
            backE->next = new Entry(o);
            backE = backE->next;
        }else{
            frontE = new Entry(o);
            backE = frontE;
        }
        qsize++;
    }
    
    T front(){
        return frontE->element;
    }
    
    void pop(){
        if(frontE){
            Entry* e = frontE;
            frontE = frontE->next;
            delete e;
            qsize--;
        }
    }
    
    bool empty(){
        return qsize == 0;
    }
    
    size_t size(){
        return qsize;
    }
    
private:

    typedef struct Entry{
        T element;   
        Entry* next;   //下一个元素
        
        Entry(T e):element(e), next(NULL){}
    }Entry;
    
private:
    size_t qsize;
    Entry* frontE;  //前端元素
    Entry* backE;   //后端元素
};

#endif	/* QUEUE_H */

