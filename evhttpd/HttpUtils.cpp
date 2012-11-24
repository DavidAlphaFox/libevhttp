/* 
 * File:   HttpUtils.cpp
 * Author: try
 * 
 * Created on 2011年6月15日, 下午2:41
 */

#include "HttpUtils.h"
#include "resources.h"

HttpUtils::HttpUtils() {
}

HttpUtils::~HttpUtils() {
}

bool HttpUtils::readBody(char* body, size_t* areadBytes, ssize_t contentLength, const char* dataBuff, size_t dataSize){
    if(contentLength < 0){
        return true;
    }
    int unreadBytes = contentLength - *areadBytes;
    if(dataSize < unreadBytes){
        memcpy(body + *areadBytes, dataBuff, dataSize);
        *areadBytes += dataSize;
        return false;
    }else{
        memcpy(body + *areadBytes, dataBuff, unreadBytes);
        *areadBytes += unreadBytes;
        body[contentLength] = 0;
        //Body数据已经读完了
        return true;
    }
}


void HttpUtils::parseParameters(char* data, KeyValues<>* params){
    char* kstart = data;  //Key开始位置
    char* kend = NULL;    //Key开始位置
    char* vstart = NULL; //值开始位置
    char* vend = NULL;   //值结束位置
    char c;
    while(true){
        c = *data;
        switch(c){
            case '=':
                kend = data;
                vstart = data+1;
                break;
            case 0:
            case '&':
                if(kend){
                    vend = data;
                }else{
                    kend = data;
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
                    if(strlen(key)!=0){
                        params->add(key, value);
                    }
                }
                
                //准备下一项的参数
                kstart = data+1;
                kend = NULL;
                vstart = NULL;
                vend = NULL;
                break;
        }
        if(!c){//到结果位置, 退出
            break;
        }
        data++;
    }
}

