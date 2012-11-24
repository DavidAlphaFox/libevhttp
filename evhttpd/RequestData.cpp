/* 
 * File:   RequestData.cpp
 * Author: try
 * 
 * Created on 2011年5月30日, 下午5:54
 */

#include <ctype.h>
#include <string.h>
#include "KeyValues.h"
#include "RequestData.h"
#include "StringUtils.h"
#include "MemoryPool.h"

using namespace std;

RequestData::RequestData():notTraversePos(0),existReqLine(false), reqLineStartPos(0), reqLineEndPos(0), 
        methodStartPos(0), methodEndPos(0), pathStartPos(0), pathEndPos(0), parameterStartPos(0), parameterEndPos(0), protocolStartPos(0), protocolEndPos(0),
        method(NULL), path(NULL), parameter(NULL), protocol(NULL),
        existReqHeaders(false), headersStartPos(0), headersEndPos(0), currentHeaderStartPos(0),
        existReqBody(false), bodyStartPos(0), //body(NULL),
        requestLineDuplicate(NULL), requestHeadersDuplicate(NULL), requestBodyDuplicate(NULL),
        parameterPrepared(false), parameterDataSource(NULL)

    {
    memset(&remoteAddr, 0, sizeof(remoteAddr));
}

RequestData::~RequestData() {
    clean();
}

/** 清理ParameterData */
void RequestData::cleanParameterData(){
    parameters.clear();
    if(parameterDataSource){
        MemoryPool::instance.free(parameterDataSource);
        parameterDataSource = NULL;
    }
    parameterPrepared = false; //参数数据是否准备好标记，默认:false
}


void RequestData::clean(){
    
    data.clear();

    notTraversePos = 0;

    existReqLine = false;
    reqLineStartPos = 0;  //请求行开始位置, 默认为：0
    reqLineEndPos = 0;    //请求行结束位置
    methodStartPos = 0;   //请求方式开始位置
    methodEndPos = 0;   //请求方式结束位置
    pathStartPos = 0;   //资源路径开始位置
    pathEndPos = 0;   //资源路径结束位置
    parameterStartPos = 0;   //请求查询参数开始位置
    parameterEndPos = 0;   //请求查询参数结束位置
    protocolStartPos = 0;   //协议信息开始位置
    protocolEndPos = 0;   //协议信息结束位置
    method = NULL;     //请求方式
    path = NULL;       //资源路径
    parameter = NULL;  //请求参数信息
    protocol = NULL;   //协议及版本信息

    existReqHeaders = false; //消息头存在标记
    headersStartPos = 0; //消息头开始位置, 指向当前未处理的头
    headersEndPos = 0;   //最后一个头的结束位置
    currentHeaderStartPos = 0;  // 指向当前未处理完的头开始位置

    existReqBody = false; //请求主体存在标记
    bodyStartPos = 0;  //请求主体开始位置

    headers.clear();

    //清除头位置信息
    HeadersPosData::iterator it = headersPosData.begin();
    for(;it != headersPosData.end(); it++){
        delete *it;
    }
    headersPosData.clear();

    if(requestLineDuplicate){
        MemoryPool::instance.free(requestLineDuplicate);
        requestLineDuplicate = NULL;
    }

    if(requestHeadersDuplicate){
        MemoryPool::instance.free(requestHeadersDuplicate);
        requestHeadersDuplicate = NULL;
    }

    if(requestBodyDuplicate){
        MemoryPool::instance.free(requestBodyDuplicate);
        requestBodyDuplicate = NULL;
    }

    //清除参数信息
    cleanParameterData();
//    
//    
//    print();
}


void RequestData::buildRequestLine(){
    if(existReqLine && !requestLineDuplicate){
        int n = reqLineEndPos - reqLineStartPos;
        requestLineDuplicate = (char*)MemoryPool::instance.malloc(n + 1);
        memcpy(requestLineDuplicate, data.data(), n);

        method = &requestLineDuplicate[methodStartPos];
        requestLineDuplicate[methodEndPos] = 0;

        path = &requestLineDuplicate[pathStartPos];
        requestLineDuplicate[pathEndPos] = 0;

        parameter = &requestLineDuplicate[parameterStartPos];
        requestLineDuplicate[parameterEndPos] = 0;

        protocol = &requestLineDuplicate[protocolStartPos];
        requestLineDuplicate[protocolEndPos] = 0;
    }
}


