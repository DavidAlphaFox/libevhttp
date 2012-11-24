/* 
 * File:   Http11HandlerFactory.h
 * Author: try
 *
 * Created on 2011年6月3日, 上午11:31
 */

#ifndef _HTTP11HANDLERFACTORY_H
#define	_HTTP11HANDLERFACTORY_H

#include "../HttpHandlerFactory.h"

class Http11HandlerFactory  : public HttpHandlerFactory{
public:
    Http11HandlerFactory();
    virtual ~Http11HandlerFactory();

    /**
     * @see HttpHandlerFactory#create()
     */
    virtual HttpHandler* create();

    /**
     * @see HttpHandlerFactory#free(HttpHandler* handler)
     */
    virtual void free(HttpHandler* handler);    

private:

};

#endif	/* _HTTP11HANDLERFACTORY_H */

