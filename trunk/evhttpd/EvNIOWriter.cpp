/* 
 * File:   EvNIOWriter.cpp
 * Author: try
 * 
 * Created on 2011年6月29日, 下午3:45
 */

#include "EvNIOWriter.h"
#include "SocketUtils.h"
#include <errno.h>
#include <iostream>
#include <queue>
#include "MemoryPool.h"

using namespace std;

EvNIOWriter::EvNIOWriter():fd(-1) {
    init();
}

EvNIOWriter::EvNIOWriter(FD fd):fd(-1) {
    init();
    setFD(fd);
}

EvNIOWriter::~EvNIOWriter() {
    reset();
}

void EvNIOWriter::init(){
    isRunning = false;
    eagainCount = 0;
    suspendTimeout = 0.01;
    writeEvtimer.set<EvNIOWriter, &EvNIOWriter::timerCallback>(this);
}

void EvNIOWriter::setFD(FD fd){
    if(fd>=0 && this->fd != fd){
        this->fd = fd;
        SocketUtils::setNonblock(fd);
    }
}

/**
    * 开始写操作
    */
void EvNIOWriter::start(){
    LOG_DEBUG("isRunning:%s", (isRunning?"true":"false"));
    if(!isRunning){
        isRunning = true;
        if(!dataQueue.empty()){
            startWriteEvtimer();
        }
    }
}

/**
    * 暂停写操作, 可以使用start()方法重新启动写操作
    */
void EvNIOWriter::stop(){
    isRunning = false;
    stopWriteEvtimer();
}

/**
 * 关闭
 */
void EvNIOWriter::close(){
    if(fd < 0)return;
    reset();
    fd = -1;
}

/**
 * 检查是否已经被关闭
 */
bool EvNIOWriter::isClosed(){
    return fd < 0;
}

bool EvNIOWriter::write(const char* buff, size_t n, bool copy /*  = true */, off_t offset /*  = -1 */){
    if(isClosed())return false;
    LOG_DEBUG("n:%d, copy:%d, offset:%d", n, copy, offset);
    QueueDataItem* item;
    if(copy){
        char* nbuff = (char*)MemoryPool::instance.malloc(n);
        memcpy(nbuff, buff, n);
        item = new QueueDataItem(nbuff, n, offset, copy);
    }else{
        item = new QueueDataItem((void*)buff, n, offset, copy);
    }
    dataQueue.push(item); 
    
    //触发写事件
    if(isRunning){
        startWriteEvtimer();
    }
    
    return true;
}

bool EvNIOWriter::transport(IIOPump* iopumper){
    if(isClosed())return false;
    
    //设置传输完成事件
    iopumper->addListener(EVENT_TYPE_IO_TRANSPORT, this);
    
    dataQueue.push(new QueueDataItem(iopumper));
    
    //触发写事件
    if(isRunning){
        startWriteEvtimer();
    }
    
    return true;
}


ssize_t EvNIOWriter::realWrite(const char* buff, size_t n, off_t offset){
    LOG_DEBUG("offset:%d", offset);
    int wsize = -1;
    if(offset >= 0 ){
        off_t off = ::lseek(fd, offset, SEEK_SET);
        if(off >= 0){
            wsize = ::write(fd, buff, n);
        }else{
            LOG_WARN("lseek error, errno:%d", errno);
            errno = ESPIPE;
        }
    }else{
        wsize = ::write(fd, buff, n);
    }
    return wsize;
}

void EvNIOWriter::reset(){
    while(!dataQueue.empty()){
        popQueueFront();
    }
    stopWriteEvtimer();
}

//从队列首删除元素
void EvNIOWriter::popQueueFront(){
    if(dataQueue.empty())return;
    QueueDataItem* item = dataQueue.front();
    dataQueue.pop();
    if(item->copy && item->buff){
        MemoryPool::instance.free(item->buff);
    }
    delete item;
}

/**
 * 返回正准备写入的数据块数量,
 */
size_t EvNIOWriter::waitingQueueSize(){
    return dataQueue.size();
}


