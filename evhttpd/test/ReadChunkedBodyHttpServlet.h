/* 
 * File:   ReadChunkedBodyHttpServlet.h
 * Author: try
 *
 * Created on 2011年7月17日, 上午12:27
 */

#ifndef READCHUNKEDBODYHTTPSERVLET_H
#define	READCHUNKEDBODYHTTPSERVLET_H

#include "../HttpServlet.h"
#include "../ChunkedBodyBuilder.h"

class ReadChunkedBodyHttpServlet : public HttpServlet, public ChunkedBodyBuilder{
public:
    ReadChunkedBodyHttpServlet():readBodyDone(false), bodyError(false){}
    virtual ~ReadChunkedBodyHttpServlet(){}
    
    
    virtual bool onReadBodyEvent(const char* buff, size_t n){
        if(request->isChunkedTransferEncoding){
            build(buff, n);
            if(bodyError){
                response->setStatus(400, "body data error");
                return true;
            }else if(readBodyDone){
                return true;
            }
        }else{
            response->setStatus(400, "Content-Type not is ChunkedTransferEncoding");
            return true;            
        }
        return false;
    }
    
    
    virtual void service(Request& req, Response& resp){
        
    }
    
    
protected:
    virtual bool onBuildCompletionEvent(const BodyBuildCompletionEvent* e){
        if(e->length > 0){
            //在此处理读到的数据
            
            
        }else if(e->length == 0){
            readBodyDone = true;
            return true;
        }else{
            bodyError = true;
            return true;
        }
        return false;
    }
    
private:
    bool readBodyDone;
    bool bodyError;
};

#endif	/* READCHUNKEDBODYHTTPSERVLET_H */

