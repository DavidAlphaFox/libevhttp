/* 
 * File:   Response.h
 * Author: try
 *
 * Created on 2011年6月2日, 下午11:19
 */

#ifndef _RESPONSE_H
#define	_RESPONSE_H
#include "resources.h"
#include "libev.h"
#include <vector>
#include "Request.h"
#include <ext/hash_map>
#include <string>
#include "KeyValues.h"
#include "Config.h"
#include "MemoryPool.h"
#include "SendfileIOPump.h"

/**
 * 请求响应信息, 设置响应行，响应头以及主体数据等
 */
class HttpHandler;
class Response {
    friend class HttpHandler;
public:
    Response(HttpHandler& httpHandler, Request& request, Config* config);
    virtual ~Response();

    /** 新增或追加响应头, 多个值以:" ,"号分隔 */
    void addHeader(const char* name, const char* value);

    /** 新增或替换响应头 */
    void setHeader(const char* name, const char* value);

    /** 新增或追加响应头, 值为int类型 */
    void addHeader(const char* name, int value);

    /** 新增或追加响应头, 值为int类型 */
    void setHeader(const char* name, int value);
    
    /** 新增或追加响应头， 值为时间戳，精确到(秒) */
    void addDateHeader(const char* name, long int value);

    /** 新增或追加响应头， 值为时间戳，精确到(秒) */
    void setDateHeader(const char* name, long int value);    
    
    /** 设置内容长度 */
    void setContentLength(long len);

    /** 设置内容数据类型 */
    void setContentType(const char* type);

    /** 向客户端写数据，b不能为NULL, 且strlen(b+off)必须大于等于len*/
    bool write(const char* b, unsigned int off, unsigned int len);

    /** 向客户端写数据，b不能为NULL, 且strlen(b)必须大于等于len*/
    bool write(const char* b, unsigned int len);

    /** 向客户端写数据，str为以0结尾的字符串 */
    bool write(const char* str);

    /** 向客户端写入一个long类型数据 */
    bool write(long value);
    
    /** 向客户端写入一个int类型数据 */
    bool write(int value);    
    
    /** 
     * 发送文件
     * @param fd 文件FD
     * @param readOffset 从文件readOffset所指位置开始读文件
     * @param len 发送内容长度, 必须大于0
     * @param speed 发送速度，字节单位，如果值为0，不限速
     * @param maxSendSize, 最多一次发送的数据量
     */
    bool sendfile(FD fd, off_t readOffset, size_t len, unsigned int speed, size_t maxSendSize=DEFAULT_DATA_SIZE);
    
    /** 刷新写缓冲 */
    bool flushBuffer();
    
    /** 设置响应状态码 */
    void setStatus(unsigned short int status, const char* msg=NULL);
    
    /** 返回状态码 */
    unsigned short int getStatusCode();

    /** 返回状态消息 */
    const char* getStatusMessage();    
    
    
    /** 返回正准备向客户端发送的数据块数量 */
    size_t waitingWriteQueueSize();
    
    /** 
     * 检查响应头是否已经发送出去
     * 
     * 注意：headerSended()只表示头已经被写入到发送队列，并不一定真的已经发送到客户端, 
     *       如果要保证头已经被成功发送，需要使用waitingWriteQueueSize()检测发送队列是否空
     */
    bool headerSended();
    
    /**
     * 请求应答模式, 如果要异步处理请求，在HttpServlet.service方法返回前设置setAsynAnswerMode(true), 
     * 在异步模式下，处理完请求后需要调用complete方法通知请求处理完成。
     * 
     * @param b true，为同步模式(默认), false, 为异步模式
     */
    void setAsynAnswerMode(bool b);
    
    /**
     * 检查是否异步应答方式
     */
    bool isAsynAnswerMode();
    
    /**
     * 通知请求处理完成, 用于以异步方式处理请求时，告诉框架请求已经处理结束
     */
    void complete();
    
private:
    HttpHandler& httpHandler;   //指向HttpHandler
    Request& request;          //请求对象
    unsigned short int status; //HTTP状态码, 默认为:200, @TODO 将此状态码直接指向httpHandler的httpCode
    std::string msg;
    unsigned int headerLength;          //响应头长度，默认为:0
    bool headerWriteFlag;        //如果已经输出响应头，设置此标记
    bool isChunkedTransferEncoding;  //内容传输方式是否：chunked方式
    long contentLength;         //内容长度，默认为:-1
    unsigned int bufferSize;   //写缓冲区大小，默认为:8192字节，
    unsigned int bufferPointer;    //当前写指针位置，默认为:0
    char* buffer; //写主体内容Buffer
    std::vector<char> headerBuffer; //写消息头Buffer
    bool asynAnswerMode; //是否异步应该方式，值true为异步应答，false为同步方式(默认)

    //消息头
    typedef KeyValues<HashMapStrIgnoreCaseCompare> Headers;
    Headers headers;

    //定义MemoryPool::Group内存，方便批量释放
    MemoryPool::Group memGroup;
    
    //用于发送文件的IOPump
    typedef std::vector<SendfileIOPump*> IOPumpers;
    IOPumpers ioPumpers;
    
private:
    //写响应行和响应头信息
    bool flushHeaders();

    /** 刷新写缓冲, flushEmpty指定是否对空缓冲进行处理 */
    bool writeBuffer(bool flushEmpty);
    
    /** 清理 */
    void clean();
    
    /** 向客户端发送回车换行符 */
    bool sendln();
};

#endif	/* _RESPONSE_H */

