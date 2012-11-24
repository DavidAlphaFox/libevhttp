/* 
 * File:   HttpHandlerFactory.h
 * Author: try
 *
 * Created on 2011年6月2日, 上午12:01
 */

#ifndef _HTTPHANDLERFACTORY_H
#define	_HTTPHANDLERFACTORY_H
#include "resources.h"

/**
 * HttpHandler工厂
 */
class HttpHandler;
class HttpHandlerFactory {
public:
    HttpHandlerFactory();
    virtual ~HttpHandlerFactory();

    /** 创建HttpHandler */
    virtual HttpHandler* create();

    /** 释放HttpHandler */
    virtual void free(HttpHandler* handler);

private:

};

#endif	/* _HTTPHANDLERFACTORY_H */

