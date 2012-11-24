/* 
 * File:   EvNIOReader.cpp
 * Author: try
 * 
 * Created on 2011年7月2日, 下午4:12
 */

#include <errno.h>
#include "EvNIOReader.h"
#include "SocketUtils.h"
#include "MemoryPool.h"

EvNIOReader::EvNIOReader():fd(-1) {
    readevio.set<EvNIOReader, &EvNIOReader::readCallback>(this);
 
}

EvNIOReader::EvNIOReader(FD fd):fd(-1){
    readevio.set<EvNIOReader, &EvNIOReader::readCallback>(this);
    setFD(fd);
}

EvNIOReader::~EvNIOReader() {
    reset();
}

void EvNIOReader::setFD(FD fd){
    if(fd >=0 && this->fd != fd){
        this->fd = fd;
        SocketUtils::setNonblock(fd);
        readevio.set(fd, EV_READ);
    }
}

void EvNIOReader::close(){
    if(fd < 0)return;
    reset();
    fd = -1;
}

bool EvNIOReader::isClosed(){
    return fd < 0;
}

bool EvNIOReader::read(char* buff, size_t n, bool full /*  = false */, off_t offset /* = -1*/){
    if(isClosed())return false;
  
    if(buff){
        entryQueue.push(new Entry(buff, n, offset, full));
    }else{
        buff = (char*)MemoryPool::instance.malloc(n);
        entryQueue.push(new Entry(buff, n, offset, full, true));
    }
    
    feedReadEvio();
    return true;
}

size_t EvNIOReader::waitingQueueSize(){
    return entryQueue.size();
}

void EvNIOReader::reset(){
    stopReadEvio();
    while(!entryQueue.empty()){
        popEntry();
    }
}  

void EvNIOReader::popEntry(){
    if(entryQueue.empty())return;
       Entry* e = entryQueue.front();
       entryQueue.pop(); 
       if(e->mustfree){
           MemoryPool::instance.free(e->buff);
       }
       delete e;
}

ssize_t EvNIOReader::realRead(char* buff, size_t n, off_t offset){
    LOG_DEBUG("n:%d, offset:%d", n, offset);
    int rsize = -1;
    if(offset >= 0 ){
        off_t off = ::lseek(fd, offset, SEEK_SET);
        if(off >= 0){
            rsize = ::read(fd, buff, n);
        }else{
            LOG_WARN("lseek error, errno:%d", errno);
        }
    }else{
        rsize = ::read(fd, buff, n);
    }
    return rsize;
}

void EvNIOReader::readCallback(ev::io &evio, int revents){
    LOG_DEBUG("isClosed():%d, entryQueue.size():%d", isClosed(), entryQueue.size());
    if(isClosed()){
        LOG_DEBUG("isClosed()");
        return;
    }
    
    if(entryQueue.empty()){
        LOG_DEBUG("entryQueue.size():%d", entryQueue.size());
        stopReadEvio();
        return;
    }
    
    Entry* e = entryQueue.front();
    //errno = 0;
    size_t rsize = realRead(e->buff + e->actualLength, e->planLength - e->actualLength, e->pointer);
    LOG_DEBUG("rsize:%d", rsize);
    if(rsize < 0 && ERRNO_IS_AGAIN()){//errno == EAGAIN){
        LOG_DEBUG("errno:%d, rsize:", errno, rsize);
        //待下一次重读,
        feedReadEvio();
        return;
    }
    if(rsize <= 0){
        //读失败
        LOG_DEBUG("errno:%d, rsize:", errno, rsize);
        fireReadCompletionEvent(e->buff, rsize, e->pointer);
        close();
        return;
    }    
    
    e->actualLength += rsize;
    if(e->full && e->actualLength < e->planLength){
        LOG_DEBUG("actualLength:%d, planLength:%d", e->actualLength, e->planLength);
        return;
    }
    
    fireReadCompletionEvent(e->buff, e->actualLength, e->pointer);
    popEntry();

    
    LOG_DEBUG("entryQueue.size():%d", entryQueue.size());
    if(entryQueue.empty()){
        stopReadEvio();
    }else{
        feedReadEvio();
    }
}

void EvNIOReader::stopReadEvio(){
    if(isClosed())return;
    if(readevio.is_active()){
        readevio.stop();
    }
}


void EvNIOReader::feedReadEvio(){
    if(isClosed())return;
    if(!readevio.is_active()){
        readevio.start();
    }
    if(!readevio.is_pending()){
        readevio.loop.feed_fd_event(fd, EV_READ);
    }
}
