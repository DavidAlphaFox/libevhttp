/* 
 * File:   AbstractIOPump.cpp
 * Author: try
 * 
 * Created on 2011年7月14日, 下午3:43
 */

#include "AbstractIOPump.h"

AbstractIOPump::AbstractIOPump():transportEvent(this),status(NEW), offset(-1), length(0) {
}

AbstractIOPump::~AbstractIOPump() {
}

bool AbstractIOPump::fireTransportEvent(const IOReadCompletionEvent* readEvent, const IOWriteCompletionEvent* writeEvent){
    transportEvent.readEvent = readEvent;
    transportEvent.writeEvent = writeEvent;
    if(onTransportEvent(&transportEvent)){
       //事件已经被处理，返回
       return true; 
    }

    //把事件交给其它监听器处理
    if(fire(&transportEvent)){
       //事件已经被处理，返回
       return true; 
    }

    LOG_INFO("PipelineTransportEvent have not been processed");
    return false;
}       

/**
 *  根据计划传输速度计算读数据频率和每次读数据量
 * 
 * @param speed 计划传输速度, 如果值小于等于0表示无限制
 * @param timeoutInterval 在此返回读数据频率
 * @param readBytesPerEach 在此返回每次读多少数据量
 * @param buffSize 数据缓冲区大小
 */
void AbstractIOPump::calculateSpeed(double speed, double* timeoutInterval, size_t* readBytesPerEach, size_t buffSize){
        if(speed > 0){
            //计算每次需要读出的字节数量和读时间间隔
            *readBytesPerEach = (int)(speed + ((buffSize/speed)/3)*speed);
            if(*readBytesPerEach > buffSize){
                *readBytesPerEach = buffSize;
            }
            *timeoutInterval = *readBytesPerEach/speed;
            if(*timeoutInterval > MAX_IO_READ_WRITE_INTERVAL){ //如果时间间隔大于MAX_TIMEOUT_INTERVAL秒,就重新计算 
                *timeoutInterval = MAX_IO_READ_WRITE_INTERVAL;
                *readBytesPerEach = (int)(*timeoutInterval*speed);
                *readBytesPerEach = *readBytesPerEach > buffSize?buffSize:*readBytesPerEach;
            }
        }else{
            //无限制
            *timeoutInterval = 0.001;
            *readBytesPerEach = buffSize;
        }
        LOG_DEBUG("speed:%f(/sec), timeoutInterval:%f, readBytesPerEach:%d", speed, *timeoutInterval, *readBytesPerEach);
}



