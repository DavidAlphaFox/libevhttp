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



bool HttpUtils::urlEncode(const char* srcurl, std::string& dsturl){
    if(!srcurl)return false;
    int i; 
    char ch; 
    int srcurlsize = strlen(srcurl);
    char buff[32];
    for (i=0; i<srcurlsize; i++) {
        ch = srcurl[i]; 
        if ((ch >= 'A') && (ch <= 'Z')) { 
            //ss_string_append_char(dsturl, ch); 
            dsturl.append(1, ch);
        } else if ((ch >= 'a') && (ch <= 'z')) { 
            //ss_string_append_char(dsturl, ch); 
            dsturl.append(1, ch);
        } else if ((ch >= '0') && (ch <= '9')) { 
            //ss_string_append_char(dsturl, ch); 
            dsturl.append(1, ch);
        } else if(ch == ' '){ 
            //ss_string_append_char(dsturl, '+');
            dsturl.append(1, '+');
        } else { 
           sprintf(buff, "%%%02X", (unsigned char)ch); 
           //ss_string_append_cstr(dsturl, buff);
           dsturl.append(buff);
        } 
    } 
    return true;
}


char HttpUtils::char2num(char ch){ 
    if(ch>='0' && ch<='9')return (char)(ch-'0'); 
    if(ch>='a' && ch<='f')return (char)(ch-'a'+10); 
    if(ch>='A' && ch<='F')return (char)(ch-'A'+10); 
    return CHAR_ZERO; 
}

bool HttpUtils::urlDecode(const char* srcurl, std::string& dsturl){
    if(!srcurl)return false;
    char ch, ch1, ch2; 
    int i; 
    int srcurlsize = strlen(srcurl);
    for (i=0; i<srcurlsize; i++) {
        ch = srcurl[i]; 
        switch (ch) { 
            case '+': 
                //ss_string_append_char(dsturl, SS_CHAR_SPACE); 
                dsturl.append(1, (char)CHAR_SP);
                break;
            case '%': 
                if (i+2 < srcurlsize) { 
                    ch1 = char2num(srcurl[i+1]); 
                    ch2 = char2num(srcurl[i+2]); 
                    if ((ch1 != CHAR_ZERO) && (ch2 != CHAR_ZERO)) { 
                        //ss_string_append_char(dsturl, (char)((ch1<<4) | ch2)); 
                        dsturl.append(1, (char)((ch1<<4) | ch2)); 
                        i += 2; 
                        break; 
                    }
                }

            default: 
                //ss_string_append_char(dsturl, ch); 
                dsturl.append(1, ch);
                break; 
        } 
    }
    
    return true;
}



