/* 
 * File:   Process.cpp
 * Author: try
 * 
 * Created on 2011年5月26日, 上午10:18
 */

#include <iostream>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
//#include <bits/stl_vector.h>
#include "Process.h"
#include "events.h"
#include "SocketUtils.h"
#include "MemoryPool.h"

using namespace std;

//一次对receiveMessage分配的空间
//#define RECEIVE_MESSAGE_EACH_RESERVE sizeof(ProcessMessage_t)

Process::Process(bool sendFDEnable, bool sendMessageEnable):target(*this), pid(0),exitStatus(0), exiting(false), exitFlag(false),
        fdfd(fdfds[1]), msgfd(msgfds[1]), childProcess(false), sendFDEnable(sendFDEnable), sendMessageEnable(sendMessageEnable), msgExist(false), 
    receiveMessageData(NULL), receiveMessageDataLength(0), receiveMessageEvent(this), sendMessageEvent(this)
{
    fdfd = -1;
    msgfd = -1;
    memset(&receiveMessage, 0, sizeof(ProcessMessage_t));
    memset(&sendedMessage, 0, sizeof(ProcessMessage_t));
    receiveMessageEvent.message = &receiveMessage;
    
}

Process::~Process() {
    LOG_DEBUG("");
    close();
    if(receiveMessageData){
        MemoryPool::instance.free(receiveMessageData);
        receiveMessageDataLength = 0;
        receiveMessageData = NULL;
    }    
}

void Process::close(){
    
    if(sendMessageEnable){
        msgIOReader.close();
        msgIOWriter.close();
    }
    if(sendFDEnable){
        fdSender.close();
    }
    
    closeCmdFD();
    closeFdFD();
}

bool Process::start(void* param /* =NULL */){
    if(pid){
        return false;//进程已经启动
    }

    //创建父子间通讯的socket pair
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fdfds) < 0) {
        return false;//创建父子进程通讯描述符失败
    }    

    //创建父子间通讯的socket pair
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, msgfds) < 0) {
        return false;//创建父子进程通讯描述符失败
    }
    
    pid = fork();
    switch(pid){
        case -1:
            return false;
        case 0:
            //进入子进程
            ::close(msgfds[1]); //关闭子进程自己描述符
            ::close(fdfds[1]);
            
            msgfd = msgfds[0];
            fdfd = fdfds[0];
            
            pid = getpid();
            
            if(sendMessageEnable){
                //用于处理父进程消息事件
                msgIOReader.setFD(msgfd);
                msgIOReader.addListener(EVENT_TYPE_IO_READ_COMPLETION, this);
                prepareReadMessage(); //准备读父进程发送来的消息

                //用于处理父进程消息事件
                msgIOWriter.setFD(msgfd);
                msgIOWriter.addListener(EVENT_TYPE_IO_WRITE_COMPLETION, this);  
                msgIOWriter.start();
            }
            
            childProcess = true;
            //触发子进程启动事件, 在子进程中
            ProcessStartedEvent psEvent(childProcess, this);
            fireProcessStartedEvent(&psEvent);
            
            //进入子进程执行
            int code = target.run(param);
            exit(code);
            return true;
    }
    ::close(msgfds[0]); //关闭父进程自己描述符
    ::close(fdfds[0]);

    if(sendFDEnable){
        fdSender.setFD(fdfd);
        fdSender.addListener(EVENT_TYPE_SEND_FD_COMPLETION, this);
        fdSender.start();
    }
    
    if(sendMessageEnable){
        msgIOReader.setFD(msgfd);
        msgIOReader.addListener(EVENT_TYPE_IO_READ_COMPLETION, this);
        prepareReadMessage(); //准备读子进程发送来的消息

        msgIOWriter.setFD(msgfd);
        msgIOWriter.addListener(EVENT_TYPE_IO_WRITE_COMPLETION, this);
        msgIOWriter.start();
    }
    
    childProcess = false;
    //触发子进程启动事件, 在父进程中
    ProcessStartedEvent psEvent(childProcess, this);
    fireProcessStartedEvent(&psEvent);

    return true;
}

void Process::fireProcessStartedEvent(const ProcessStartedEvent* e){
    if(onProcessStartedEvent(e)){
        return; //事件已经被处理，返回
    }
    
    //向其它监听器转发事件
    fire(e);
}


bool Process::fireProcessSendMessageEvent(const ProcessSendMessageEvent* e){
    if(onSendMessageEvent(e)){
        return true; //事件已经被处理，返回
    }

    //向其它监听器转发事件
    return fire(e);
}

