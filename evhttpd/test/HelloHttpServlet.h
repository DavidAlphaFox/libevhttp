/* 
 * File:   HelloHttpServlet.h
 * Author: try
 *
 * Created on 2011年7月15日, 下午5:51
 */

#ifndef HELLOHTTPSERVLET_H
#define	HELLOHTTPSERVLET_H

#include "../HttpServlet.h"

using namespace std;

class HelloHttpServlet : public HttpServlet{
public:
    HelloHttpServlet(){}
    virtual ~HelloHttpServlet(){}
    
    virtual void service(Request& req, Response& resp){
        
        resp.setContentType("text/html; charset=utf-8");
        resp.write("Hello World!");
        
        
    }
 
};

#endif	/* HELLOHTTPSERVLET_H */

