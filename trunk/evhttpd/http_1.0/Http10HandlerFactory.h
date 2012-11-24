/* 
 * File:   Http10HandlerFactory.h
 * Author: try
 *
 * Created on 2011年6月2日, 上午9:37
 */

#ifndef _HTTP10HANDLERFACTORY_H
#define	_HTTP10HANDLERFACTORY_H

#include "../HttpHandlerFactory.h"

class Http10HandlerFactory : public HttpHandlerFactory{
public:
    Http10HandlerFactory();
    virtual ~Http10HandlerFactory();

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

#endif	/* _HTTP10HANDLERFACTORY_H */

