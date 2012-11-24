/* 
 * File:   main.cpp
 * Author: try
 *
 * Created on 2011年5月26日, 上午10:02
 */

#include <netinet/tcp.h>
#include <errno.h>
#include "libev.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/file.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <vector>
#include <arpa/inet.h>
#include <ext/hash_map>
#include <time.h>
#include "HttpServlet.h"

#include "http_1.0/Http10HandlerFactory.h"
#include "http_1.1/Http11HandlerFactory.h"


#include "HttpProcess.h"
#include "HttpServer.h"
#include "Config.h"
#include "SocketUtils.h"
#include "CleanerTimer.h"


//#include "nvwa/debug_new.h"

using namespace std;

#define SERVER_PORT 3080

double getCurrentTime();

ev::timer* evtimerPt = NULL;
HttpServer* httpServerPt = NULL;
HttpServer* monitorServerPt = NULL;
HttpProcess* monitorProcess = NULL;
Config config;

unsigned long lastTotalRequestCount = 0;
double reqCountPerSecondPeak = 0.0; //峰值
double lastTime = getCurrentTime(); //单：秒

const char* NULL_VALUE = "null";
//如果v为NULL, 返回"null"
const char* value(const char* v){
    return v?v:NULL_VALUE;
}

double getCurrentTime(){
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return tv.tv_sec+(double)tv.tv_usec/1000000.0;
}


static void signalHandler(int sno) {
    LOG_ERROR("pid is %d, signal is %d", getpid(), sno);
    CleanerTimer::instance.cleanupAll();
    exit(sno);
}

void setSignalHandler(){
//    signal(SIGPIPE, SIG_IGN);
    
    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGINT, signalHandler);
    //signal(SIGHUP, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGSTOP, signalHandler);
    signal(SIGTSTP, signalHandler);
    /* 处理kill信号 */
    signal(SIGKILL, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGHUP, signalHandler);
    /* 处理定时更新修改的数据到磁盘信号 */
    signal(SIGALRM, signalHandler);
    
    

}

#include "HttpUtils.h"
class ExampleHttpServlet : public HttpServlet{
private:
    char* body;
    size_t areadBytes; //已读字节数量
    KeyValues<> bodyParams; //form表单数据
public:
    ExampleHttpServlet():body(NULL), areadBytes(0){
    }
    virtual ~ExampleHttpServlet(){
//        LOG_INFO("")
        bodyParams.clear();
        if(body){
            MemoryPool::instance.free(body);
            body = NULL;
        }
    }

    /** 读Body数据时事件，buff中为刚读出的数据，n为数量, 如果你认为Body数据已经读完了，就返回true吧 */
    virtual bool onReadBodyEvent(const char* buff, size_t n){
        const char* ctttype = request->getContentType();
        ssize_t cttlen = request->getContentLength();
        LOG_INFO("request->getContentType():%s", request->getContentType()?request->getContentType():"null");
        LOG_INFO("request->getContentLength():%d", request->getContentLength());
        
        if(ctttype && strcmp(ctttype, "application/x-www-form-urlencoded")==0){
            //是form表单
            if(!body){
                body = (char*)MemoryPool::instance.malloc(request->getContentLength()+1);
                body[request->getContentLength()] = 0;
            }
            if(HttpUtils::readBody(body, &areadBytes, cttlen, buff, n)){
                //body数据读完了
                LOG_INFO("read body done, body is:%s", body);
                //分析值对格式表单数据
                HttpUtils::parseParameters(body, &bodyParams);
                return true;
            }
        }else{
            //其它类型
            
            return true;
        }
   
        return false;
    }
    
    virtual void service(Request& req, Response& resp){
        resp.setContentType("text/html; charset=utf-8");

        resp.write("[all request headers]<br>");
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
        
        resp.flushBuffer();

        resp.addHeader("Server", "evhttp");

        resp.addHeader("Test", "t1");
        resp.addHeader("Test", "t2");
        resp.addHeader("Test", "t3");

        resp.addHeader("T1", "t11");
        resp.addHeader("T1", "t12");
        resp.setHeader("T1", "t13");
        
        resp.setHeader("H1", "h1\n h2\r\n h3\n h4\n\th5\r\n\th6");


        resp.write("[all request query parameters]<br>");
        const vector<const char*> pnames = req.getParameterNames();
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
        resp.flushBuffer();
        
        resp.write("[all request body parameters]<br>");
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
        resp.flushBuffer();
        
        resp.write("<br>[remote addr]<br>");
        const char* ip = inet_ntoa(req.remoteAddr.sin_addr);
        resp.write("ip=");
        resp.write(ip);
        int port = ntohs(req.remoteAddr.sin_port);
        resp.write("<br>port=");
        resp.write(port);
        
        resp.flushBuffer();
        
        const char* exitVal = req.getParameter("exit");
        if(exitVal && strcmp(exitVal, "yes")==0){
            exit(1);
        }
    }



};

    
class EmptyHttpServlet : public HttpServlet{
public:
    EmptyHttpServlet(){};
    virtual ~EmptyHttpServlet(){};
    
    virtual void service(Request& req, Response& resp){
        resp.setContentType("text/html; charset=utf-8");
        resp.write("Hello World!<br><br>");
        httpHandler->setIdleTimeout(true);
    }
};



#include "EvNIOWriter.h"
class UploadNIOWriter : public EvNIOWriter{
public:
    UploadNIOWriter():planWriteBytes(0), writeCompleteBytes(0), uploadFD(-1){
        cout<<"planWriteBytes:"<<planWriteBytes<<endl;
        
    }
    
    virtual ~UploadNIOWriter(){
        if(uploadFD != -1){
            close();
            ::close(uploadFD);
        }
    }    
    
