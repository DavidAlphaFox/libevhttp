/* 
 * File:   RequestData.h
 * Author: try
 *
 * Created on 2011年5月30日, 下午5:54
 */

#ifndef _REQUESTDATA_H
#define	_REQUESTDATA_H
#include <vector>
#include <ext/hash_map>
#include <string>
#include <iostream>
#include <netinet/in.h>
#include "KeyValues.h"

/**
 * 具体请求数据，包括请求行，消息头，主体位置信息等
 */
class RequestData {

public:
    RequestData();
    virtual ~RequestData();


    /** 清理 */
    void clean();

    /** 生成消息头位置信息 */
    void saveHeaderPos(int keyStartPos, int keyEndPos, int valueStartPos, int valueEndPos);

    /** 构建请求行信息 */
    void buildRequestLine();

    /** 构建消息头信息 */
    void buildRequestHeaders();    

//    /* 构建请求主体 */
//    void buildRequestBody(int contentLength);

    /** 构建请求参数 */
    void buildRequestParameters();


public:
    //请求原始数据，包括请求行，头和部份主体数据
    typedef std::vector<char> Data;
    typedef Data::iterator DataIterator;
    Data data;

    int notTraversePos;    //在原始数据中未遍历过的位置
    
public:

    /* 请求行，请求方式_资源路径_协议及版本号 */
    //位置表示法
    bool existReqLine; //请求行存在标记
    int reqLineStartPos;  //请求行开始位置, 默认为：0
    int reqLineEndPos;    //请求行结束位置
    int methodStartPos;   //请求方式开始位置
    int methodEndPos;   //请求方式结束位置
    int pathStartPos;   //资源路径开始位置
    int pathEndPos;   //资源路径结束位置
    int parameterStartPos;   //请求查询参数开始位置
    int parameterEndPos;   //请求查询参数结束位置
    int protocolStartPos;   //协议信息开始位置
    int protocolEndPos;   //协议信息结束位置

    //串表示法
    const char* method;     //请求方式
    const char* path;       //资源路径
    const char* parameter;  //请求参数信息
    const char* protocol;   //协议及版本信息

    //以上位置属性仅当existReqLine时有效

    /* 消息头相关 */
    bool existReqHeaders; //消息头存在标记
    int headersStartPos; //消息头开始位置, 指向当前未处理的头
    int headersEndPos;   //最后一个头的结束位置
    int currentHeaderStartPos;  // 指向当前未处理完的头开始位置

    /* 主体数据, 主体数据需要根据请求方式以及主体内容长度决定，在此只记录开始位置 */
    bool existReqBody;
    int bodyStartPos;  //请求主体开始位置

    //所有消息头
    typedef KeyValues<HashMapStrIgnoreCaseCompare> Headers;
    Headers headers;    

    //参数值
    //所有参数数据
    typedef KeyValues<> Parameters;
    Parameters parameters;

    char* parameterDataSource;
    bool parameterPrepared; //参数数据是否准备好标记，默认:false

    //客户端地址信息
    struct sockaddr_in remoteAddr;
    
private:
    //请求行复本
    char* requestLineDuplicate;

    //消息头复本
    char* requestHeadersDuplicate;

    //主体内容复本
    char* requestBodyDuplicate;

    //头位置信息
    //消息头位置信息, 包括头Key和Value
    typedef struct HeaderPos{
        int keyStartPos;    //Key开始位置
        int keyEndPos;      //Key结束位置
        int valueStartPos;    //Value开始位置
        int valueEndPos;      //Value结束位置
    public:
        HeaderPos(int keyStartPos, int keyEndPos, int valueStartPos, int valueEndPos)
            :keyStartPos(keyStartPos), keyEndPos(keyEndPos), valueStartPos(valueStartPos), valueEndPos(valueEndPos)
            {}
    }HeaderPos;
    typedef std::vector<HeaderPos*> HeadersPosData;
    HeadersPosData headersPosData;

private:
    /** 分析参数 */
    void parseParameters(char* p);
    /** 添加参数 */
//    void addParameter(const char* key, const char* value);    
    /** 清理ParameterData */
    void cleanParameterData();
public:


    void print(){

        std::cout<<"[RequestData start]\n";
        std::cout<<(std::string("[").append(data.begin(), data.end()).append("]"))<<std::endl;

        std::cout<<"Request.data.size:"<<data.size()<<std::endl;
        std::cout<<"notTraversePos:"<<notTraversePos<<std::endl;
        std::cout<<"existReqLine:"<<(existReqLine?"true":"false")<<std::endl;
        std::cout<<"reqLineStartPos:"<<reqLineStartPos<<std::endl;
        std::cout<<"reqLineEndPos:"<<reqLineEndPos<<std::endl;
        std::cout<<"methodStartPos:"<<methodStartPos<<std::endl;
        std::cout<<"methodEndPos:"<<methodEndPos<<std::endl;
        std::cout<<"pathStartPos:"<<pathStartPos<<std::endl;
        std::cout<<"pathEndPos:"<<pathEndPos<<std::endl;
        std::cout<<"parameterStartPos:"<<parameterStartPos<<std::endl;
        std::cout<<"parameterEndPos:"<<parameterEndPos<<std::endl;
        std::cout<<"protocolStartPos:"<<protocolStartPos<<std::endl;
        std::cout<<"protocolEndPos:"<<protocolEndPos<<std::endl;
        if(existReqLine){
            std::cout<<"method:"<<method<<std::endl;
            std::cout<<"path:"<<path<<std::endl;
            std::cout<<"parameter:"<<parameter<<std::endl;
            std::cout<<"protocol:"<<protocol<<std::endl;
        }

        std::cout<<"existReqHeaders:"<<(existReqHeaders?"true":"false")<<std::endl;
        std::cout<<"headersStartPos:"<<headersStartPos<<std::endl;
        std::cout<<"headersEndPos:"<<headersEndPos<<std::endl;
        std::cout<<"currentHeaderStartPos:"<<currentHeaderStartPos<<std::endl;

//        if(existReqHeaders){
//            std::vector<const char*> hnames;
//            headers.getNames(hnames);
//            for(int i=0; i<hnames.size(); i++){
//                std::vector<const char*> vals;
//                headers.gets(hnames[i], vals);
//                for(int j=0; j<vals.size(); j++){
//                    std::cout<<hnames[i]<<"="<<(vals[j]?vals[j]:"null")<<std::endl;
//                }
//            }

//            HeadersData::iterator it = headersData.begin();
//            for(; it != headersData.end(); it++){
//                std::cout<<(it->first)<<"="<<(it->second->value.text)<<std::endl;
//                HeaderValue* hv = &(it->second->value);
//                while(hv->next){
//                    std::cout<<""<<(hv->next->text)<<std::endl;
//                    hv = hv->next;
//                }
//            }
//        }

        std::cout<<"bodyStartPos:"<<bodyStartPos<<std::endl;
//        std::cout<<"body:"<<(body?body:"NULL")<<std::endl;
        std::cout<<"[Request end]"<<std::endl;

    }
};

#endif	/* _REQUESTDATA_H */