void RequestData::buildRequestHeaders(){
    if(existReqHeaders && !requestHeadersDuplicate){
        int n = headersEndPos - headersStartPos;
        requestHeadersDuplicate = (char*)MemoryPool::instance.malloc(n + 1);
        memcpy(requestHeadersDuplicate, &(data.data()[headersStartPos]), n);
        //requestHeadersDuplicate[n] = 0;
        
        HeadersPosData::iterator it = headersPosData.begin();
        for(;it != headersPosData.end(); it++){
            HeaderPos& headerPos = *(*it);
            int& keyStartPos = headerPos.keyStartPos;
            int& keyEndPos = headerPos.keyEndPos;
            int& valueStartPos = headerPos.valueStartPos;
            int& valueEndPos = headerPos.valueEndPos;

            char* key = &requestHeadersDuplicate[keyStartPos - headersStartPos];
            requestHeadersDuplicate[keyEndPos - headersStartPos] = 0;

            char* text = &requestHeadersDuplicate[valueStartPos - headersStartPos];
            requestHeadersDuplicate[valueEndPos - headersStartPos] = 0;
            headers.add(key, text);
        }
    }
}

void RequestData::saveHeaderPos(int keyStartPos, int keyEndPos, int valueStartPos, int valueEndPos){
    char c;
    for(int i=valueStartPos; i<valueEndPos; i++){
        c = data[i];
        if(!(c == CHAR_SP || c == CHAR_HT || c == CHAR_CR || c == CHAR_LF)){
            valueStartPos = i;
            break;
        }
    }
    headersPosData.push_back(new HeaderPos(keyStartPos, keyEndPos, valueStartPos, valueEndPos));
}


//void RequestData::buildRequestBody(int contentLength){
//    if(!requestBodyDuplicate && contentLength>=0 && (data.size() - bodyStartPos)>=contentLength){
//        requestBodyDuplicate = (char*)MemoryPool::instance.malloc(contentLength + 1);
//        memcpy(requestBodyDuplicate, &(data.data()[bodyStartPos]), contentLength);
//        requestBodyDuplicate[contentLength] = 0;
////        body = requestBodyDuplicate;
//        existReqBody = true;
//    }
//}



void RequestData::buildRequestParameters(){
    if(parameterPrepared){
        return;
    }

    int paramSourceSize = 0;
//    int bodySize = 0;
    int reqLineParamSize = 0;

    //计算参数数据长度
    if(existReqLine){
        reqLineParamSize = parameterEndPos - parameterStartPos;
    }

    //计算主体数据长度
//    if(existReqBody && body){
//        bodySize = 1 + strlen(body);
//    }

//    paramSourceSize = reqLineParamSize + bodySize + 1;
    paramSourceSize = reqLineParamSize + 1;

    parameterDataSource = (char*)MemoryPool::instance.malloc(paramSourceSize);
    char* begin = parameterDataSource;
    if(reqLineParamSize > 0){
        memcpy(begin, parameter, reqLineParamSize);
        begin += reqLineParamSize;
    }
//    if(bodySize > 0){
//        if(parameterDataSource > 0){
//            *begin = '&';
//            begin++;
//        }
//        memcpy(begin, body, bodySize);
//        begin += bodySize;
//    }

    *begin = 0; //设置串结束标记
    parseParameters(parameterDataSource);
    parameterPrepared = true;
}

void RequestData::parseParameters(char* p){
    char* kstart = p;  //Key开始位置
    char* kend = NULL;    //Key开始位置
    char* vstart = NULL; //值开始位置
    char* vend = NULL;   //值结束位置
    char c;
    while(true){
        c = *p;
        switch(c){
            case '=':
                kend = p;
                vstart = p+1;
                break;
            case 0:
            case '&':
                if(kend){
                    vend = p;
                }else{
                    kend = p;
                }
                //处理完成一个参数项
                int ksize = kend - kstart;
                char* key = kstart;
                key[ksize] = 0;

                char* value = NULL;
                if(vstart){
                    int vsize = vend - vstart;
                    if(vsize >= 0){
                        value = vstart;
                        value[vsize] = 0;
                    }
                }
                if(ksize > 0){
                    //addParameter(key, value);
//                    if(strlen(key) != 0){
                        parameters.add(key, value);
//                    }
                }
                
                //准备下一项的参数
                kstart = p+1;
                kend = NULL;
                vstart = NULL;
                vend = NULL;
                break;
        }
        if(!c){//到结果位置, 退出
            break;
        }
        p++;
    }
}

//void RequestData::addParameter(const char* key, const char* value){
////    if(strlen(key) == 0){
////        return;
////    }
//    parameters.add(key, value);
//}



