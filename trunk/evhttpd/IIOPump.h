/* 
 * File:   IIOPump.h
 * Author: try
 *
 * Created on 2011年7月14日, 下午3:22
 */

#include <sys/types.h>
#include "resources.h"
#include "events.h"
#include "event/IEventDispatcher.h"

#ifndef IIOPUMP_H
#define	IIOPUMP_H


/**
 * 主动式IO操作抽象接口, 对实现此接口的类型有如下要求:
 * 
 *  1. 当调用start()方法启动后， 将会自动根据一定速率从IO输入源读出数据并写入
 *     到IO输出目的地, 典型实现有: RWIOPump, SendfileIOPump等
 *  
 *  2. 可调节数据输送速度，RWIOPump, SendfileIOPump都可指定输出速度
 * 
 */
class IIOPump : public IEventDispatcher{
public:
    IIOPump(){}
    virtual ~IIOPump(){}
    
    /**
     * 启动
     * 
     * @return true，成功
     */
    virtual bool start()=0;
    
    /**
     * 设置从什么位置开始读, 默认为:-1, 从IO当前位置开始读，
     * 如果是打开的文件，从文件第0位置开始读
     * 
     */
    virtual void setOffset(off_t offset)=0;
    
    /**
     * 设置最大传输长度，默认为：0， 表示将所有数据传完或出错为止
     * 
     * @param len 最大传输数据量
     */
    virtual void setLength(size_t len)=0;
    
    /**
     * 关闭
     */
    virtual void close()=0;

    /**
     * 检查是否正处于准备状态
     */
    virtual bool isReady()=0;
    
    /**
     * 检查是否已经关闭
     */
    virtual bool isClosed()=0;

    /**
     * 检查是否正运行中
     */
    virtual bool isRunning()=0;
    
    /**
     * 复位
     */
    virtual void reset()=0;
    
    
protected:
    /**
     * 传输事件，当从IO源读出数据或向IO目的地写入数据后会调用此方法，也可监听
     * IOTransportEvent事件
     * 
     * @param e 传输事件
     * 
     * @return true，事件被处理，false，未被处理
     */
    virtual bool onTransportEvent(const IOTransportEvent* e){
        return false;
    }

};

#endif	/* IIOPUMP_H */

