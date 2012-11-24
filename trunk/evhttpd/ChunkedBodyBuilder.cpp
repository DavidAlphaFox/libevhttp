/* 
 * File:   ChunkedBodyBuilder.cpp
 * Author: try
 * 
 * Created on 2011年7月16日, 下午11:11
 */

#include <errno.h>
#include "resources.h"
#include "ChunkedBodyBuilder.h"


ChunkedBodyBuilder::ChunkedBodyBuilder():event(this), lastChar(0), chunkSize(-1), chunkReadedSize(0) {
}

ChunkedBodyBuilder::~ChunkedBodyBuilder() {
}

bool ChunkedBodyBuilder::build(const char* dataBuff, size_t dataSize){
    int idx = 0; //在dataBuff中的位置
    char c;
    for(; idx < dataSize; idx++){
        if(chunkSize == -1){
            //等于-1，说明正在读chunk数据块控制行
            for(; idx<dataSize; idx++){
                c = dataBuff[idx];
                if(chunkSizeBuff.size() == 0 && (c == CHAR_CR || c == CHAR_LF)){
                    //读第二个块时会有这种情况
                    continue;

                }else  if(c == CHAR_LF){
                    //说明控制行读完了, 检查chunkSizeBuff大小，如果chunkSizeBuff.size==0, 说明控制行错误
                    if(chunkSizeBuff.size()==0){
                        //responseData->errorCode = 1;
                        fireBuildCompletionEvent(dataBuff, -1);//说明控制行错误
                        return true;
                    }
                    chunkSizeBuff[chunkSizeBuff.size()-1] = '\0';
                    errno = 0;
                    chunkSize = strtol(chunkSizeBuff.data(), NULL, 16);
                    if(errno == ERANGE){
                        fireBuildCompletionEvent(dataBuff, -1);//说明控制行错误
                        return true;
                    }
                    idx++;
                    break;
                }else{
                    chunkSizeBuff.push_back(c);
                }
            }
        }

        if(dataSize == idx){
            //已经处理到读出的数据未部了
            break;
        }

        if(chunkSize > 0){
            int nextSize = (dataSize - idx) - (chunkSize - chunkReadedSize );
            if(nextSize >= 0){
                //说明此块已经读完了
                fireBuildCompletionEvent(&dataBuff[idx], chunkSize - chunkReadedSize);
                idx += (chunkSize - chunkReadedSize);
                //准备处理下一块, 初始化相关数据
                chunkSizeBuff.clear();
                chunkSize = -1;
                chunkReadedSize = 0;
                lastChar = 0;
                continue;
            }else{
                int size = dataSize - idx;
                fireBuildCompletionEvent(&dataBuff[idx], size);
                chunkReadedSize += size;
                break;
            }

        }

        if(chunkSize == 0){
            //说明chunk块读完了，开始读扩展头信息
            for(;idx<dataSize; idx++){
                c = dataBuff[idx];
                //扩展头暂时不处理
                if(lastChar == CHAR_LF && c == CHAR_CR){
                    fireBuildCompletionEvent(dataBuff, 0); //数据读完了
                    //所有块数据已经读完了
                    chunkSizeBuff.clear();
                    chunkSize = -1;
                    chunkReadedSize = 0;
                    lastChar = 0;
                    return true;
                }
                lastChar = c;
            }
        }            
    }
    return false;
}


