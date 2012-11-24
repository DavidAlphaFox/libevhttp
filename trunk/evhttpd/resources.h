/* 
 * File:   resources.h
 * Author: try
 *
 * Created on 2011年5月27日, 上午11:09
 */

#ifndef _RESOURCES_H
#define	_RESOURCES_H

//#define LOG_LEVEL LOG_DEBUG_LEVEL
//#define LOG_LEVEL LOG_INFO_LEVEL
//#define LOG_LEVEL LOG_ERROR_LEVEL
#define LOG_LEVEL LOG_WARN_LEVEL

#include <ext/hash_map>
#include <string.h>
#include <netinet/in.h>
#include "log.h"

/** 定义主进程与子进程间的交互消息类型, 宽度：一个字节, 
 * 100以内的消息类型为系统保留，100以上可以用户自定义使用 */
typedef char MSG_TYPE;

//自定义消息
#define MSG_TYPE_USER_DEFINED 1
//查询进程当前活动Handler数量
#define MSG_TYPE_QUERY_ACTIVE_HANDLERS 2
//查询进程所有已经完成的请求数量
#define MSG_TYPE_QUERY_TOTAL_REQUESTS 3
//查询进程当前所有HttpHandler数量
#define MSG_TYPE_QUERY_HTTP_HANDLERS 4
//通知退出消息, 一般为主进程通知子进程需要退出
#define MSG_TYPE_EXIT 5

//处理请求的进程数量
#define WORK_PROCESS_COUNT 1

//默认数据大小，
#define DEFAULT_DATA_SIZE 1024*1024

//读缓冲区大小, 默认:1024*20
#define HTTP_REQUEST_BUFFER_SIZE 1024*20

//写缓冲区大小, 默认:1024*20
#define HTTP_RESPONSE_BUFFER_SIZE 1024*20

//允许的最大IO读或写频率,单位：秒
#define MAX_IO_READ_WRITE_INTERVAL 5.0

//CleanerTimer定时器处理间隔，单位: 秒
#define CLEANER_TIMER_INTERVAL 10.0

//定义FD
typedef int FD;

//FD类型值宽度
static const unsigned int FD_SIZE = sizeof(FD);

//int类型值宽度
static const unsigned int INT_SIZE = sizeof(int);

//int类型值宽度
static const unsigned int LONG_SIZE = sizeof(long);

////向子进程发送FD失败且errno为EAGAIN时尝试重复发送FD最多次数
//#define EAGAIN_SEND_FD_MAX_TIMES 30

//连接空闲超时时间, 单位：秒
#define IDLE_TIMEOUT 30

//释放HttpServlet超时时间，默认为:1秒，超过1秒的HttpServlet项将会被清理
#define HTTP_SERVLET_FREE_TIMEOUT 1

//HttpHandlerPool中最多保留多少个HttpHandler, 0为没限制
#define POOL_MAX_HTTP_HANDLER_COUNT 0

//支持版本定义
static const char* HTTP_1_0 = "HTTP/1.0";
static const char* HTTP_1_1 = "HTTP/1.1";
static const int PROTOCOL_LENGTH = 8;

/* 在HTTP/1.0,或HTTP/1.1 中已经实现的请求方式 */
static const char* METHOD_GET = "GET";
static const char* METHOD_POST = "POST";
static const char* METHOD_HEAD = "HEAD";
static const char* METHOD_PUT = "PUT";

//以下是仅在1.1中实现的请求方式

/* 字符常量 */
//空格
#define CHAR_SP 32
//制表符
#define CHAR_HT 9
//回车
#define CHAR_CR 13
//换行
#define CHAR_LF 10
//?号
#define CHAR_QUESTION '?'
//=号
#define CHAR_EQ '='
//:号
#define CHAR_COLON ':'

//回车换行符，\r\n
#define STR_CRLF "\r\n"

/* 允许请求行最大字节数量, 默认：16384 */
#define  REQUEST_LINE_MAX_BYTES 16384

/*  HTTP CODE 定义 */
/*
 HTTP 错误及相应错误码说明

  1. 当method错误时, 即不为 GET或POST
      HTTP/1.1 501 Not Implemented
  2. HTTP版本错误，即不为HTTP/1.0或HTTP/1.1
      HTTP/1.1 505 HTTP Version Not Supported
  3. 内部服务器错误
      HTTP/1.1 500 Internal Server Error
  4. 400 错误请求
     HTTP/1.1 400 Bad RequestBak
 */
//400 错误请求   (HTTP/1.1 400 Bad RequestBak)
#define HTTP_CODE_400 400
//405 当method错误时, 即不为 GET或POST或其它规范允许的值 (HTTP/1.1 405 Not Implemented)
#define HTTP_CODE_405 405
//414 请求URI太大 (HTTP/1.1 400 URI too large)
#define HTTP_CODE_414 414
//500 内部服务器错误 (HTTP/1.1 500 Internal Server Error)
#define HTTP_CODE_500 500
//501 未被使用
//#define HTTP_CODE_501 501
//505 HTTP协议错误，即不为HTTP/1.0或HTTP/1.1或其它支持协议版本 (HTTP/1.1 505 HTTP Version Not Supported)
#define HTTP_CODE_505 505
//200 OK
#define HTTP_CODE_200 200

/* 用于Hash_Map键的串类型数据比较, 全等 */
typedef struct HashMapStrCompare{
    bool operator()(const char* ls, const char* rs) const{
        return strcmp(ls, rs)==0;
    }
}HashMapStrCompare;

/* 用于Hash_Map键的串类型数据比较, 忽略大小写(ignoring case) */
typedef struct HashMapStrIgnoreCaseCompare{
    bool operator()(const char* ls, const char* rs) const{
        return strcasecmp(ls, rs)==0;
    }
}HashMapStrIgnoreCaseCompare;


/* 指针类型Hash函数和比较器定义 */
//比较器
typedef struct PointerCompare{
    bool operator()(const void* ls, const void* rs) const{
        return ls == rs;
    }
}PointerCompare;    
//Hash函数
typedef struct PointerHash{
    size_t operator()(const void* h) const{
        __gnu_cxx::hash<unsigned long> H;
        return H((unsigned long)h);
    }
}PointerHash;     


//sockaddr_in结构长度
static socklen_t SOCKET_ADDR_LEN = sizeof(struct sockaddr_in);

//检查errno是否EAGAIN
#define ERRNO_IS_AGAIN() (errno == EAGAIN || errno == EINTR)


/** 定义进程消息结构 */
typedef struct ProcessMessage_t{
    /** 消息类型, 宽度(1) */
    MSG_TYPE type;
    /** 消息发送端的进程ID, 宽度(sizeof(pid_t))*/
    pid_t pid;
    /** 消息长度, 宽度(4) */
    int32_t length;
    /** 数据区, 数据区的内容长度为length定义的长度 */
    void* data;
    
}ProcessMessage_t;

#endif	/* _RESOURCES_H */

