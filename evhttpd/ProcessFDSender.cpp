/* 
 * File:   ProcessFDSender.cpp
 * Author: try
 * 
 * Created on 2011年7月8日, 下午4:52
 */

#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <queue>
#include "ProcessFDSender.h"
#include "MemoryPool.h"

ProcessFDSender::ProcessFDSender():EvNIOWriter(), fdSendCompletionEvent(this) {
}

ProcessFDSender::ProcessFDSender(FD fd):EvNIOWriter(fd), fdSendCompletionEvent(this) {
}

ProcessFDSender::~ProcessFDSender() {
}

bool ProcessFDSender::sendFD(FD fd){
    LOG_DEBUG("fd:%d", fd);
    return EvNIOWriter::write((char*)(&fd), FD_SIZE);
}


bool ProcessFDSender::fireWriteCompletionEvent(const char* buff, ssize_t wsize, off_t offset){
    if(wsize < 0){
        LOG_WARN("wsize:%d, dataQueue.size():%d", wsize, dataQueue.size());
    }
    
    //LOG_DEBUG("wsize:%d, dataQueue.size():%d", wsize, dataQueue.size());
    if(wsize == 0){
        //队列空，发送完成
        return true;
    }
    
    //把事件交给其它监听器处理
    if(wsize==FD_SIZE){
        fdSendCompletionEvent.success = true;
        fdSendCompletionEvent.fd = *((FD*)buff);
    }else{
        fdSendCompletionEvent.success = false;
    }
    if(fire(&fdSendCompletionEvent)){
       //事件已经被处理，返回
       return true; 
    }
    return false;

    LOG_INFO("FDSendCompletionEvent have not been processed");
} 

ssize_t ProcessFDSender::realWrite(const char* buff, size_t n, off_t offset){
    if(n != FD_SIZE){
        LOG_ERROR("n not is FD_SIZE, n:%ld, FD_SIZE:%d", (long)n, FD_SIZE);
        errno = EBADF;
        return -1;
    }
    return realSendFD(fd, *((FD*)buff));
}


ssize_t ProcessFDSender::realSendFD(FD recvFD, FD sendedFD){
    union{
    struct cmsghdr unit; //这里定义辅助数据对象的目的是为了让msg_control 缓冲区和辅助数据单元对齐，所以不是简单地定义一个char control[…]，而是定义一个union
    char control[CMSG_SPACE(FD_SIZE)]; //辅助数据缓冲区
    }control_buf;
    memset(control_buf.control, 0, CMSG_SPACE(FD_SIZE));
    
    struct cmsghdr *unitptr;
    msg.msg_control = control_buf.control;
    msg.msg_controllen = sizeof(control_buf.control);
    unitptr = CMSG_FIRSTHDR(&msg);
    unitptr->cmsg_len = CMSG_LEN(FD_SIZE);
    unitptr->cmsg_level = SOL_SOCKET;
    unitptr->cmsg_type = SCM_RIGHTS;
    *((FD *)CMSG_DATA(unitptr)) = sendedFD; //CMSG_DATA()返回与cmsghdr 相关联的数据的第一个字节的指针
    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    iov[0].iov_base = &sendedFD;
    iov[0].iov_len = FD_SIZE;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    
    ssize_t wsize = ::sendmsg(recvFD, &msg, 0);
    if(wsize > 0){
        return FD_SIZE;
    }
    LOG_DEBUG("sendmsg.wsize:%ld, errno:%d, recvFD:%d, sendedFD:%d", (long)wsize, errno, recvFD, sendedFD);
    if(errno != EAGAIN){
        LOG_WARN("sendmsg error, wsize:%ld, sendedFD:%d, errno:%d, dataQueue.size():%d", (long)wsize, sendedFD, errno, dataQueue.size());
    }
    return -1;
}



