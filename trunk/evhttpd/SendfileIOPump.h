/* 
 * File:   SendfileIOPump.h
 * Author: try
 *
 * Created on 2011年7月12日, 下午12:27
 */

#ifndef SENDFILEIOPUMP_H
#define	SENDFILEIOPUMP_H

#include "AbstractIOPump.h"
#include "resources.h"
#include "libev.h"


/**
 * 基于Sendfile实现的IOPump
 */
class SendfileIOPump : public AbstractIOPump{
    
public:
    /**
     * @param recvfd 接收方FD
     * @param sendfd 发送方FD
     * @param speed 发送速度，字节单位，如果值为0，不限速
     * @param maxSendSize, 最多一次发送的数据量
     */
    SendfileIOPump(FD recvfd, FD sendfd, unsigned int speed, size_t maxSendSize=DEFAULT_DATA_SIZE);
    virtual ~SendfileIOPump();
    
    
    /**
     * 启动
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
    
private:
    FD recefd;
    FD sendfd;
    unsigned int speed;
    size_t maxSendSize;
    size_t dmaxSendSize;
//    off_t sendOffset; //sendfile的offset参数
    size_t sendedBytes; //已经传输的数据量，只有仅当length属性大于0时才有意义
    
    double timeoutInterval; //超时间隔，等待多少时间再发送，单位：秒
    double dupTimeoutInterval;//初始化时计算出的超时间隔
    size_t readBytesPerEach; ///初始化时计算出的每次读字节数 
    size_t sendBytesPerEach; //最终每次发送字节数量, 会根据每次实际发送出的数量重新计算
        
    int eagainCount; //EAGAIN计数
    int maxEagainCount; //最大EAGAIN计数次数， eagainCount不会大于maxEagainCount值
    
    IOReadCompletionEvent readEvent; //读完成事件，注意：事件的dataBuff为NULL, 因为并没有将数据读入到用户空间
    IOWriteCompletionEvent writeEvent; //写完成事件, 注意：事件的dataBuff为NULL, 因为并没有将数据读入到用户空间
    
    ev::timer sendfiletimer; //读文件定时器，用来控制读文件速率
    
private:

    void startSendfileTimer();
    
    void stopSendfileTimer();
  
    void timerCallback(ev::timer &ev, int revents);
    
    void sendData();
};

#endif	/* SENDFILEIOPUMP_H */