bool Process::fireProcessReceiveMessageEvent(const ProcessReceiveMessageEvent* e){
    if(onReceiveMessageEvent(e)){
        return true; //事件已经被处理，返回
    }

    //向其它监听器转发事件
    return fire(e);
}


bool Process::handle(const Event* e){
    LOG_DEBUG("childProcess:%d", childProcess);
    switch(e->getType()){
        case EVENT_TYPE_SEND_FD_COMPLETION:
            if(!childProcess){
                if(onFDSendCompletionEvent((const FDSendCompletionEvent*)e)){
                    return true;
                }
            }
            break;
        case EVENT_TYPE_IO_READ_COMPLETION:
            return handleReadMessageCompletionEvent((IOReadCompletionEvent*)e);
        case EVENT_TYPE_IO_WRITE_COMPLETION:
            return handleWriteMessageCompletionEvent((IOWriteCompletionEvent*)e);
    }    
    return false;
}

bool Process::onFDSendCompletionEvent(const FDSendCompletionEvent* e){
//    LOG_DEBUG("e->fd:%d, e->success:%d", e->fd, e->success);
    if(!e->success){
        LOG_WARN("send FD error, fd is %d", e->fd);
    }
    //不管成功与否都要将此描述符关闭
    ::close(e->fd);
    return true;
}

bool Process::isAlive(){
    check();
    return pid && !exitFlag;
}

/**
    * 检查是否正在退出过程中
    */
bool Process::isExiting(){
    return exiting;
}

bool Process::isExit(){
    check();
    return exitFlag;
}

int Process::getExitStatus(){
    check();
    return exitStatus;
}

void Process::check(){
    if(pid && !exitFlag){
        errno = 0;
        pid_t tpid = waitpid(pid, &exitStatus, WNOHANG);
        LOG_DEBUG("tpid:%d, pid:%d, exitStatus:%d, errno:%d, %d, %d, %d, %d", tpid, pid, exitStatus, errno, WIFEXITED(exitStatus), WEXITSTATUS(exitStatus), WIFSIGNALED(exitStatus), WTERMSIG(exitStatus));
        if(tpid == pid || (tpid == -1 && errno == ECHILD)){
            exitFlag = true;
        }
    }
}

bool Process::sendFD(FD fd){
    if(exiting){
        LOG_DEBUG("exiting.");
        return false;
    }
    return fdSender.sendFD(fd);
}


bool Process::recvFD(FD* fd){
    static int srcfd;
//    struct msghdr msg; // 辅助数据对象
//    struct iovec iov[1];
    union{
        struct cmsghdr unit; //这里定义辅助数据对象的目的是为了让msg_control 缓冲区和辅助数据单元对齐，所以不是简单地定义一个char control[…]，而是定义一个union
        char control[CMSG_SPACE(INT_SIZE)]; //辅助数据缓冲区
    }control_buf;
    struct cmsghdr *unitptr;
    msg.msg_control = control_buf.control; //辅助数据缓冲区
    msg.msg_controllen = sizeof(control_buf.control); //辅助数据大小
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    iov[0].iov_base = &srcfd;
    iov[0].iov_len = INT_SIZE;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;
    
    //errno = 0;
    ssize_t rsize = recvmsg(fdfd, &msg, 0);
    LOG_DEBUG("rsize:%d", rsize);
    if (rsize <= 0){
        LOG_WARN("recvmsg error 1, rsize:%d, FD_SIZE:%d, errno:%d", rsize, FD_SIZE, errno);
        return false;
    }
    
    if ((unitptr = CMSG_FIRSTHDR(&msg)) != NULL && unitptr->cmsg_len == CMSG_LEN(INT_SIZE)){ //cmsg_len 应该=CMSG_LEN 宏得出的结果
        if (unitptr->cmsg_level == SOL_SOCKET && unitptr->cmsg_type == SCM_RIGHTS){ //UNIX DOMAIN 用SOL-SOCKET
            *fd = *((FD *)CMSG_DATA(unitptr)); //获得该辅助对象的数据
            //cout<<"recvmsg, fd:"<<*fd<<endl;
            return true;
        }
    }
    LOG_WARN("recvmsg error 2, rsize:%d", rsize);
    return false;
}


bool Process::prepareReadMessage(){
//    receiveMessage.clear();
    msgExist = false;
    return msgIOReader.read((char*)&receiveMessage, sizeof(ProcessMessage_t), true);
}

