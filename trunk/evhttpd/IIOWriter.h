/* 
 * File:   IIOWriter.h
 * Author: try
 *
 * Created on 2011年6月29日, 下午3:21
 */

#ifndef IIOWRITER_H
#define	IIOWRITER_H
#include <sys/types.h>
#include "resources.h"
#include "events.h"
#include "event/IEventDispatcher.h"
#include "IIOPump.h"

/**
 * 抽象IO Writer，定义IO写相关方法
 */
class IIOWriter : public IEventDispatcher{
public:
    struct QueueDataItem;
public:
    IIOWriter(){}
    virtual ~IIOWriter(){}
    
    
    /**
     * 开始写操作
     */
    virtual void start()=0;
    
    /**
     * 暂停写操作, 可以使用start()方法重新启动写操作
     */
    virtual void stop()=0;    
    
    /**
     * 关闭
     */
    virtual void close()=0;
    
    /**
     * 检查是否已经被关闭
     */
    virtual bool isClosed()=0;
    
    /**
     * 提交写数据请求, 在写响应事件方法(onWriteCompletionEvent)中处理写入情况，
     * 或监听IOWriteCompletionEvent事件来处理写入成功或失败情况。
     * 
     * @param buff 数据区
     * @param n 写入数据量
     * @param copy 指示是否内部创建新buff来暂存待发送数据, 默认为:true
     * @param offset 将数据写入到目标什么位置，如果值为-1为顺序写(默认)
     * 
     * @return true,如果操作成功, false,失败,errno中返回错误码
     */
    virtual bool write(const char* buff, size_t n, bool copy = true, off_t offset = -1)=0;
    
    
    /**
     * 通过IIOPump实现文件或其它类型数据传输
     * 
     * @param iopumper IIOPump的实现类型
     * 
     * @return true,如果操作成功, false,失败,errno中返回错误码
     */
    virtual bool transport(IIOPump* iopumper)=0;
    
    /**
     * 返回提交的还未处理完的写请求数量
     */
    virtual size_t waitingQueueSize()=0;
   
//    /**
//     * 返回写队列第一个QueueDataItem*元素
//     */
//    virtual QueueDataItem* getQueueFront()=0;
    
    /**
     * 复位
     */
    virtual void reset()=0;
    
    
protected:

    /**
     * 当完成写入数据后会调用此方法 
     * 
     * @param e 写完成事件
     * @see IOWriteCompletionEvent
     * 
     * @return true, 事件被处理，false, 事件未被处理
     */
    virtual bool onWriteCompletionEvent(const IOWriteCompletionEvent* e){
        return false;
    }
    
private:

};

#endif	/* IIOWRITER_H */

