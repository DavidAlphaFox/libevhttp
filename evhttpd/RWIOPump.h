/* 
 * File:   RWIOPump.h
 * Author: try
 *
 * Created on 2011年7月12日, 上午10:26
 */

#ifndef RWIOPUMP_H
#define	RWIOPUMP_H

#include "libev.h"
#include "resources.h"
#include "IIOReader.h"
#include "IIOWriter.h"
#include "event/IEventListener.h"
#include "AbstractIOPump.h"

/**
 * 基于IIOReader和IIOWriter的IOPump
 */
class RWIOPump : public AbstractIOPump, public IEventListener{
public:
    /**
     * 
     * @param ioReader 将从此IOReader中读取数据
     * @param ioWriter 将从IOReader读出的数据写入到此IIOWriter中
     * @param speed 数据读写速度，每秒传输多少字节， 如果值为0, 不限速
     * @param buffSize 数据缓冲区大小
     */
    RWIOPump(IIOReader* ioReader, IIOWriter* ioWriter, unsigned int speed, size_t buffSize=DEFAULT_DATA_SIZE);
    virtual ~RWIOPump();
    
    
    /**
     * 启动此Pump
     */
    virtual bool start();
    
    /**
     * 关闭
     */
    virtual void close();

    /**
     * 复位
     */
    virtual void reset();
    
    /**
     * @see IIOPump#setLength(size_t len)
     */
    virtual void setLength(size_t len);    
    
    /**
     * 处理监听到的事件
     * @see IEventListener#handle(const Event* e)
     */
    virtual bool handle(const Event* e);
    
    
protected:
    /**
     * 从IIOReader读到数据后调用此方法
     */
    virtual bool handleReadCompletion(const IOReadCompletionEvent* e);    
    
    /**
     * 向IIOWriter写入数据完成后调用此方法
     */
    virtual bool handleWriteCompletion(const IOWriteCompletionEvent* e);
    
private:

    void startReadDataTimer();
    
    void readData(off_t off = -1);
    
    void timerCallback(ev::timer &ev, int revents);
    
    void writeData(const char* buff, ssize_t n);
    
private:
    size_t buffSize;
    char* buff;
    double timeoutInterval; //单位：秒
    size_t readBytesPerEach; //每次读字节数
    IIOReader* ioReader;
    IIOWriter* ioWriter;
    ev::timer timer; //用来控制读写速率
    const IOReadCompletionEvent* readEvent; //最近一次完成读事件   
    unsigned int speed;
    size_t sendedBytes; //已经传输的数据量，只有仅当length属性大于0时才有意义
};

#endif	/* RWIOPUMP_H */

