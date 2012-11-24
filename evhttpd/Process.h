/* 
 * File:   Process.h
 * Author: try
 *
 * Created on 2011年5月26日, 上午10:18
 */

#ifndef _PROCESS_H
#define	_PROCESS_H

#include <vector>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "resources.h"
#include "IRunnable.h"
#include "event/EventDispatcher.h"
#include "event/IEventListener.h"
#include "ProcessFDSender.h"
#include "SocketNIOWriter.h"
#include "SocketNIOReader.h"

/**
 * 进程基类
 */
class Process : public EventDispatcher, public IEventListener, protected IRunnable{
public:
    Process(bool sendFDEnable=true, bool sendMessageEnable=true);
    virtual ~Process();

    /**
     * 启动进程, 参数为向子进程传递的初始参数, 返回true, 成功创建进程, 否则返回false
     */
    virtual bool start(void* param=NULL);

    /**
     * 返回子进程ID
     */
    pid_t getId(){
        return pid;
    }

    /**
     * 返回子进程或父进程之间的消息通道FD描述符
     */
    FD getCmdFD(){
        return msgfd;
    }


    
    /**
     * 返回子进程或父进程之间的FD传输通道描述符
     */
    FD getFdFD(){
        return fdfd;
    }    
    
   
    /** 
     * 检查子进程是否活动
     */
    bool isAlive();
    
    /**
     * 检查是否正在退出过程中
     */
    bool isExiting();
    
    /**
     * 检查子进程是否结束
     */
    bool isExit();

    /**
     * 返回是否子进程
     */
    bool isChildProcess(){
        return childProcess;
    }
    
    /**
     * 返回进程退出状态, 如果进程还未开始或结束，返回0
     * 退出状态定义同：wait或waitpid方法的stat_loc值
     */
    int getExitStatus();

    /**
     * 向子进程发送Socket描述符
     */
    virtual bool sendFD(FD fd);

    /**
     * 接收父进程发送来的Socket描述符
     * 
     * @param fd 在fd中返回读到的描述符
     */
    virtual bool recvFD(FD* fd);


    /**
     * 向子进程或父进程发送消息
     */
    virtual bool sendMessage(MSG_TYPE msg, void* buff=NULL, unsigned int n=0);    
    
    /**
     * 向子进程或父进程发送消息
     */    
    virtual bool sendMessage(ProcessMessage_t* msg);
    
    /**
     * 向子进程或父进程发送退出消息, 如果是子进程向主进程发送的此消息，
     * 说明子进程即将退出, 如果是主进程向子进程发送的此消息，说明通知子进程赶快退出，
     * 在主进程或子进程中将收到MSG_TYPE_EXIT类型的ProcessReceiveMessageEvent事件
     */
    virtual void notifyClose();
    
    /**
     * 关闭相关资源
     */
    virtual void close();        
    
protected:
    /**
     * 进程开始执行, 参数是从主进程输入子进程的参数，返回值为进程退出码，[在子进程中执行]
     * 
     * @see IRunnable#run(void* param=NULL){
     */
    virtual int run(void* param=NULL){
        return 0;
    }


    /**
     * 实现IEventListener接口后，作为事件监听器身份来处理到的事件, 
     * 返回false, 将继续处理事件，否则中止事件处理
     * 
     * @see IEventListener#handle(const Event* e)
     */
    virtual bool handle(const Event* e);
    
    /**
     * 关闭父子进程间的消息通道FD描述符
     */
    void closeCmdFD(){
        if(msgfd != -1 ){ 
            ::close(msgfd);
            msgfd = -1;
        }
    }    
    
    /**
     * 关闭父子进程间的FD传输通道描述符
     */
    void closeFdFD(){
        if(fdfd != -1){ 
            ::close(fdfd);
            fdfd = -1;
        }
    }            
    
   
protected:
    
    /**
     * 当成功开启一个子进程时调用，[在父进程中执行]
     */
    virtual bool onProcessStartedEvent(const ProcessStartedEvent* e){
        return false;
    }
    
    /**
     * 当向子进程发送FD完成时事件
     */
    virtual bool onFDSendCompletionEvent(const FDSendCompletionEvent* e);
    
    /**
     * 发送消息完成事件
     */
    virtual bool onSendMessageEvent(const ProcessSendMessageEvent* e){
        return false;
    }
    
    /**
     * 接收消息完成事件
     */
    virtual bool onReceiveMessageEvent(const ProcessReceiveMessageEvent* e); 
    
protected:

    //线程执行目标
    IRunnable& target;    
    //子进程ID
    pid_t pid;
    //进程退出状态, 默认值: 0
    int exitStatus;
    //标识进程正退出过程中
    bool exiting;
    //进程是否已经退出
    bool exitFlag;
    //用于父子进程间传输socket描述符，fds[0]为父进程描述符, fds[1]为子进程描述符
    FD fdfds[2];
    //指向父进程或子进程的FD传输通道描述符
    FD& fdfd;
            
    //用于父子进程间传输消息的描述符, msgfds[0]为父进程连接描述符，msgfds[1]为子进程连接描述符
    FD msgfds[2];
    //指向父进程或子进程的消息通道描述符
    FD& msgfd;
    
    //标记是否子进程, 值为:true，说明是子进程，否则为父进程
    bool childProcess;
    
    //指示是否允许在进程间传送消息, 默认:true
    bool sendMessageEnable;
    //指示是否允许在进程间发送FD, 默认:true
    bool sendFDEnable;
protected:
    //用于向子进程发送FD
    ProcessFDSender fdSender;
    
    //用于向父进程或子进程写消息的IIOWriter
    SocketNIOWriter msgIOWriter;
    
    //用于从父进程或子进程读消息的IIOReader
    SocketNIOReader msgIOReader;
private:
    //标记MSG_TYPE消息头是否已经准备好, 当成功读出消息头时时，设置此值, 复位时清除此标记
    bool msgExist;
    
private:
    struct msghdr msg;
    struct iovec iov[1];

private:
    inline void check();
    
private:
    char* receiveMessageData; //receiveMessage的data数据，如果有的话
    size_t receiveMessageDataLength; //当前receiveMessageData长度
    ProcessMessage_t receiveMessage;
    ProcessReceiveMessageEvent receiveMessageEvent;
    
    ProcessMessage_t sendedMessage;
    ProcessSendMessageEvent sendMessageEvent;
    
private:
    void fireProcessStartedEvent(const ProcessStartedEvent* e);
    
    bool prepareReadMessage();   
   
    bool prepareReadMessageData();
       
    bool handleReadMessageCompletionEvent(IOReadCompletionEvent* e);
    
    bool fireProcessSendMessageEvent(const ProcessSendMessageEvent* e);
    
    bool handleWriteMessageCompletionEvent(IOWriteCompletionEvent* e);    
    
    bool fireProcessReceiveMessageEvent(const ProcessReceiveMessageEvent* e);
    
};

#endif	/* _PROCESS_H */