    void createTmpFile(){
        if(uploadFD != -1){
//            ::close(uploadFD);
            close();
            ::close(uploadFD);
        }
        uploadFD = ::open("upload.tmp", O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
        cout<<"uploadFD:"<<uploadFD<<endl;
        setFD(uploadFD);
    }
    
    void setPlanWriteBytes(size_t n){
        planWriteBytes = n;
    }
    
protected:
    
    /**
     * 当write完成向fd写入数据后调用此方法 
     * @param wsize 实际写入的数据量, 如果写入错误返回-1, errno中返回错误码
     * @return 如果事件被处理，返回true, 否则，返回false
     */
    virtual bool onWriteCompletionEvent(const IOWriteCompletionEvent* e){
        if(e->length > 0){
            writeCompleteBytes += e->length;
        }
    }
    
//    virtual bool onWriteCompletionEvent(const char* buff, ssize_t rsize, off_t offset){
//        if(rsize > 0){
//            writeCompleteBytes += rsize;
//        }
//    }    
    
//    virtual bool onWriteEvent(ssize_t wsize){
////        cout<<"onWriteEvent.wsize:"<<wsize<<endl;
//        if(planWriteBytes >0 && writeCompleteBytes >= planWriteBytes){
//            //说明写数据完成
////            cout<<"onWriteEvent.write.complete.writeCompleteBytes():"<<writeCompleteBytes()<<", completeCount():"<<writeCompleteCount()<<", writeCount():"<<writeCount()<<endl;
//        }else if(wsize < 0){
//            //写错误
//            
//        }
//
//        return true;
//    }    
public:
    FD uploadFD; //上传的临时文件FD
    size_t planWriteBytes; //计划写入字节数量，默认为:0
    size_t writeCompleteBytes; //累计写入字节数量
};

UploadNIOWriter* uploadIOWriter = NULL; //仅测试用，实际应用不能这样整

class UploadHttpServlet : public HttpServlet{
public:
    UploadHttpServlet():uploadSize(0){
        if(!uploadIOWriter){
            uploadIOWriter = new UploadNIOWriter();
        }
        uploadIOWriter->createTmpFile();
        
    }
    virtual ~UploadHttpServlet(){
//        delete ioWriter;
    }
    
    virtual bool onReadBodyEvent(const char* buff, size_t n){
        
//        LOG_INFO("request->getContentType():%s", request->getContentType()?request->getContentType():"null");
//        LOG_INFO("request->getContentLength():%d", request->getContentLength());        
//        LOG_INFO("request->isMultipartContentType:%d", request->isMultipartContentType);   
        if(request->isMultipartContentType){
//            LOG_INFO("request.boundary:%s ", request->boundary);
            
            int cttlen = request->getContentLength();
            uploadSize += n;
            if(uploadSize >= cttlen){
                uploadIOWriter->setPlanWriteBytes(cttlen);
            } 
            
            if(!uploadIOWriter->write(buff, n)){
//                cout<<"error 1"<<endl;
                response->setStatus(400, "write error");
                return true;
            }
            
            if(uploadSize >= cttlen){
//                cout<<"complete, uploadSize:"<<uploadSize<<", req.contentLength:"<<cttlen<<endl;
                //上传完成
                
                return true;
            }
            
        }else{
            //错误
//            cout<<"UploadHttpServlet.onReadBodyEvent.error 2"<<endl;
            response->setStatus(400, "Content-Type not is multipart/form-data; ");
            return true;
        }
        
        
        //req.completeReadBody();
        
        return false;
    }    
    
    virtual void service(Request& req, Response& resp){
//        LOG_INFO("request->isMultipartContentType:%d", request->isMultipartContentType);   
        resp.setContentType("text/html; charset=utf-8");
        resp.write("uploading ...!<br><br>");
        resp.flushBuffer();
    }
    

    
private:
    FD fd;
    size_t uploadSize; //已经上传的内容大小
    
};

#define BUFF_SIZE 1024*1024

//计算根据速度speed，计算处理时间间隔(timeoutInterval)和每次处理字节数量(readBytesPerEach)
void calculateSpeed(double speed, double* timeoutInterval, int* readBytesPerEach){
        if(speed > 0){
            //计算每次需要读出的字节数量和读时间间隔
            *readBytesPerEach = (int)(speed + ((BUFF_SIZE/speed)/3)*speed);
            if(*readBytesPerEach > BUFF_SIZE){
                *readBytesPerEach = BUFF_SIZE;
            }
            *timeoutInterval = *readBytesPerEach/speed;
            if(*timeoutInterval > 5){ //如果时间间隔大于5秒,就重新计算 
                *timeoutInterval = 5;
                *readBytesPerEach = (int)(*timeoutInterval*speed);
                *readBytesPerEach = *readBytesPerEach > BUFF_SIZE?BUFF_SIZE:*readBytesPerEach;
            }
        }else{
            //无限制
            *timeoutInterval = 0.0;
            *readBytesPerEach = BUFF_SIZE;
        }
        LOG_DEBUG("calculateSpeed, speed:%f(/sec), timeoutInterval:%f, readBytesPerEach:%d", speed, *timeoutInterval, *readBytesPerEach);
}


#include "IIOReader.h"
#include "IIOWriter.h"
#include "EvNIOReader.h"
#include "event/IEventListener.h"
#include "event/EventDispatcher.h"
#include "RWIOPump.h"
#include "SendfileIOPump.h"

/** 通过IOPump来传输数据 */
class IOPumpHttpServlet : public HttpServlet, public IEventListener{
public:
    IOPumpHttpServlet():flvfd(-1), ioPump(NULL), ioReader(NULL), ioWriter(NULL){

    }
    
    virtual ~IOPumpHttpServlet(){
        close();
        if(ioPump){
            delete ioPump;
            ioPump = NULL;
        }
        if(ioReader){
            delete ioReader;
            ioReader = NULL;
        }
        if(ioWriter){
            delete ioWriter;
            ioWriter = NULL;
        }
    }
    
