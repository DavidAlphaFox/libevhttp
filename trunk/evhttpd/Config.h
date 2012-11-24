/* 
 * File:   Config.h
 * Author: try
 *
 * Created on 2011年6月11日, 上午10:59
 */

#ifndef CONFIG_H
#define	CONFIG_H
#include "resources.h"
#include "event/IEventListener.h"

/**
 * HTTP Server配置
 */

class HttpHandlerFactory;
class Config {
public:
   
    /** 处理请求的子进程数量, 默认值在WORK_PROCESS_COUNT中定义 */
    unsigned int workProcessCount;
    
    /** 请求读缓冲区大小, 默认值在HTTP_REQUEST_BUFFER_SIZE中定义 */
    unsigned int requestBufferSize;

    /** 请求响应写缓冲区大小，默认值在HTTP_RESPONSE_BUFFER_SIZE中定义 */
    unsigned int responseBufferSize;
    
    /** 允许请求行最大字节数量, 默认：REQUEST_LINE_MAX_BYTES */
    unsigned int requestLineMaxBytes;

    /** 连接空闲超时时间, 单位：秒 */
    double idleTimeout;
     
    /** HttpHandlerPool中最多保留多少个HttpHandler, 默认值：POOL_MAX_HTTP_HANDLER_COUNT */
    unsigned int poolMaxHttpHandlerCount;
    
    /** HttpHandler工厂 */
    HttpHandlerFactory* handlerFactory;
    
    /** 各种事件监听器，注意：监听有可能在父进程中执行，也有可能在子进程中执行 */
    IEventListener* eventListener;
     
public:
    Config():
        workProcessCount(WORK_PROCESS_COUNT),
        requestBufferSize(HTTP_REQUEST_BUFFER_SIZE),
        responseBufferSize(HTTP_RESPONSE_BUFFER_SIZE),
        requestLineMaxBytes(REQUEST_LINE_MAX_BYTES),    
        idleTimeout(IDLE_TIMEOUT),
        poolMaxHttpHandlerCount(POOL_MAX_HTTP_HANDLER_COUNT),
        handlerFactory(NULL),
        eventListener(NULL)
    {}
    virtual ~Config(){}

};

#endif	/* CONFIG_H */

