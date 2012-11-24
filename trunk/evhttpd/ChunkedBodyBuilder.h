/* 
 * File:   ChunkedBodyBuilder.h
 * Author: try
 *
 * Created on 2011年7月16日, 下午11:11
 */

#ifndef CHUNKEDBODYBUILDER_H
#define	CHUNKEDBODYBUILDER_H

#include <vector>
#include "IBodyBuilder.h"


class ChunkedBodyBuilder : public IBodyBuilder{
public:
    ChunkedBodyBuilder();
    virtual ~ChunkedBodyBuilder();
    
    /**
     * 构建主体数据，实际构建好的数据在onBody中返回
     * 
     * @param dataBuff 本次读到的主体数据
     * @param dataSize 本次读到的主体数据量
     * 
     * @return true, 表示数据已经读完了，false说明还要继续读
     */
    virtual bool build(const char* dataBuff, size_t dataSize);
    
    
protected:
    
    /**
     * @see IBodyBuilder#onBuildCompletionEvent(const char* buff, size_t n)
     */
    virtual bool onBuildCompletionEvent(const BodyBuildCompletionEvent* e){
        return false;
    }
    
private:
    void fireBuildCompletionEvent(const char* buff, ssize_t size){
        event.dataBuff = buff;
        event.length = size;
        if(onBuildCompletionEvent(&event)){
            return;
        }
        
        fire(&event);
    }
    
private:
    BodyBuildCompletionEvent event;
    
    char lastChar;
    //块大小16进制表示法
    std::vector<char> chunkSizeBuff;
    //块大小，默认为-1，如果为0说明块接收完成
    long chunkSize;
    //当前在一个数据块中已经读出的数据大小
    long chunkReadedSize;
    
    
};

#endif	/* CHUNKEDBODYBUILDER_H */