    virtual bool onWriteQueueEmptyEvent(){
        LOG_DEBUG("response->waitingWriteQueueSize():%d", response->waitingWriteQueueSize());
        if(response->headerSended() && ioPump->isReady()){
            ioPump->start();
        }
        return true;
    }   
    
    
    virtual void service(Request& req, Response& resp){
       LOG_DEBUG("");
        resp.setContentType("video/x-flv");
        resp.addDateHeader("Date", time(NULL));
        
        //设置速率, 每秒下载速度
        const char* speed = req.getParameter("speed");
        int speedval = 0;
        if(speed){
            speedval  = atoi(speed);
        }else{
            speedval = 1024*1024; //默认速度为1M字节
        }
        
        const char* flvfile = req.path+2; //去掉pipe部份前缀
        LOG_DEBUG("flvfile:%s", flvfile);
        if(!flvfile){
            resp.setStatus(400, "error");
            return;
        }

        struct stat filestat;
        if(stat(flvfile, &filestat)!=0){
            LOG_DEBUG("stat.error, flvfile is %s", flvfile)
            resp.setStatus(400, "error");
            return;
        }
        
        flvfd = ::open(flvfile, O_RDONLY | O_NONBLOCK);
        LOG_DEBUG("flvfile:%d, errno:%d", flvfd, errno);
        if(flvfd > 0){
            const char* engine = req.getParameter("engine");
            if(!engine){
                engine = "SF";
            }
            LOG_DEBUG("engine:%s", engine);
            if(strcmp(engine, "SF")==0){
                ioPump = new SendfileIOPump(httpHandler->getClientFD(), flvfd, speedval, BUFF_SIZE);
            }else{
                if(strcmp(engine, "NIO")==0){
                    ioReader = new EvNIOReader(flvfd);
//                }else if(strcmp(engine, "AIO")==0){
//                    ioReader = new AIOReader(flvfd);
                }else{
                    resp.setStatus(400, "error engine, engine in (NIO,AIO)");
                    return;
                }
                ioWriter = new EvNIOWriter(httpHandler->getClientFD());
                ioPump = new RWIOPump(ioReader, ioWriter, speedval, BUFF_SIZE);
            }
            configIORWPipeline();
            
            httpHandler->setIdleTimeout(false);
            resp.setContentLength(filestat.st_size);
            resp.flushBuffer();//刷新响应头
            resp.setAsynAnswerMode(true); //设置异步响应模式
            return;
        }else{
            resp.setStatus(400, "flv file not find");
            return;
        }
    }
    
    virtual bool onCompletionEvent(){
        LOG_DEBUG("");
        close();
        return true;
    }      
    
    void close(){
        LOG_DEBUG("");
        if(ioPump){
            ioPump->close();
        }
        if(ioReader){
            ioReader->close();
        }
        if(ioWriter){
            ioWriter->close();
        }
        if(flvfd > 0){
          ::close(flvfd);
          flvfd = -1;
        }
        
    }
    
    void complete(){
        LOG_DEBUG("");
        close();
        response->complete();
    }
    
    /**
    * 处理读写事件
    */
    virtual bool handle(const Event* e){
        LOG_DEBUG("e->getType():%d", e->getType());
        return handleTransportEvent((const IOTransportEvent*)e);
    }    
    
private:
    void configIORWPipeline(){
        ioPump->addListener(EVENT_TYPE_IO_TRANSPORT, this);
    }
    
    bool handleTransportEvent(const IOTransportEvent* e){
        if(e->readEvent->length <=0){ //读出错或读结束
            LOG_DEBUG("2 read error or complete e->readEvent->length:%d", e->readEvent->length);
            complete();
        }else  if(e->writeEvent){
            if(e->writeEvent->length < 0){//写错误
                LOG_DEBUG("write error, e->readEvent->length:%d", e->writeEvent->length);
                complete();
            }
        }
        return true;
    }
    
private:
    FD flvfd;
    IIOPump* ioPump;
    IIOReader* ioReader;
    IIOWriter* ioWriter;
};

//dwn
class DownloadHttpServlet : public HttpServlet, public IEventListener{
public:
    DownloadHttpServlet():iIOReader(NULL),flvfd(-1){
        readfiletimer.set<DownloadHttpServlet, &DownloadHttpServlet::timerCallback> (this);
        LOG_DEBUG("");
    }
    
    virtual ~DownloadHttpServlet(){
        closeFlvfile();
        if(iIOReader)delete iIOReader;
        LOG_DEBUG("");
    }
    
    virtual void service(Request& req, Response& resp){
        LOG_DEBUG("");
        resp.setContentType("video/x-flv");
        resp.addDateHeader("Date", time(NULL));
        
        //设置速率, 每秒下载速度
        const char* speed = req.getParameter("speed");
        double speedval = 0;
        if(speed){
            LOG_DEBUG("speed:[%s]", speed);
            speedval  = atof(speed);
            LOG_DEBUG("speedval:[%d]", speedval);
        }else{
            speedval = 1024*1024;
        }
        calculateSpeed(speedval, &timeoutInterval, &readBytesPerEach);
                
        const char* flvfile = req.path+4; //去掉dwn部份前缀
        LOG_DEBUG("flvfile:%s", flvfile);
        if(!flvfile){
            resp.setStatus(400, "error");
            return;
        }

        flvfd = ::open(flvfile, O_RDONLY | O_NONBLOCK);
        LOG_DEBUG("flvfile:%d, errno:%d", flvfd, errno);
        if(flvfd > 0){
            const char* engine = req.getParameter("engine");
            
            if(!engine || strcmp(engine, "NIO")==0){
                iIOReader = new EvNIOReader(flvfd);
//            }else if(strcmp(engine, "AIO")==0){
//                iIOReader = new AIOReader(flvfd);
            }else{
                resp.setStatus(400, "error engine, engine in (NIO,AIO)");
                return;
            }
            configIOReader(iIOReader);
            readData();
            resp.setAsynAnswerMode(true);
            return;
        }else{
            resp.setStatus(400, "flv file not find");
            return;
        }
    }
    

    
    virtual bool onWriteQueueEmptyEvent(){
        if(iIOReader && !iIOReader->isClosed()){
            if(iIOReader->waitingQueueSize() < 1){
                    startReadfileTimer();
            }
        }
        return true;
    } 
    
    
    virtual bool onCompletionEvent(){
        LOG_DEBUG("");
        closeFlvfile();
        return true;
    }    
    
    
    void configIOReader(IIOReader* iIOReader){
        //向IOReader中添加事件监听器
        iIOReader->addListener(EVENT_TYPE_IO_READ_COMPLETION, this);
    }
    /**
    * 处理读事件
    */
    virtual bool handle(const Event* e){
        const IOReadCompletionEvent* rce = (const IOReadCompletionEvent*)e;
        LOG_DEBUG("read size is (%d), offset is (%ld)", rce->length, rce->offset);
        return onReadCompletionEvent(rce->dataBuff, rce->length, rce->offset);
    }
    
protected:

