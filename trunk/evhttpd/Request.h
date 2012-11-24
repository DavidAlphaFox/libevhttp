/* 
 * File:   Request.h
 * Author: try
 *
 * Created on 2011年6月2日, 下午11:18
 */

#ifndef _REQUEST_H
#define	_REQUEST_H

#include <vector>
#include "RequestData.h"
#include <netinet/in.h>


/**
 * HTTP请求信息
 */

class Request {
    friend class HttpHandler;
    friend class Response;
public:
    /**
     * @param requestData 具体请求信息
     */
    Request(RequestData& requestData);
    virtual ~Request();

    /** 返回头信息, 如果头不存在，返回NULL */
    const char* getHeader(const char* name);

    /** 在vector中返回头信息，一键多值 */
    std::vector<const char*>& getHeaders(const char* name, std::vector<const char*>& rsv);

    /** 返回所有请求头名称 */
    const std::vector<const char*>& getHeaderNames();

    /** 返回主体内容类型, 如果内容类型不存在，返回NULL */
    const char* getContentType();
    
    /** 返回内容长度 */
    ssize_t getContentLength();

    /** 返回请求参数, 如果参数不存在，返回NULL */
    const char* getParameter(const char* name);

    /** 在vector中返回请求参数信息，一键多值 */
    std::vector<const char*>& getParameterValues(const char* name, std::vector<const char*>& rsv);

    /** 在vector中返回请求参数名称 */
    const std::vector<const char*>& getParameterNames();

public:
    /* 客户端地址信息 */
    struct sockaddr_in& remoteAddr;
    
    /* 请求行相关 */
    const char* & method;     //请求方式
    const char* & path;       //资源路径
    const char* & parameter;  //请求参数信息
    const char* & protocol;   //协议及版本信息

    /* 请求方式 */
    bool isGet;  //是否GET方式
    bool isPost; //是否POST方式
    bool isHead; //是否HEAD方式
    bool isPut;  //是否PUT方式

    /* 协议版本 */
    bool isHttp1_0;  //是HTTP/1.0
    bool isHttp1_1;  //是HTTP/1.1

    /* 请求主体内容 */
//    const char* & body;
    
    /* 是否 Content-Type为 multipart/form-data类型 */
    bool isMultipartContentType;
    /* 仅当isMultipartContentType为true时，boundary值才有意义 */
    const char* boundary;
    
    /* Transfer-Encoding是否chunked */
    bool isChunkedTransferEncoding;

    /** 内容类型, 如果不存在，返回空指针NULL */
    const char* contentType;
    
    /* 内容长度, 默认为:-1 */
    ssize_t contentLength;
    
private:
    RequestData& requestData;

    std::vector<const char*> headerNames;

    std::vector<const char*> parameterNames;
    
private:
    
    void clean();
    
};

#endif	/* _REQUEST_H */

