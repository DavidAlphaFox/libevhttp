/* 
 * File:   ReadFormBodyHttpServlet.h
 * Author: try
 *
 * Created on 2011年7月16日, 下午3:38
 */

#ifndef READFORMBODYHTTPSERVLET_H
#define	READFORMBODYHTTPSERVLET_H

#include <vector>
#include "../HttpServlet.h"
#include "../HttpUtils.h"

using namespace std;

class ReadFormBodyHttpServlet : public HttpServlet{
    
private:
    char* body; //form表单数据
    size_t areadBytes; //已读字节数
    KeyValues<> bodyParams; //存放分离后的form表单数据    
    
public:
    ReadFormBodyHttpServlet():body(NULL), areadBytes(0){}
    virtual ~ReadFormBodyHttpServlet(){
        bodyParams.clear();
        if(body){
            delete[] body;
        }
    }
    
    
    virtual bool onReadBodyEvent(const char* buff, size_t n){
        if(request->contentType && strcmp(request->contentType, "application/x-www-form-urlencoded")==0){
            //是form表单
            if(!body){
                body = new char[request->contentLength + 1];
            }
            if(HttpUtils::readBody(body, &areadBytes, request->contentLength, buff, n)){
                //body数据读完了
                //分析值对格式表单数据, 将值对存入bodyParams中
                HttpUtils::parseParameters(body, &bodyParams);
                return true;
            }
        }else{
            //其它类型
            response->setStatus(400, "Content-Type not is application/x-www-form-urlencoded");
            return true;
        }
   
        return false;
    }    
    
    virtual void service(Request& req, Response& resp){
        resp.setContentType("text/html; charset=utf-8");
        
        resp.write("<br>[all request parameters data]<br>");
        
        const vector<const char*>& pnames = req.getParameterNames();
        for(vector<const char*>::const_iterator it = pnames.begin(); it != pnames.end(); it++){
            resp.write(*it);
            resp.write("=");
            vector<const char*> pvalues;
            req.getParameterValues(*it, pvalues);
            for(vector<const char*>::iterator vit = pvalues.begin(); vit != pvalues.end(); vit++){
                resp.write(value(*vit));
                resp.write(",");
            }
            resp.write("<br>");
        }
        
        resp.write("<br>[all request headers]<br>");
        const vector<const char*>& hnames =  req.getHeaderNames();
        for(int i=0; i<hnames.size(); i++){
            vector<const char*> vals;
            req.getHeaders(hnames[i], vals);
            for(int j=0; j<vals.size(); j++){
                //cout<<hnames[i]<<"="<<value(vals[j])<<endl;
                resp.write(hnames[i]);
                resp.write("=");
                resp.write(value(vals[j]));
                resp.write("<br>");
            }
        }        
        
        resp.write("<br>[form data]<br>");
        vector<const char*> bnames;
        bodyParams.getNames(bnames);
        for(vector<const char*>::const_iterator it = bnames.begin(); it != bnames.end(); it++){
            resp.write(*it);
            resp.write("=");
            vector<const char*> pvalues;
            bodyParams.gets(*it, pvalues);
            for(vector<const char*>::iterator vit = pvalues.begin(); vit != pvalues.end(); vit++){
                resp.write(value(*vit));
                resp.write(",");
            }
            resp.write("<br>");
        }
    }
    
    
private:
    const char* value(const char* v){
        return v?v:"null";
    }
};

#endif	/* READFORMBODYHTTPSERVLET_H */