    virtual bool onReadCompletionEvent(const char* buff, ssize_t rsize, long offset){
//        LOG_DEBUG("FlvAIOHttpServlet.onReadCompletionEvent.rsize:%ld, totalCount:%ld, completeBytes:%ld, completeCount:%ld", rsize, totalCount(), completeBytes(), completeCount());
        LOG_DEBUG("rsize:%ld", rsize);
        if(rsize > 0){
            if(!response->write(buff, rsize)){
                response->setStatus(400, "send flv error");
                complete();
            }
            response->flushBuffer();
            
        }else if(rsize < 0){
            response->setStatus(400, "read flv error");
            LOG_WARN("error, rsize:%d", rsize);
            complete();
        }else{
            complete();
        }
        return true;
    }    
    
    
private:
    void closeFlvfile(){
        if(iIOReader)iIOReader->close();
        if(readfiletimer.is_active()){
            readfiletimer.stop();
        }
        if(flvfd > 0){
          ::close(flvfd);
          flvfd = -1;
        }
    }    
    
    void complete(){
//        closeFlvfile();
        //向Hander发送消息
//        response->complete();
//        response->notifyComplete();
        response->complete();
    }    
    
    void startReadfileTimer(){
//        if(!readfiletimer.is_active()){
            readfiletimer.start(timeoutInterval, 0);
//        }
    }    
    
    void readData(){
        if(!iIOReader){
            LOG_WARN("iIOReader is NULL");
            return;  
        }
        if(iIOReader->isClosed()){
            LOG_WARN("iIOReader is Closed");
            return;
        }
        if(iIOReader->waitingQueueSize() > 0){
            LOG_WARN("waitingQueueSize():%d", iIOReader->waitingQueueSize()); //还有读数据请求没有返回，还是再等等
            return;
        }
        iIOReader->read(buff, readBytesPerEach);
    }    
    
    void timerCallback(ev::timer &ev, int revents){
        readData();
    }
    
private:
    IIOReader* iIOReader;
    FD flvfd;
    char buff[BUFF_SIZE];
    ev::timer readfiletimer; //读文件定时器，用来控制读文件速率
    double timeoutInterval; //单位：秒
    int readBytesPerEach; //每次读字节数    
    
};


#include <sys/sendfile.h>
class SendfileHttpServlet : public HttpServlet{
public:
    SendfileHttpServlet():flvfd(-1), offset(0), firstSend(true), sendBytesPerEach(0){
        sendfiletimer.set<SendfileHttpServlet, &SendfileHttpServlet::timerCallback> (this);
    };
    virtual ~SendfileHttpServlet(){};
    
    virtual void service(Request& req, Response& resp){
        LOG_DEBUG("flvfd is %d", flvfd);
        resp.setContentType("video/x-flv");

        //设置速率, 每秒下载速度
        const char* speed = req.getParameter("speed");
        double speedval = 0;
        if(speed){
            speedval  = atof(speed);
        }else{
            speedval = 1024*1024;
        }
        calculateSpeed(speedval, &timeoutInterval, &readBytesPerEach);
        sendBytesPerEach = readBytesPerEach;

        const char* flvfile = req.path+3; //去掉sf部份前缀
        LOG_DEBUG("flvfile:%s", flvfile);
        if(!flvfile){
            resp.setStatus(400, "error");
            return;
        }
        
        struct stat filestat;
        if(stat(flvfile, &filestat)!=0){
            LOG_DEBUG("stat.error, flvfile is %s", flvfile)
        }
        flvfd = ::open(flvfile, O_RDONLY | O_NONBLOCK);
//        flvfd = ::open(flvfile, O_RDONLY);
        if(flvfd > 0){
            LOG_DEBUG("flvfd:%d flvfile.size:%ld", flvfd, filestat.st_size);
            offset = 0;
            firstSend = true;
            httpHandler->setIdleTimeout(false);
            filesize = filestat.st_size;
            resp.setContentLength(filesize);
            bool rs = resp.flushBuffer();
            LOG_DEBUG("flvfd:%d, rs:%d", flvfd, rs);
            startSendfileTimer();
            resp.setAsynAnswerMode(true);
            return;
        }else{
            resp.setStatus(400, "error");
            return;
        }
    }
    
    
    virtual bool onWriteQueueEmptyEvent(){
        if(flvfd < 0)return true;
        startSendfileTimer();
        return true;
    }     
    
    virtual bool onCompletionEvent(){
        LOG_DEBUG("flvfd:%d", flvfd);
        closeFlvfile();
        return true;
    }        
    
private:
    FD flvfd;    //文件FD
    unsigned long filesize;
    off_t offset; //sendfile的offset参数
    
    ev::timer sendfiletimer; //读文件定时器，用来控制读文件速率
    double timeoutInterval; //单位：秒
    int readBytesPerEach; //每次读字节数（预计）   
    int sendBytesPerEach; //最终每次发送字节数量，等于：readBytesPerEach+（sendBytesPerEach - 上一次发送的字节数）
    bool firstSend; //是否第一次发送
    
private:
    void startSendfileTimer(){
        if(!sendfiletimer.is_active()){
            if(firstSend){
                sendfiletimer.start(0, 0); //第一次发送就不延时
                firstSend = false;
            }else{
                sendfiletimer.start(timeoutInterval, 0);
            }
        }
    }        
    
    void closeFlvfile(){
        if(sendfiletimer.is_active()){
            sendfiletimer.stop();
        }
        if(flvfd > 0){
          ::close(flvfd);
          flvfd = -1;
       }
    }    
    
    void complete(){
//        closeFlvfile();
        //向Hander发送消息
        response->complete();
    }      
    
