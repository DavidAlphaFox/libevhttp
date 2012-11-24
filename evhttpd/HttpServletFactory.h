/* 
 * File:   HttpServletFactory.h
 * Author: try
 *
 * Created on 2011年6月3日, 上午8:43
 */

#ifndef _HTTPSERVLETFACTORY_H
#define	_HTTPSERVLETFACTORY_H
#include "resources.h"
#include "Request.h"
#include "HttpServlet.h"

/**
 * HttpServlet工厂
 */
class HttpServletFactory {
public:
    HttpServletFactory(){}
    virtual ~HttpServletFactory(){}

    /** 
     * 创建HttpServlet 
     * 
     * @param path http请求路径
     * 
     */
    virtual HttpServlet* create(const char* path){
        return &defaultHttpServlet;
    }

    /** 
     * 释放HttpServlet 
     */
    virtual void free(HttpServlet* servlet){
        
    }

private:
    HttpServlet defaultHttpServlet;
};

#endif	/* _HTTPSERVLETFACTORY_H */

