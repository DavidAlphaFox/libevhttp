/* 
 * File:   SendfileIOPump.cpp
 * Author: try
 * 
 * Created on 2011年7月12日, 下午12:27
 */

#include <errno.h>
#include <sys/sendfile.h>
#include <math.h>
#include <stdlib.h>
#include "SendfileIOPump.h"

SendfileIOPump::SendfileIOPump(FD recvfd, FD sendfd, unsigned int speed, size_t maxSendSize):
   recefd(recvfd), sendfd(sendfd), speed((unsigned int)(speed*1.3)), maxSendSize(maxSendSize), sendedBytes(0), eagainCount(0),
   readEvent(this), writeEvent(this)
{
    sendfiletimer.set<SendfileIOPump, &SendfileIOPump::timerCallback> (this);
    offset = 0;
    dmaxSendSize = maxSendSize + maxSendSize;
    setLength(0);
}

SendfileIOPump::~SendfileIOPump() {
    //LOG_DEBUG("");
    close();
}

void SendfileIOPump::setLength(size_t len){
    this->length = len;
    if(length > 0){
        //根据length计算timeoutInterval, readBytesPerEach, sendBytesPerEach
        calculateSpeed(speed, &timeoutInterval, &readBytesPerEach, length>maxSendSize?maxSendSize:length);
//        LOG_DEBUG("1 readBytesPerEach:%d, timeoutInterval:%f", readBytesPerEach, timeoutInterval);
        sendBytesPerEach = readBytesPerEach;
    }else{
        calculateSpeed(speed, &timeoutInterval, &readBytesPerEach, maxSendSize);
//        LOG_DEBUG("2 readBytesPerEach:%d, timeoutInterval:%f", readBytesPerEach, timeoutInterval);
        sendBytesPerEach = readBytesPerEach;
    }
    dupTimeoutInterval = timeoutInterval;
    //maxEagainCount = (int)(1.0/timeoutInterval);    
    maxEagainCount = 20;
    //LOG_DEBUG("maxEagainCount:%f", maxEagainCount);
};

/**
 * 启动
 */
bool SendfileIOPump::start(){
    if(status == NEW){
        status = RUNNING;
        sendData();
        return true;
    }
    return false;
}

/**
 * 关闭
 */
void SendfileIOPump::close(){
    if(status == CLOSED)return;
    stopSendfileTimer();
    status = CLOSED;
}

/**
 * 复位
 */
void SendfileIOPump::reset(){
    if(status == CLOSED)return;
    status = NEW;
    offset = 0;
    sendBytesPerEach = readBytesPerEach;
    timeoutInterval = dupTimeoutInterval;
    if(sendfiletimer.is_active()){
        sendfiletimer.stop();
    }
}


void SendfileIOPump::startSendfileTimer(){
    if(!isRunning()) return;
    if(eagainCount > 5){
        LOG_DEBUG("eagainCount:%d, timeoutInterval:%f,  calInterval:%f", eagainCount, timeoutInterval, (timeoutInterval + eagainCount * eagainCount * timeoutInterval))
    }
    sendfiletimer.start(eagainCount>0?(timeoutInterval + eagainCount * eagainCount * timeoutInterval):timeoutInterval, 0);
}        

void SendfileIOPump::stopSendfileTimer(){
    if(sendfiletimer.is_active()){
        sendfiletimer.stop();
    }
 }

void SendfileIOPump::timerCallback(ev::timer &ev, int revents){
    sendData();
}    

//此方法实现的不满意，待有时间再优化
void SendfileIOPump::sendData(){
    if(!isRunning()) return;
//    LOG_DEBUG("offset:%d", offset);
    ssize_t sendsize = ::sendfile(recefd, sendfd, &offset, sendBytesPerEach);
    bool isAgain = ERRNO_IS_AGAIN();
    //处理length大于0的情况, 指定了下载数据量
    if(length > 0 && sendsize > 0){
        sendedBytes += sendsize;
        if(sendedBytes >= length){
            //已经传输完计划传输量
            readEvent.length = sendsize;
            writeEvent.length = sendsize;
            //触发事件
            fireTransportEvent(&readEvent, &writeEvent);
            //传输结束
            readEvent.length = 0;
            writeEvent.length = 0;
            //触发事件
            fireTransportEvent(&readEvent, &writeEvent);
            return;
        }
        
    }
    if(sendsize >=0 || (sendsize < 0 && !isAgain)){
        readEvent.length = sendsize;
        writeEvent.length = sendsize;
        //触发事件
        fireTransportEvent(&readEvent, &writeEvent);
        if(!isRunning()) return;
    }
    
    //以下代码写的不满意，以后有时间再优化
    if(sendsize >= 0 || isAgain){
        //重新计算下载数据量和频率
        if(length > 0){
            //指定下载数据量的情况
            if(sendsize > 0){
                maxSendSize = (sendsize + sendsize/3);
                size_t excessBytes = length - sendedBytes; //还剩下多少数据量没有发送
                maxSendSize = maxSendSize < excessBytes?maxSendSize:excessBytes;
                calculateSpeed(speed, &timeoutInterval, &sendBytesPerEach, maxSendSize);
                if(eagainCount > 0){
                    eagainCount = eagainCount/2;
                }
            }else{
                sendBytesPerEach = readBytesPerEach; //发送数据量不变
                timeoutInterval = dupTimeoutInterval;
                if(eagainCount < maxEagainCount){
                    eagainCount++;
                }
            }
            
        }else{
            //没指定下载数据量的情况
            if(sendsize > 0){
                calculateSpeed(speed, &timeoutInterval, &sendBytesPerEach, (sendsize + sendsize/3));
                if(eagainCount > 0){
                    eagainCount = eagainCount/2;
                }
            }else{
                timeoutInterval = dupTimeoutInterval;
                sendBytesPerEach = readBytesPerEach;
                if(eagainCount < maxEagainCount){
                    eagainCount++;
                }
            }
        }
        //继续发
        startSendfileTimer();

    }

}    
    




