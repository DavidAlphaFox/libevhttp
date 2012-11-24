/* 
 * File:   IIOReader.h
 * Author: try
 *
 * Created on 2011年7月2日, 下午3:47
 */

#ifndef IIOREADER_H
#define	IIOREADER_H
#include <sys/types.h>
#include <sys/stat.h>
#include "resources.h"
#include "events.h"
#include "event/IEventDispatcher.h"

/**
 * 抽象IO Reader，定义IO读相关方法
 */
class IIOReader : public IEventDispatcher{
public:
    IIOReader(){}
    virtual ~IIOReader(){}
    
    /**
     * 关闭IOReader
     */
    virtual void close()=0;
    
    /**
     * 检查是否已经被关闭
     */
    virtual bool isClosed()=0;
    
    /**
     * 提交读数据请求, 如果操作成功，返回true, 否则在errno中返回错误码。在读响应
     * 事件方法(onReadCompletionEvent)中处理读到的数据，或监听IOReadCompletionEvent事件
     * 来处理读到的数据。
     * 
     * 注意:在socket套节字上不能设置offset参数大于等于0，因为不能在套接字上进行lseek操作
     * 
     * @param buff 数据区，读出的数据存在这里, 如果buff为NULL值，将会自动创建新的数据缓冲, 并在执
     *             行完onReadCompletionEvent后自动释放
     * @param n 本次读出的数据量
     * @param full 指示是否将buff读满(即n字节)后才触发读完成事件, 默认值:false
     * @param offset 读偏移位置，可以指定从什么位置开始读, 如果值小于0，说明为顺序读
     */    
    virtual bool read(char* buff, size_t n, bool full = false, off_t offset = -1)=0;
    
    
    /**
     * 返回提交的还未处理完的读请求数量
     */
    virtual size_t waitingQueueSize()=0;

    /**
     * 复位
     */
    virtual void reset()=0;   
    
protected:
    /** 
     * 当读数据完成后调用此方法
     * 
     * @param e 读完成事件
     * @see IOReadCompletionEvent
     * 
     * @return true, 事件被处理，false, 事件未被处理
     **/
    virtual bool onReadCompletionEvent(const IOReadCompletionEvent* e){
        return false;
    }
    
private:

};

#endif	/* IIOREADER_H */

