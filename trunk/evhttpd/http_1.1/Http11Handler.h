/* 
 * File:   Http11Handler.h
 * Author: try
 *
 * Created on 2011年6月3日, 上午11:32
 */

#ifndef _HTTP11HANDLER_H
#define	_HTTP11HANDLER_H

#include "../http_1.0/Http10Handler.h"

class Http11Handler : public Http10Handler{
public:
    Http11Handler();
    virtual ~Http11Handler();

    /** 当请求行和消息头准备好后会进入此方法 */
    virtual bool handleRequest();    
    
//--事件-----------------------------------------------------------------------
protected:

    /** 当请求行准备好后，发送此事件
     * 当此事件已经被处理，返回true，如果返回false, 说明此事件还未被处理过 */
     virtual bool onRequestLineEvent();

    /** 当请求处理完成后产生此事件，在HttpHandler处理完请求后需要需要显示的调用此方法 */
    virtual bool onCompleteEvent();
    
private:

};

#endif	/* _HTTP11HANDLER_H */