void EvNIOWriter::timerCallback(ev::timer& timer, int revents){
    LOG_DEBUG("");
    if(isClosed()){
        LOG_WARN("isClosed()");
        return;
    }
    
    if(dataQueue.empty()){
        stopWriteEvtimer();
        return;
    }
    LOG_DEBUG("dataQueue.size:%d", dataQueue.size());
    QueueDataItem* e = dataQueue.front();

    if(e->iopumper){//如果是传输文件
        stopWriteEvtimer(); //先停止当前写timer事件
        if(!e->iopumper->start()){ //开始传输数据
            IIOPump* iopumper = e->iopumper;
            off_t targetPointer = e->targetPointer;
            //如果启动失败，发现完成事件
            fireWriteCompletionEvent(NULL, -1, targetPointer, iopumper);
            popQueueFront();
            close();
        }
        return;
    }
    
    
    const char* buff =  (char*)e->buff + e->buffOffset;
    LOG_DEBUG("e->n:%d, e->buffOffset:%d, e->targetPointer:%ld", e->n, e->buffOffset, e->targetPointer);
    //errno = 0;
    ssize_t wsize = realWrite(buff, e->n, e->targetPointer);
    LOG_DEBUG("wsize:%d, errno:%d, e->n:%d", wsize, errno, e->n);

    if(wsize == e->n){
        //如果发送成功，就移除池中的当前已经发送的消息, 并继续发送下一条消息
        fireWriteCompletionEvent(buff, wsize, e->targetPointer);
        popQueueFront();
    }else if(wsize >= 0){
        //发送了一部份, 把没发送完的部份留着下次发送
        e->buffOffset += wsize;
        e->n -= wsize;
        fireWriteCompletionEvent(buff, wsize, e->targetPointer);
    }else if(!ERRNO_IS_AGAIN()){//(errno != EAGAIN){ 
        off_t targetPointer = e->targetPointer;
        //发送失败
       fireWriteCompletionEvent(buff, -1, targetPointer);
       popQueueFront();
       close();
       return;
    }else{
        //errno == EAGAIN， 太忙，等段时间多
        //LOG_WARN("1, errno:%d, eagainCount:%d", errno, eagainCount);
        if(eagainCount < 100){
            eagainCount++;
        }
        startWriteEvtimer();
        return;
    }
    
    if(eagainCount > 0){
        //LOG_WARN("2, errno:%d, eagainCount:%d", errno, eagainCount);
        eagainCount = eagainCount/2;
    }
    
    if(dataQueue.empty()){
        stopWriteEvtimer();
        fireWriteCompletionEvent(NULL, 0, -1);//队列空，写完成
    }else{
        startWriteEvtimer();
    }
    
}

bool EvNIOWriter::handle(const Event* e){
    if(e->getType() == EVENT_TYPE_IO_TRANSPORT){
        IOTransportEvent* te = (IOTransportEvent*)e;
        IIOPump* ioPumper = (IIOPump*)te->getSource();
        if(!ioPumper){
            LOG_ERROR("ioPumper is NULL");
        }
        if(te->readEvent->length < 0 || (te->writeEvent && te->writeEvent->length < 0)){ //读出错或写出错(写事件有可能为NULL)
            //LOG_WARN("EvNIOWriter::handle, te->readEvent->length:%d, te->writeEvent->length:%d", te->readEvent->length, ((te->writeEvent)?te->writeEvent->length:-100));
            ioPumper->close();
            fireWriteCompletionEvent(NULL, -1, te->readEvent->offset, ioPumper);
            popQueueFront();
            close();
        }else if(te->readEvent->length == 0){ //读结束
            ioPumper->close();
            if(dataQueue.empty()){
                LOG_ERROR("dataQueue is empty, dataQueue.size is %d", dataQueue.size());
                fireWriteCompletionEvent(NULL, -1, te->readEvent->offset, ioPumper);
                popQueueFront();
                close();
                return true;
            }
            QueueDataItem* d = dataQueue.front();
            if(!d->iopumper){
                LOG_ERROR("iopumper is NULL, dataQueue.size is %d", dataQueue.size());
                fireWriteCompletionEvent(NULL, -1, te->readEvent->offset, ioPumper);
                popQueueFront();
                close();
                return true;
            }
            popQueueFront();
            if(dataQueue.empty()){
                stopWriteEvtimer();
                fireWriteCompletionEvent(NULL, 0, te->readEvent->offset);//队列空，写完成
            }else{
                startWriteEvtimer();
            }            
        }else{
            fireWriteCompletionEvent(te->readEvent->dataBuff, te->readEvent->length, te->readEvent->offset, ioPumper);
        }
        return true;
    }
    return false;
}


void EvNIOWriter::startWriteEvtimer(){
    if(!writeEvtimer.is_active()){
        LOG_DEBUG("suspendTime:%f, waitingQueueSize():%d, eagainCount:%d, eagainCount * suspendTimeout is %f", suspendTimeout, waitingQueueSize(), eagainCount, (eagainCount * suspendTimeout));
        if(eagainCount == 0){
            writeEvtimer.start(0, 0);
        }else{
            writeEvtimer.start(eagainCount * eagainCount * suspendTimeout, 0);
        }
    }
}

void EvNIOWriter::stopWriteEvtimer(){
    if(writeEvtimer.is_active()){
        writeEvtimer.stop();
    }
}


void EvNIOWriter::setSuspendTimeout(double t){
    if(t <= 0.0){
        suspendTimeout = 0.01;
    }else{
        suspendTimeout = t;
    }
}


