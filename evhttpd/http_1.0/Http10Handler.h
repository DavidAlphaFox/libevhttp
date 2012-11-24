/* 
 * File:   Http10Handler.h
 * Author: try
 *
 * Created on 2011年6月2日, 上午9:39
 */

#ifndef _HTTP10HANDLER_H
#define	_HTTP10HANDLER_H

#include "../HttpHandler.h"

class Http10Handler: public HttpHandler{
public:
    Http10Handler();
    virtual ~Http10Handler();

protected:

//    /** 准备请求主体数据 */
//    virtual bool prepareRequestBody();

    /** 当请求行和消息头准备好后会进入此方法 */
    virtual bool handleRequest();

protected:    


//--事件-----------------------------------------------------------------------
protected:
    /** 在成功读入数据后调用此方法, 参数为本次读入的数据量
     * 当此事件已经被处理，返回true，如果返回false, 说明此事件还未被处理过 */
    virtual bool onReadEvent(unsigned int n);

    /** 当请求行准备好后，发送此事件
     * 当此事件已经被处理，返回true，如果返回false, 说明此事件还未被处理过 */
    virtual bool onRequestLineEvent();

    /** 当消息头准备好后，发送此事件
     * 当此事件已经被处理，返回true，如果返回false, 说明此事件还未被处理过 */
    virtual bool onRequestHeadersEvent();

    /** 当HttpCode被设置时产生此事件, 即httpCode!=0时 */
    virtual bool onHttpCodeEvent();

    /** 当请求处理完成后产生此事件，在HttpHandler处理完请求后需要需要显示的调用此方法 */
    virtual bool onCompleteEvent();
    
private:
    /** 是否 Content-Type为 multipart/form-data类型, 如果是，同时设置request.boundary */
    bool isMultipartContentType();
    
    /** Transfer-Encoding是否chunked */
    bool isChunkedTransferEncoding();    
    
    /** 返回主体内容长度 */
    ssize_t getContentLength();
};

#endif	/* _HTTP10HANDLER_H */