bool Process::prepareReadMessageData(){
    LOG_DEBUG("type:%d, pid:%d, length:%d", receiveMessage.type, receiveMessage.pid, receiveMessage.length);
    //receiveMessage.reserve(receiveMessage.size()+receiveMessagePt->length);
//    reserveReceiveMessage(receiveMessagePt->length);
    if(receiveMessage.length <= 0){
        return true;
    }
    
    if(!receiveMessageData){
        receiveMessageData = (char*)MemoryPool::instance.malloc(receiveMessage.length);
        receiveMessageDataLength = receiveMessage.length;
    }else if(receiveMessage.length > receiveMessageDataLength){
        receiveMessageData = (char*)MemoryPool::instance.realloc(receiveMessageData, receiveMessage.length);
        receiveMessageDataLength = receiveMessage.length;
    }
    receiveMessage.data = receiveMessageData;
    return msgIOReader.read(receiveMessageData, receiveMessage.length, true);
}

//void Process::resetMessage(){
////    command = 0;
//    receiveMessage.clear();
//    msgExist = false;
//}


void Process::notifyClose(){
    exiting = true;
    sendMessage(MSG_TYPE_EXIT, NULL, 0);
//    msgIOReader.close();
//    msgIOWriter.close();
//    fdSender.close();        
}

/**
 * 向子进程或父进程发送消息
 */
bool Process::sendMessage(MSG_TYPE msg, void* buff, unsigned int n){
    LOG_DEBUG("msg:%d, n:%d", msg, n)
    sendedMessage.type = msg;
    sendedMessage.pid = getpid();
    sendedMessage.length = n;
    if(n > 0){
        if(buff){
            sendedMessage.data = buff;
        }else{
            LOG_ERROR("n > 0 but buff is NULL");
            return false;
        }
    }
    return sendMessage(&sendedMessage);
}

/**
 * 向子进程或父进程发送消息
 */
bool Process::sendMessage(ProcessMessage_t* msg){
    if(!isChildProcess() && !isAlive()){
        return false;
    }
    LOG_DEBUG("type:%d, length:%d", msg->type, msg->length);
    if(!msgIOWriter.write((const char*)msg, sizeof(ProcessMessage_t))){
        LOG_WARN("send message error, msg:%d, n:%d, errno:%d", msg->type, msg->length, errno)
        return false;
    }
    if(msg->data && msg->length > 0){
        if(!msgIOWriter.write((const char*)msg->data, msg->length)){
            LOG_WARN("send message data error, n:%d, errno:%d", msg->length, errno)
            return false;
        }
    }
    return true;
}

/**
 * 处理从子进程或父进程中读到的消息数据
 */
bool Process::handleReadMessageCompletionEvent(IOReadCompletionEvent* e){
    LOG_DEBUG("readsize:%d, msgExist:%d", e->length, msgExist);

    receiveMessageEvent.childProcess = childProcess;
    if(e->length <= 0){
        //exitProcess();
        receiveMessageEvent.succeed = false;
        return fireProcessReceiveMessageEvent(&receiveMessageEvent);
    }else{
        if(!msgExist && receiveMessage.length > 0){
            msgExist = true;
            return prepareReadMessageData();
        }else{
            //触发事件
            receiveMessageEvent.succeed = true;
//            receiveMessageEvent.message->data = receiveMessage.data()+sizeof(ProcessMessage_t);
            bool handleRs = fireProcessReceiveMessageEvent(&receiveMessageEvent);
//            resetMessage();
            prepareReadMessage();
            return handleRs;
         }
    }
    
}


/**
 * 处理从子进程或父进程中读到的消息数据
 */
bool Process::handleWriteMessageCompletionEvent(IOWriteCompletionEvent* e){
    LOG_DEBUG("writesize:%d, msgIOWriter.waitingQueueSize():%d", e->length, msgIOWriter.waitingQueueSize());
    
    sendMessageEvent.childProcess = childProcess;
    if(e->length < 0){
        sendMessageEvent.succeed = false;
    }else{
        sendMessageEvent.succeed = true;
    }
    return fireProcessSendMessageEvent(&sendMessageEvent);
}



    /**
     * 接收消息完成事件
     */
    bool Process::onReceiveMessageEvent(const ProcessReceiveMessageEvent* e){
        if(e->succeed){
            if(e->message->type == MSG_TYPE_EXIT){
                LOG_DEBUG("type:MSG_TYPE_EXIT, childProcess:%d", e->childProcess);
                exiting = true;
            }
        }
        
        return false;
    } 