    void timerCallback(ev::timer &ev, int revents){
        LOG_DEBUG("");
        sendData();
    }    
    
    void sendData(){
        ssize_t sendsize = ::sendfile(httpHandler->getClientFD(), flvfd, &offset, sendBytesPerEach);
        LOG_DEBUG("flvfd:%d, plan sendBytesPerEach:%d, sendfile.size:%ld, offset:%ld, errno:%d", flvfd, sendBytesPerEach, sendsize, offset, errno);
        if(sendsize == 0){
            LOG_DEBUG("complete, flvfd:%d, sendsize:%ld", flvfd, sendsize);
            complete();//完成
        }else if(sendsize<0 && errno != EAGAIN){
            LOG_WARN("sendfile.error, sendsize:%ld, errno:%d", sendsize, errno);
            complete();
        }else{
            
            if(sendsize >= filesize){
                //已经发送完了
                 LOG_DEBUG("complete, flvfd:%d, sendsize:%ld", flvfd, sendsize);
                 complete();
                 return;
            }
            filesize -= sendsize;
            //计算下一次预发送的字节数量
            sendBytesPerEach = readBytesPerEach + (sendBytesPerEach - sendsize);
            //继续发
            startSendfileTimer();
        }
    }    
    

    
};



#include "EvNIOReader.h"
class Flv2HttpServlet : public HttpServlet, protected EvNIOReader{
public:
    Flv2HttpServlet():flvfd(-1){
        readfiletimer.set<Flv2HttpServlet, &Flv2HttpServlet::timerCallback> (this);
    }
    
    virtual ~Flv2HttpServlet(){

    }
    
    virtual void service(Request& req, Response& resp){
        LOG_DEBUG("Flv2HttpServlet.service");
        resp.setContentType("video/x-flv");
        resp.addDateHeader("Date", time(NULL));

        const char* flvfile = req.getParameter("file");
        if(!flvfile){
            resp.setStatus(400, "error");
            return;
        }

        flvfd = ::open(flvfile, O_RDONLY | O_NONBLOCK);
        if(flvfd > 0){
            timeoutInterval = 1.0;
            EvNIOReader::setFD(flvfd);
            readData();
            resp.setAsynAnswerMode(true);
            return;
        }else{
            resp.setStatus(400, "error");
            return;
        }
    }
    
    virtual bool onWriteQueueEmptyEvent(){
        if(!EvNIOReader::isClosed()){
            if(waitingQueueSize() < 1){
                startReadfileTimer();
            }
        }
        return true;
    } 
    
    
    virtual bool onCompletionEvent(){
        LOG_DEBUG("Flv2HttpServlet.onCompletionEvent");
        closeFlvfile();
        return true;
    }    
    
    
protected:

    virtual bool onReadCompletionEvent(const char* buff, ssize_t rsize, long offset){
        LOG_DEBUG("Flv2HttpServlet.onReadCompletionEvent.rsize:%ld", rsize);
        if(rsize > 0){
            if(!response->write(buff, rsize)){
                response->setStatus(400, "send flv error");
                complete();
            }
            response->flushBuffer();
            
        }else if(rsize < 0){
            response->setStatus(400, "read flv error");
            complete();
        }else{
            complete();
        }
        return true;
    }    
    
    
private:
    void closeFlvfile(){
        EvNIOReader::close();
        if(readfiletimer.is_active()){
            readfiletimer.stop();
        }
        if(flvfd > 0){
          ::close(flvfd);
          flvfd = -1;
        }
    }    
    
    void complete(){
        closeFlvfile();
        //向Hander发送消息
        response->complete();
    }    
    
    void startReadfileTimer(){
        if(!readfiletimer.is_active()){
            readfiletimer.start(timeoutInterval, 0);
        }
    }    
    
    void readData(){
        static int bytesPerSecond = 1024*100; //速率，计划每秒读字节数量
        //计算读数量
        int rsize = (int)(bytesPerSecond*timeoutInterval);
        read(buff, rsize<BUFF_SIZE?rsize:BUFF_SIZE);
    }    
    
    void timerCallback(ev::timer &ev, int revents){
        readData();
    }
    
private:
    FD flvfd;
    char buff[BUFF_SIZE];
    
    ev::timer readfiletimer; //读文件定时器，用来控制读文件速率
    double timeoutInterval; //单位：秒
};

class FlvHttpServlet : public HttpServlet{
public:
    FlvHttpServlet():srfd(-1), readBytes(0), timeoutInterval(0.0), readenable(true){
        readfileio.set<FlvHttpServlet, &FlvHttpServlet::readCallback> (this);
        readfiletimer.set<FlvHttpServlet, &FlvHttpServlet::timerCallback> (this);
    };
    virtual ~FlvHttpServlet(){
        closeReadfileEv();
    };
    
    /**
     * 响应完后，触发此事件，主要用于异步向客户端响应数据时，得知响应完成情况, 
     * 只要是请求完成就会触发此事件，而不管是正常还是异常完成
     * @return 
     */
    virtual bool onCompletionEvent(){
//        cout<<"FlvHttpServlet.onCompletionEvent"<<endl;
        //请求已经处理完成或错误，所以需要停止readfileio
        closeReadfileEv();
        return true;
    }    
    
    virtual bool onWriteQueueEmptyEvent(){
        if(srfd > -1){
            startReadfileTimer();
        }
        return true;
    }    
    
    void timerCallback(ev::timer &ev, int revents){
//        cout<<"FlvHttpServlet.timerCallback, response->waitingWriteQueueSize():"<<response->waitingWriteQueueSize()<<",clock():"<<clock()<<endl;
        //feedReadfileEv();
        //设置能读标记
//        if(response->waitingWriteQueueSize() < 1){
            readenable = true;
            feedReadfileEv();
//        }
//        readfiletimer.stop();
    }
    
