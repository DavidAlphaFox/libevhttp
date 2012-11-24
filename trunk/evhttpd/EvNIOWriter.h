/* 
 * File:   EvNIOWriter.h
 * Author: try
 *
 * Created on 2011年6月29日, 下午3:45
 */

#ifndef EVNIOWRITER_H
#define	EVNIOWRITER_H

#include "libev.h"
#include <queue>
#include "AbstractIOWriter.h"
#include "event/IEventListener.h"
#include "resources.h"

/**
 * 基于Libev事件库的异步非阻塞IO写
 */
class EvNIOWriter : public AbstractIOWriter, protected IEventListener{
public:
    EvNIOWriter();
    EvNIOWriter(FD fd);
    virtual ~EvNIOWriter();
    
    /**
     * 设置设备描述符
     */
    virtual void setFD(FD fd);
    
    /**
     * 开始写操作
     */
    virtual void start();
    
    /**
     * 暂停写操作, 可以使用start()方法重新启动写操作
     */
    virtual void stop();       
    
    /**
     * 关闭
     */
    virtual void close();
    
    /**
     * 检查是否已经被关闭
     */
    virtual bool isClosed();    
    
    /**
     * 写数据, 如果操作成功，返回true, 否则在errno中返回错误码
     * 由于采用异步写操作，所以返回值并不是成功写入数量，而需要在写响应事件中处理
     */    
//    virtual bool write(const char* buff, size_t n, off_t offset = -1);
    
    virtual bool write(const char* buff, size_t n, bool copy = true, off_t offset = -1);
    
    /**
     * 通过IIOPump实现文件或其它类型数据传输
     * 
     * @param iopumper IIOPump的实现类型
     * 
     * @return true,如果操作成功, false,失败,errno中返回错误码,
     *          由于采用异步写操作，所以返回值并不是成功写入数量，而需要在写响应事件中处理
     */
    virtual bool transport(IIOPump* iopumper);    
    
    /**
     * 返回正准备写入的数据块数量,
     */
    virtual size_t waitingQueueSize();

//    /**
//     * 返回写队列第一个QueueDataItem*元素
//     */
//    virtual QueueDataItem* getQueueFront();    
    
    /**
     * 复位
     */
    virtual void reset();
    
    /**
     * 设置当IO繁忙时暂停写操作超时时间
     * @param t
     */
    void setSuspendTimeout(double t);
    
protected:
    /**
     * 真正写操作
     */
    virtual ssize_t realWrite(const char* buff, size_t n, off_t offset = -1);
    
    /**
     * 从队列首删除元素
     */
    virtual void popQueueFront();        
    
protected:    
    /**
    * 主要处理IIOPump事件, 返回false, 将继续处理事件，否则中止事件处理
    */
    virtual bool handle(const Event* e);    
    
protected:
    //IO描述符
    FD fd;
    //写忙时暂停时间, 单位，秒，默认:0.01
    double suspendTimeout; 
    //用于异步写数据的EV事件
    ev::timer writeEvtimer;
    //EAGAIN计数
    unsigned int eagainCount;
    //标记是否运行状态, 默认为：false, 使用start()方法将其设置为true,可用stop()方法将其设置为false
    bool isRunning;
    
    
protected:
    /**
     * 写队列数据项, 当只写入数据的一部份时，需要修改buffOffset和n的值
     */
    typedef struct QueueDataItem{
        void* buff;  //消息数据
        size_t buffOffset;  //在buff中的位置，指示从这个位置开始操作，默认为:0
        size_t n; //数据长度
        off_t targetPointer;   //目标FD位置指针, 指示写入到文件什么位置，只能对写文件使用
        bool copy;  //指示是否内部创建新buff来暂存发送数据, 默认为:true, copy被设置，需要手动释放buff
        IIOPump* iopumper; //如果iopumper不为NULL, 说明通过IIOPump来写数据
        bool isSendfile;   //标记是否发送文件, 当isSendfile被设置时，iopumper不为空
        QueueDataItem(void* buff, size_t n, off_t targetPointer, bool copy)
                :buff(buff), buffOffset(0), n(n), targetPointer(targetPointer), copy(copy), iopumper(NULL), isSendfile(false){}
        QueueDataItem(IIOPump* iopumper)
                :buff(NULL), buffOffset(0), n(0), targetPointer(-1), copy(false), iopumper(iopumper), isSendfile(true){}        
    }QueueDataItem;
    
    
    //数据队列
    typedef std::queue<QueueDataItem*> DataQueue;
    DataQueue dataQueue;
    
private:
    //初始化
    void init();
    
    void timerCallback(ev::timer& timer, int revents);
    
    void startWriteEvtimer();
    
    void stopWriteEvtimer();
    
};

#endif	/* EVNIOWRITER_H */

