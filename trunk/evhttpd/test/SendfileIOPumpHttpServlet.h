/* 
 * File:   SendfileIOPumpHttpServlet.h
 * Author: try
 *
 * Created on 2011年7月15日, 下午11:47
 */

#ifndef SENDFILEIOPUMPHTTPSERVLET_H
#define	SENDFILEIOPUMPHTTPSERVLET_H

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>

#include "../resources.h"
#include "../IIOPump.h"
#include "../SendfileIOPump.h"
#include "../HttpServlet.h"
#include "../events.h"
#include "../event/IEventListener.h"


#define SEND_SIZE 1024*512


/**
 * 使用SendfileIOPump来下载文件实例，用法
 * 
 * http://192.168.99.60:3080/sf/home/vdev/flvs/1.flv?speed=1048576&offset=0&length=0
 */
class SendfileIOPumpHttpServlet : public HttpServlet, public IEventListener{
public:
    SendfileIOPumpHttpServlet():flvfd(-1), ioPump(NULL){}
    virtual ~SendfileIOPumpHttpServlet(){
        close();
        if(ioPump){
            delete ioPump;
            ioPump = NULL;
        }
    }
    
    virtual void service(Request& req, Response& resp){
        
        //下载速度, 每秒下载速度
        const char* speed = req.getParameter("speed");
        int speedval = 0;
        if(speed){
            speedval  = atoi(speed);
        }else{
            speedval = 1024*1024; //默认速度为1M字节
        }    
        
        //下载位置
        const char* offset = req.getParameter("offset");
        off_t offsetval = 0;
        if(offset){
            offsetval  = atol(offset);
        }else{
            offsetval = 0;
        }  
        
        //下载数据量
        const char* length = req.getParameter("length");
        off_t lengthval = 0;
        if(length){
            lengthval  = atol(length);
        }else{
            lengthval = 0;
        }          
        
        //取文件路径，去掉URL根位置部分
        const char* flvfile = req.path+3; //去掉/sf部份前缀
        if(!flvfile){
            resp.setStatus(400, "error");
            return;
        }
        
        //取文件信息
        struct stat filestat;
        if(stat(flvfile, &filestat)!=0){
            LOG_INFO("file not exist! errno:%d", errno);
            resp.setStatus(400, "error");
            return;
        }
        if(S_ISDIR(filestat.st_mode)){
            LOG_INFO("is dir");
            resp.setStatus(400, "error, is dir");
            return;
        }
        
        //打开文件
        flvfd = ::open(flvfile, O_RDONLY | O_NONBLOCK);
        if(flvfd < 0){
            resp.setStatus(400, "open file error");
            return;
        }

        ioPump = new SendfileIOPump(httpHandler->getClientFD(), flvfd, speedval, SEND_SIZE);
        ioPump->setOffset(offsetval);
        ioPump->setLength(lengthval);
        configIOPump();
        
        httpHandler->setIdleTimeout(false);
        resp.setContentType("video/x-flv");
        //计算并设置下载的内容数据量
        size_t efflen = filestat.st_size - offsetval;
        if(lengthval <= 0){
            resp.setContentLength(efflen);
        }else{
            if(lengthval < efflen){
                resp.setContentLength(lengthval);
            }else{
                resp.setContentLength(efflen);
            }
        }
        resp.flushBuffer();//首先要输出响应头
        resp.setAsynAnswerMode(true); //设置异步响应模式
        return;        
    }    
    
    virtual bool onWriteQueueEmptyEvent(){
        //LOG_INFO("response->headerSended():%d, ioPump->isReady():%d", response->headerSended(), ioPump->isReady());
        if(response->headerSended() && ioPump && ioPump->isReady()){
            //当消息头成功发送后开始发送主体数据
            ioPump->start();
        }
        return true;
    }
    
    virtual bool onCompletionEvent(){
        close();
        return true;
    }      
    
    /**
     * 处理数据传输事件
     */
    virtual bool handle(const Event* e){
        if(e->getType() == EVENT_TYPE_IO_TRANSPORT){
            const IOTransportEvent* te = (const IOTransportEvent*)e;
            if(te->readEvent->length <=0){ //读出错或读结束
                complete();
            }else  if(te->writeEvent){//写事件有可能为NULL
                if(te->writeEvent->length < 0){//写错误
                    complete();
                }
            }
            return true;
        }
        return false;
    }    
    
private:
    void configIOPump(){
        ioPump->addListener(EVENT_TYPE_IO_TRANSPORT, this);
    }    
    
    void complete(){
        close();
        response->complete(); //通知完成
    }    
    
    void close(){
        if(ioPump){
            ioPump->close();
        }
        if(flvfd > 0){
          ::close(flvfd);
          flvfd = -1;
        }
    }    
    
private:
    FD flvfd;
    IIOPump* ioPump;
};

#endif	/* SENDFILEIOPUMPHTTPSERVLET_H */