    void readCallback(ev::io &evio, int revents){
        if(response->waitingWriteQueueSize() > 1){
            stopReadfileEv();
            return;
        }
        
        if(!readenable){
            return;
        }
        readenable = false;
        
//        cout<<"1 FlvHttpServlet.readCallback.reading, response->waitingWriteQueueSize():"<<response->waitingWriteQueueSize()<<endl;
            
        errno = 0;
        int rsize = ::read(evio.fd, (char *)buff, BUFF_SIZE);
//        cout<<"FlvHttpServlet.readCallback:"<<rsize<<endl;
        if(rsize > 0){
            readBytes +=  rsize;
            if(!write(rsize)){
                writeStatus(400, "send flv error");
                complete();
                return;
            }
        }else if(rsize < 0 && errno != EAGAIN){
            writeStatus(400, "read flv error");
            complete();
            return;
        }else if(rsize == 0){
            complete();
            return;
        }
        
//        cout<<"2 FlvHttpServlet.readCallback.reading, response->waitingWriteQueueSize():"<<response->waitingWriteQueueSize()<<endl;
        if(response->waitingWriteQueueSize() > 1){
            stopReadfileEv();
        }else{
            startReadfileTimer();
//            feedReadfileEv();
        }
        
        
    }
    
    void closeReadfileEv(){
        stopReadfileEv();
//        if(readfiletimer.is_active()){
//            readfiletimer.stop();
//        }
        if(srfd > 0){
          ::close(srfd);
          srfd = -1;
        }
    }
    
    void complete(){
//        cout<<"FlvHttpServlet.readCallback.complete, readBytes:"<<readBytes<<endl;
//        readfileio.stop();
        closeReadfileEv();
        
        //向Hander发送消息
        if(response){
            response->complete();
        }
    }
    
    bool write(size_t size){
        if(response && size > 0){
            return response->write(buff, size);
        }
        return false;
    }
    
    void writeStatus(int code, const char* msg){
        if(response){
            response->setStatus(code, msg);
        }
    }    
    
    virtual void service(Request& req, Response& resp){
        resp.setContentType("video/x-flv");
        resp.addDateHeader("Date", time(NULL));

        const char* flvfile = req.getParameter("file");
        if(!flvfile){
            resp.setStatus(400, "error");
            return;
        }
        
        srfd = ::open(flvfile, O_RDONLY | O_NONBLOCK);
        if(srfd > 0){
//            SocketUtils::setNonblock(srfd);
//            cout<<"readfileio.fd:"<<readfileio.fd<<endl;
//            cout<<"srfd:"<<srfd<<endl;
            readfileio.set(srfd, EV_READ);
//            readfiletimer.set(0, 1);
            timeoutInterval = 1.0;
            startReadfileEv();
//            readfileio.start();
//            readfileio.loop.feed_fd_event(srfd, EV_READ);
            
            resp.setAsynAnswerMode(true);
            return;
        }else{
            resp.setStatus(400, "error");
            return;
        }
    }
    
private:
//    Response* response;
    size_t readBytes;
    ev::io readfileio;
    FD srfd;
    char buff[BUFF_SIZE];
    
    ev::timer readfiletimer; //读文件定时器，用来控制读文件速率
    double timeoutInterval; //单位：秒
    bool readenable;
private:
    void startReadfileEv(){
        if(srfd > -1){
//            cout<<"FlvHttpServlet.startReadfileEv, fd:"<<srfd<<endl;
            if(!readfileio.is_active()){
                readfileio.start();
            }
            feedReadfileEv();
            if(!readfiletimer.is_active() && timeoutInterval > 0.0){
//                readfiletimer.set(timeoutInterval, 0);
                readfiletimer.start(timeoutInterval, 0);
            }
        }
    }

    void startReadfileTimer(){
        if(!readfiletimer.is_active()){
            readfiletimer.start(timeoutInterval, 0);
        }
    }
    
    void feedReadfileEv(){
        if(srfd > -1){
            if(!readfileio.is_active()){
                readfileio.start();
            }
            if(!readfileio.is_pending()){
                readfileio.loop.feed_fd_event(srfd, EV_READ);
            }
        }
    }    
    
    void stopReadfileEv(){
        if(srfd > -1){
//            cout<<"FlvHttpServlet.stopReadfileEv, fd:"<<srfd<<endl;
            if(readfileio.is_active()){
                readfileio.stop();
            }
            
            if(readfiletimer.is_active()){
                readfiletimer.stop();
            }
        }
    }
    
};


class ImageHttpServlet : public HttpServlet{
public:
    ImageHttpServlet(){};
    virtual ~ImageHttpServlet(){};
    
    virtual void service(Request& req, Response& resp){
        resp.setContentType("image/jpeg");
        resp.addDateHeader("Date", time(NULL));
        resp.addHeader("Last-Modified", "Fri, 03 Jun 2011 08:19:31 GMT");
        
        string imgpath = string("/root").append(req.path);
         //ifstream conf(imgpath.c_str(), ios::ate | ios::binary);
        ifstream img(imgpath.c_str(), ios::binary);
        if (!img){
            resp.setStatus(400, "error");
            return;
        }
        
        //resp.addDateHeader("Last-Modified", img.);
         //conf.seekg(ios::beg);
         img>>noskipws;
         
//         int bsize = 1024;
//         char buff[bsize];
//         while(!img.eof()){
//             img.read(buff, bsize);
//             int rsize = img.gcount();
//             resp.write(buff, rsize);
//         }
         
         char c;
         for(int i=0; img >> c; i++){
             resp.write(&c, 1);
         }
         img.close();
     }
    
};

class StatHttpServlet : public HttpServlet{
public:
    StatHttpServlet(){};
    virtual ~StatHttpServlet(){};
    
