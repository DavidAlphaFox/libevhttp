/* 
 * File:   events.h
 * Author: try
 *
 * Created on 2011年6月18日, 下午10:41
 */

#ifndef EVENTS_H
#define	EVENTS_H

#include "resources.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "event/Event.h"

/**
 * 定义各种事件
 */


//事件类型，子进程被启动
#define EVENT_TYPE_PROCESS_STARTED 1001

//事件类型，完成IO读事件, 用于IOReader中
#define EVENT_TYPE_IO_READ_COMPLETION 1002

//事件类型，完成IO写事件, 用于IOWriter中
#define EVENT_TYPE_IO_WRITE_COMPLETION 1003

//事件类型，完成Send FD事件, 用于ProcessFDSender中
#define EVENT_TYPE_SEND_FD_COMPLETION 1004

////事件类型, Pipeline数据传输事件
//#define EVENT_TYPE_PIPELINE_TRANSPORT 1005

//事件类型, IO数据传输事件
#define EVENT_TYPE_IO_TRANSPORT 1005

//事件类型，完成部份Body数据构建
#define EVENT_TYPE_BODY_BUILD_COMPLETION 1006

//事件类型, 信号, 用于ProcessSignal类型
#define EVENT_TYPE_PROCESS_SIGNAL 1007

//事件类型, 向子进程或父进程发送消息事件
#define EVENT_TYPE_PROCESS_SEND_MESSAGE 1008

//事件类型, 从子进程或父进程接收消息事件
#define EVENT_TYPE_PROCESS_RECEIVE_MESSAGE 1009

//事件类型, 定时器超时事件s
#define EVENT_TYPE_TIMER 1010

/**
 * 子进程启动事件，当启动子进程后在在子进程环境里触发此事件
 */
class ProcessStartedEvent : public Event{
public:
      ProcessStartedEvent(bool inChildProcess, void* source=NULL):Event(EVENT_TYPE_PROCESS_STARTED, source),
      inChildProcess(inChildProcess){
      }
      virtual ~ProcessStartedEvent(){}
      
      /** 事件来源位置，值true说明是在子进程，false是在父进程中 */
      bool inChildProcess;
};


/**
 * IO读完成事件，当IOReader读操作完成后触发此事件
 */
class IOReadCompletionEvent : public Event{
public:
      IOReadCompletionEvent(void* source=NULL):Event(EVENT_TYPE_IO_READ_COMPLETION, source), 
              dataBuff(NULL), length(0), offset(-1){
      }
      virtual ~IOReadCompletionEvent(){}
      
public:
    /** 读出的数据 */
    char* dataBuff;
    /** 实际读出的数据量，如果等于-1，发生错误， 等于0，读到EOF */
    ssize_t length;
    /** 本次提交读请求时read方法的offset参数值 */
    off_t offset;
      
};


class IIOPump;
/**
 * IO写完成事件，当IOWriter写操作完成后触发此事件
 */
class IOWriteCompletionEvent : public Event{
public:
      IOWriteCompletionEvent(void* source=NULL):Event(EVENT_TYPE_IO_WRITE_COMPLETION, source), 
              length(0), offset(-1), dataBuff(NULL), isSendfile(false), ioPumper(NULL){
      }
      virtual ~IOWriteCompletionEvent(){}
      
public:
    /** 写入的数据, 当发生错误或isSendfile时dataBuff值可能为NULL */
    const char* dataBuff;
    /** 实际写入的数据量，-1:发生错误, 0：说明所有写队列中的数据已经写完成 */
    ssize_t length;
    /** 本次提交写请求时write方法的offset参数值 */
    off_t offset;

    bool isSendfile;   //标记是否发送文件, 当isSendfile被设置时，iopumper不为空
    IIOPump* ioPumper; //如果iopumper不为NULL, 说明通过IIOPump来写数据
};


