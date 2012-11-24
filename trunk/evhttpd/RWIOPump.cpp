/* 
 * File:   RWIOPump.cpp
 * Author: try
 * 
 * Created on 2011年7月12日, 上午10:26
 */

#include "RWIOPump.h"
#include "MemoryPool.h"

RWIOPump::RWIOPump(IIOReader* ioReader, IIOWriter* ioWriter, unsigned int speed, size_t buffSize)
    :buffSize(buffSize), ioReader(ioReader), ioWriter(ioWriter), speed(speed), readEvent(NULL), sendedBytes(0){
    
    buff = (char*)MemoryPool::instance.malloc(buffSize);
    timer.set<RWIOPump, &RWIOPump::timerCallback> (this);
    ioReader->addListener(EVENT_TYPE_IO_READ_COMPLETION, this);
    ioWriter->addListener(EVENT_TYPE_IO_WRITE_COMPLETION, this);
    
    setLength(0);
    
}

RWIOPump::~RWIOPump(){
    close();
    MemoryPool::instance.free(buff);
}

void RWIOPump::setLength(size_t len){
    this->length = len;
    if(length > 0){
        //根据length计算timeoutInterval, readBytesPerEach, sendBytesPerEach
        calculateSpeed(speed, &timeoutInterval, &readBytesPerEach, length>buffSize?buffSize:length);
    }else{
        calculateSpeed(speed, &timeoutInterval, &readBytesPerEach, buffSize);
    } 
}


bool RWIOPump::start(){
    if(status == NEW){
        status = RUNNING;
        ioWriter->start();
        if(offset >= 0 ){
          readData(offset);
        }else{
          readData();  
        }
        return true;
    }
    return false;
}

/**
 * 关闭
 */
void RWIOPump::close(){
    if(status == CLOSED)return;
    status = CLOSED;
    if(timer.is_active()){
        timer.stop();
    }
    ioReader->close();
    ioWriter->close();
}

/**
 * 复位
 */
void RWIOPump::reset(){
    if(status == CLOSED)return;
    status = NEW;
    ioReader->reset();
    ioWriter->reset();
}

/**
 * @see IEventListener::handle
 * @param e
 * @return 
 */
bool RWIOPump::handle(const Event* e){
    if(!isRunning())return true;
    switch(e->getType()){
        case EVENT_TYPE_IO_READ_COMPLETION:
            return handleReadCompletion((const IOReadCompletionEvent*)e);
            break;
        case EVENT_TYPE_IO_WRITE_COMPLETION:
            return handleWriteCompletion((const IOWriteCompletionEvent*)e);
            break;
    }
}

/**
 * 处理从IIOReader读到的数据
 */
bool RWIOPump::handleReadCompletion(const IOReadCompletionEvent* e){
    LOG_DEBUG("");
    readEvent = e;

     bool eventrs = fireTransportEvent(e, NULL);
    
    if(!isRunning())return eventrs;
    
    if(e->length > 0){
        writeData(e->dataBuff, e->length);
    }

    return eventrs;
}

/**
 * 向IIOWriter写入数据完成
 */
bool RWIOPump::handleWriteCompletion(const IOWriteCompletionEvent* e){
    bool eventrs = fireTransportEvent(readEvent, e);
    if(!isRunning())return true;
    //LOG_DEBUG("sendsize:%d", e->length)
    //if(e->length > 0 && ioWriter->waitingQueueSize()==0){
    if(length > 0){
        sendedBytes += e->length;
        if(sendedBytes >= length){
            //已经传输完计划传输量
            //传输结束
            ((IOReadCompletionEvent*)readEvent)->length = 0;
            ((IOWriteCompletionEvent*)e)->length = 0;
            //触发事件
            return fireTransportEvent(readEvent, e);
        }

        //计算下一次读数据量
        size_t excessBytes = length - sendedBytes; //还剩下多少数据量没有发送
        if(excessBytes < readBytesPerEach){
            readBytesPerEach = excessBytes;
        }
    }else if(e->length == 0 && ioWriter->waitingQueueSize()==0){
        startReadDataTimer();
    }
    
    return eventrs;
}

void RWIOPump::startReadDataTimer(){
    if(!isRunning()) return;
    timer.start(timeoutInterval, 0);
}    

void RWIOPump::readData(off_t off){
    if(!isRunning()){
        LOG_WARN("not isRunning()");
        return;
    }

    if(!ioReader){
        LOG_WARN("ioReader is NULL");
        return;
    }
    if(ioReader->isClosed()){
        LOG_WARN("ioReader is Closed");
        return;
    }
    if(ioReader->waitingQueueSize() > 0){
        LOG_WARN("waitingQueueSize():%d", ioReader->waitingQueueSize()); //还有读数据请求没有返回，还是再等等
        return;
    }
    //LOG_DEBUG("off:%d", off);
    ioReader->read(buff, readBytesPerEach, false, off);
}    

void RWIOPump::timerCallback(ev::timer &ev, int revents){
    readData();
}

void RWIOPump::writeData(const char* buff, ssize_t n){
    if(!isRunning()){
        LOG_WARN("not !isRunning()");
        return;
    }        

    if(!ioWriter){
        LOG_WARN("ioWriter is NULL");
        return;
    }

    if(ioWriter->isClosed()){
        LOG_WARN("ioWriter is Closed");
        return;
    }

    ioWriter->write(buff, n, true, -1);

}