    virtual void service(Request& req, Response& resp){
        resp.setContentType("text/html; charset=utf-8");
        
        bool respStatus = false;
        FD srfd = ::open("evhttpd_statistical_results.html", O_RDONLY);
        if(srfd){
            if(flock(srfd, LOCK_EX | LOCK_NB) == 0){
                #define RBUFF_SIZE 1024*10
                char buff[RBUFF_SIZE];
                int rsize = ::read(srfd, buff, RBUFF_SIZE);
                if(rsize > 0){
                    resp.write(buff, rsize);
                    respStatus = true;
                }
                flock(srfd, LOCK_UN);
            }
            ::close(srfd);
        }
        
        if(!respStatus){
            static const char* headstr = "<head>\n"
            "<title>stat</title>\n"
            "<meta http-equiv=\"refresh\" content=\"3\">\n"
            "</head>"
            "<body>\n"; 
            resp.write(headstr);
            resp.write("<br>Statistical data being generated, please wait ...<br>");
            resp.write("\n<body>");
        }
       
    }
};




#include "HttpServletFactory.h"

class ExampleHttpServletFactory : public HttpServletFactory{
public:
    ExampleHttpServletFactory(){
    
    }
    virtual ~ExampleHttpServletFactory(){}

    virtual HttpServlet* create(const char* path){
//        cout<<"ExampleHttpServletFactory.create. path:"<<path<<endl;
        
        if(strncmp(path, "/d/", 3) == 0){
            return new IOPumpHttpServlet();
            
        }else if(strncmp(path, "/dwn/", 5) == 0){
            return new DownloadHttpServlet();          
            
        }else if(strncmp(path, "/sf/", 4) == 0){
            return new SendfileHttpServlet();
            
        }else if(strcmp(path, "/flv2") == 0){
            return new Flv2HttpServlet();       
            
        }else if(strcmp(path, "/flv") == 0){
            return new FlvHttpServlet();
            
//        }else if(strcmp(path, "/aio") == 0){
//            return new FlvAIOHttpServlet();            
            
        }else if(strcmp(path, "/empty") == 0){
            return new EmptyHttpServlet();
        }else if(strncmp(path, "/img/", 5) == 0){
            return new ImageHttpServlet();
        }else if(strcmp(path, "/upload") == 0){
            return new UploadHttpServlet();
        }
        return new ExampleHttpServlet();
    }
    
    void free(HttpServlet* servlet){
//        cout<<"ExampleHttpServletFactory.layBack"<<endl;
        delete servlet;
    }    
private:
//    ExampleHttpServlet exampleHttpServlet;
//    EmptyHttpServlet emptyHttpServlet;
//    ImageHttpServlet imageHttpServlet;
//    UploadHttpServlet* uploadHttpServlet;
    
};



class IDLE{
public:
    IDLE(HttpServer* httpServer):httpServer(httpServer){
    
    }
    virtual ~IDLE(){}    
    void idleCallback(ev::idle &evidle, int revents){
//        cout<<"idleCallback"<<endl;
        
//        std::vector<HttpProcess*> httpProcesses;
//        httpServer->getDispatcher()->getHttpProcesses(httpProcesses);
//        for(int i=0; i<httpProcesses.size(); i++){
//            httpProcesses[i]->queryActiveRequests();
//        }
        
    }
    
    HttpServer* httpServer;
};


class TestTimer{
private:
    vector<char> statInfo;
    
    void addStatInfo(const char* info){
        statInfo.insert(statInfo.end(), info, info+strlen(info));
    }
    void addStatInfo(long info){
        char buff[64];
        sprintf(buff, "%ld", info);
        statInfo.insert(statInfo.end(), buff, buff+strlen(buff));
    }
public:
    TestTimer(){
    
    }
    virtual ~TestTimer(){}    
    
    
    void timerCallback(ev::timer &evtimer, int revents){

        if(httpServerPt){
            
            
//            addStatInfo("text/html; charset=utf-8");
            static const char* headstr = "<head>\n"
            "<title>stat</title>\n"
            "<meta http-equiv=\"refresh\" content=\"5\">\n"
            "</head>"
            "<body>\n";
    //        cout<<"---getpid():"<<getpid()<<"---------------------------"<<endl;
            addStatInfo(headstr);
            addStatInfo("The process id is :");
            addStatInfo(getpid());
            addStatInfo("<br>");
            addStatInfo("[statistical results]<br><br>");
            if(httpServerPt){
                std::vector<HttpProcess*> httpProcesses;
                httpServerPt->getDispatcher()->getHttpProcesses(httpProcesses);
    //            cout<<"httpProcesses.size():"<<httpProcesses.size()<<endl;
                unsigned long allTotalRequestCount = 0;
                unsigned long allActiveHandlerCount = 0;
                unsigned long allHandlerCount = 0;
                char outstr[200];
                for(int i=0; i<httpProcesses.size(); i++){
                    if(!httpProcesses[i]->isAlive()){
                        continue;
                    }
                    allTotalRequestCount += httpProcesses[i]->getTotalRequestCount();
                    allActiveHandlerCount += httpProcesses[i]->getActiveHandlerCount(); 
                    allHandlerCount += httpProcesses[i]->getHttpHandlerCount();

                    sprintf(outstr, "Process(%ld), total requests:(%d), active handlers:(%d), all handlers:(%d)<br>", httpProcesses[i]->getId(), httpProcesses[i]->getTotalRequestCount(), httpProcesses[i]->getActiveHandlerCount(), httpProcesses[i]->getHttpHandlerCount());
                    addStatInfo(outstr);
    //                cout<<"getpid():"<<getpid()<<endl;
    //                cout<<(long(httpProcesses[i]))<<", pid is:"<<httpProcesses[i]->getId()<<", active requests:"<<httpProcesses[i]->getActiveHandlerCount()<<endl;
    //                cout<<(long(httpProcesses[i]))<<", pid is:"<<httpProcesses[i]->getId()<<", http handlers:"<<httpProcesses[i]->getHttpHandlerCount()<<endl;

                    httpProcesses[i]->queryActiveHandlerCount(); //发送查询活动请求的消息
                    httpProcesses[i]->queryTotalRequestCount(); //发送查询所有已完成请求的消息
                    httpProcesses[i]->queryHttpHandlerCount(); //发送查询HttpHandler数量的消息                
                }
                //计算每秒钟完成请求数量

                double ctime = getCurrentTime();

    //            cout<<"ctime:"<<ctime<<endl;
                double itime = ctime-lastTime;
    //            cout<<"itime:"<<itime<<endl;
                double reqCountPerSecond = (double)(allTotalRequestCount - lastTotalRequestCount)/(itime>0?itime:1);
                //if(reqCountPerSecondPeak > 0){
                    reqCountPerSecondPeak = reqCountPerSecondPeak<reqCountPerSecond?reqCountPerSecond:reqCountPerSecondPeak;
                //}else{
                //    reqCountPerSecondPeak = 0;
                //}
    //            cout<<"reqCountPerSecond:"<<reqCountPerSecond<<endl;
                //记录最近一次累计完成的请求数量,和当前时间
                lastTotalRequestCount = allTotalRequestCount;
                lastTime = ctime;

                addStatInfo("<br>");
                sprintf(outstr, "collect: Requests per second:(%0.00f), requests per second peak:(%0.00f),  all total requests:(%d), all active handlers:(%d), all handlers:(%d)<br>", reqCountPerSecond, reqCountPerSecondPeak, allTotalRequestCount, allActiveHandlerCount, allHandlerCount);
                addStatInfo(outstr);
            }

            addStatInfo("<br>[note]<br>");
            char notestr[100];
            sprintf(notestr, "work process count (%d), connection idle timeout time (%0.0f)s, http handler pool size (%d)<br>", config.workProcessCount, config.idleTimeout, config.poolMaxHttpHandlerCount);
            addStatInfo(notestr);

            addStatInfo("\n<body>");
            
            
            //将统计信息写入统计文件
//            statInfo.push_back('\0');
//            cout<<"statInfo:"<<statInfo.data()<<endl;
            FD srfd = ::open("evhttpd_statistical_results.html", O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
            if(srfd){
                if(flock(srfd, LOCK_EX | LOCK_NB) == 0){
                    ::write(srfd, statInfo.data(), statInfo.size());
                    flock(srfd, LOCK_UN);
                }
                ::close(srfd);
            }
            
            statInfo.clear();
            
        }
    }

};

#include "events.h"
#include "event/IEventListener.h"

class ExampleEventListener : public IEventListener{
public:
    ExampleEventListener(){};
    virtual ~ExampleEventListener(){};