///**
// * IO写事件, 当删除写队列元素时触发此事件
// */
//class IOWriteRemoveQueueElementEvent : public Event{
//public:
//      IOWriteRemoveQueueElementEvent(void* source=NULL):Event(EVENT_TYPE_IO_WRITE_REMOVE_QUEUE_ELEMENT, source), 
//              length(0), offset(-1), dataBuff(NULL), isSendfile(false), ioPumper(NULL){
//      }
//      virtual ~IOWriteRemoveQueueElementEvent(){}
//      
//public:
//    /** 数据, isSendfile时dataBuff值可能为NULL */
//    const char* dataBuff;
//    /** 数据字节长度  */
//    ssize_t length;
//    /** 本次提交写请求时write方法的offset参数值 */
//    off_t offset;
//
//    bool isSendfile;   //标记是否发送文件, 当isSendfile被设置时，iopumper不为空
//    IIOPump* ioPumper; //如果iopumper不为NULL, 说明通过IIOPump来写数据
//};

/**
 * 完成Send FD事件
 */
class FDSendCompletionEvent : public Event{
public:
      FDSendCompletionEvent(void* source=NULL):Event(EVENT_TYPE_SEND_FD_COMPLETION, source), 
              fd(-1), success(false){
      }
      virtual ~FDSendCompletionEvent(){}
      
public:
      FD fd;
      bool success;;
      
};

///**
// * Pipeline数据传输事件
// * 
// * 注意: writeEvent有可能为NULL
// */
//class PipelineTransportEvent : public Event{
//public:
//      PipelineTransportEvent(void* source=NULL):Event(EVENT_TYPE_PIPELINE_TRANSPORT, source),
//        readEvent(NULL),writeEvent(NULL){}
//      virtual ~PipelineTransportEvent(){}
//      
//   public:
//      const IOReadCompletionEvent* readEvent;
//      const IOWriteCompletionEvent* writeEvent;
//      
//};

/**
 * IO数据传输事件
 * 
 * 注意: writeEvent有可能为NULL
 */
class IOTransportEvent : public Event{
public:
      IOTransportEvent(void* source=NULL):Event(EVENT_TYPE_IO_TRANSPORT, source),
        readEvent(NULL),writeEvent(NULL){}
      virtual ~IOTransportEvent(){}
      
   public:
      const IOReadCompletionEvent* readEvent;
      const IOWriteCompletionEvent* writeEvent;
      
};

class BodyBuildCompletionEvent : public Event{
public:
      BodyBuildCompletionEvent(void* source=NULL):Event(EVENT_TYPE_BODY_BUILD_COMPLETION, source), 
              dataBuff(NULL), length(0){
      }
      virtual ~BodyBuildCompletionEvent(){}
      
public:
    /** 读出的数据 */
    const char* dataBuff;
    /** 实际读出的数据量, 值为0，表示数据已经读完，-1，出错 */
    ssize_t length;
};


class ProcessSignalEvent : public Event{
public:
      ProcessSignalEvent(void* source=NULL):Event(EVENT_TYPE_PROCESS_SIGNAL, source), 
              signal(-1){
      }
      virtual ~ProcessSignalEvent(){}
      
public:
    /** 信号 */
    int signal;
    
};


class ProcessSendMessageEvent : public Event{
public:
      ProcessSendMessageEvent(void* source=NULL):Event(EVENT_TYPE_PROCESS_SEND_MESSAGE, source), 
          succeed(true), childProcess(false)    {
      }
      virtual ~ProcessSendMessageEvent(){}
      
public:
    /** 状态, 发送是否成功 */
    bool succeed;
    /** 消息 */
//    ProcessMessage_t* message;
    /** 是否子进程 */
    bool childProcess;    
};

class ProcessReceiveMessageEvent : public Event{
public:
      ProcessReceiveMessageEvent(void* source=NULL):Event(EVENT_TYPE_PROCESS_RECEIVE_MESSAGE, source), 
          succeed(true), message(NULL),childProcess(false)    {
      }
      virtual ~ProcessReceiveMessageEvent(){}
      
public:
    /** 状态, 接收是否成功, 只有成功接收到消息时，message才有意义 */
    bool succeed;
    /** 消息 */
    ProcessMessage_t* message;
    /** 是否子进程 */
    bool childProcess;
    
};


/** 定时器事件 */
class TimerEvent : public Event{
public:
      TimerEvent(void* source=NULL):Event(EVENT_TYPE_TIMER, source)
      {
      }
      virtual ~TimerEvent(){}
      
public:
    
    
};


#endif	/* EVENTS_H */

