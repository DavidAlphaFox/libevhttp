/* 
 * File:   ProcessFDSender.h
 * Author: try
 *
 * Created on 2011年7月8日, 下午4:52
 */

#ifndef PROCESSFDSENDER_H
#define	PROCESSFDSENDER_H
#include "EvNIOWriter.h"

/**
 * 用于主进程向子进程发送FD
 */
class ProcessFDSender : public EvNIOWriter{
public:
    ProcessFDSender();
    /**
     * @param fd 接收端的FD
     */
    ProcessFDSender(FD fd);
    
    virtual ~ProcessFDSender();
    
    /**
     * 发送FD, 如果发送成功，在FDSendCompletionEvent事件中返回发送完成情况
     * 
     * @param fd 被发送的FD
     * 
     * @return true,成功，false,失败
     */
    bool sendFD(FD fd);
    
    
protected:
    FDSendCompletionEvent fdSendCompletionEvent;
protected:
    /**
     * 真正写操作
     */
    virtual ssize_t realWrite(const char* buff, size_t n, off_t offset = -1);

//    /**
//     * 从队列首删除元素
//     */
//    virtual void popQueueFront();    
    
    /**
     * 覆盖直接基类的fireWriteCompletionEvent方法
     * 
     * @see EvNIOWriter#fireWriteCompletionEvent(const char* buff, ssize_t wsize, off_t offset)
     */
    virtual bool fireWriteCompletionEvent(const char* buff, ssize_t wsize, off_t offset);
    
    
private:
    struct msghdr msg;
    struct iovec iov[1];
    
private:

    /**
     * 发送FD
     * @param recvFD 接收进程的FD
     * @param sendedFD 被发送的FD
     * @return 失败返回-1, 成功返回FD类型宽度，FD_SIZE
     */
    ssize_t realSendFD(FD recvFD, FD sendedFD);
    
    
};

#endif	/* PROCESSFDSENDER_H */