    virtual bool handle(const Event* e){
        switch(e->getType()){
            case EVENT_TYPE_PROCESS_STARTED:
                return onProcessStartedEvent((const ProcessStartedEvent*)e);
            case EVENT_TYPE_PROCESS_RECEIVE_MESSAGE:
                return onProcessReceiveMessageEvent((const ProcessReceiveMessageEvent*)e);
        }
        return false;
    }
    
    bool onProcessStartedEvent(const ProcessStartedEvent* e){
        //因为子进程会继承父进程的资源，所以在子进程启动后需要清理不需要的
        if(e->inChildProcess){
            //在子进程中
            if(evtimerPt){
                evtimerPt->stop();
            }
            if(monitorServerPt){
                monitorServerPt->stop();
                monitorServerPt->getDispatcher()->killProcesses();
            }
        }else{
            //在父进程中
        }
        return true;
    }
    
    
    bool onProcessReceiveMessageEvent(const ProcessReceiveMessageEvent* e){
        LOG_INFO("e->succeed:%d, e->childProcess:%d", e->succeed, e->childProcess);
        if(e->succeed && !e->childProcess && e->message->type == MSG_TYPE_QUERY_ACTIVE_HANDLERS){
            LOG_INFO("activeHandlers:%d", *((int*)e->message->data));
        }
        return true;
    }
    
};

class MonitorEventListener : public IEventListener{
public:
    MonitorEventListener():httpServer(NULL){};
    virtual ~MonitorEventListener(){
//        if(httpServer)delete httpServer;
    };

    virtual bool handle(const Event* e){
        switch(e->getType()){
            case EVENT_TYPE_PROCESS_STARTED:
                return onProcessStartedEvent((const ProcessStartedEvent*)e);
                break;
        }
        return false;
    }
    
    bool onProcessStartedEvent(const ProcessStartedEvent* e){
        //因为子进程会继承父进程的资源，所以在子进程启动后需要清理不需要的
        if(e->inChildProcess){
            if(evtimerPt){
                evtimerPt->stop();
            }
            if(httpServerPt){
                httpServerPt->stop();
                httpServerPt->getDispatcher()->killProcesses();
            }
        }else{
            
        }
        
        return true;
    }
    
private:
        HttpServer* httpServer;
    
};

int main44(int argc, char** argv) {
    
//    setSignalHandler();
    
//    ChildCommandListener childCommandListener;
    ExampleEventListener exampleEventListener;
    Http11HandlerFactory handlerFactory;
    
//    Config config;
    config.workProcessCount = 1;
    config.poolMaxHttpHandlerCount = 0;
    config.idleTimeout = 30;
    config.responseBufferSize = 1024*128;
    config.requestBufferSize = 1024*16;
    config.handlerFactory = &handlerFactory;
    config.eventListener = &exampleEventListener;

    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    //定时器
    TestTimer timer;
    ev::timer evtimer;
    evtimerPt = &evtimer;
    evtimer.set<TestTimer, &TestTimer::timerCallback>(&timer);
    evtimer.start(0, 5);
    
    //配置HttpServer
    ExampleHttpServletFactory servletFactory;
    HttpServer httpServer(&address, &servletFactory, &config);
//    HttpServer httpServer(&address, &exampleHttpServlet, &config);
    httpServerPt = &httpServer;
    
    //配置MonitorServer
    MonitorEventListener monitorEventListener;
    Config monitorConfig;
    monitorConfig.workProcessCount = 1;
    monitorConfig.idleTimeout = 30;
    monitorConfig.poolMaxHttpHandlerCount = 0;
    monitorConfig.eventListener = &monitorEventListener;    
    StatHttpServlet statHttpServlet;
    HttpServer monitorServer(3089, &statHttpServlet, &monitorConfig);
    monitorServerPt = &monitorServer;
    
    
    //启动HttpServer
    if(!httpServer.start()){
        return 1;
    }
    
    //启动监控HttpServer
    if(!monitorServer.start()){
        return 1;
    }
    
    //进入EV事件循环
    HttpServer::loop();
    
    return (EXIT_SUCCESS);
}

