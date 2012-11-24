/* 
 * File:   IBodyBuilder.h
 * Author: try
 *
 * Created on 2011年7月16日, 下午9:01
 */

#ifndef IBODYBUILDER_H
#define	IBODYBUILDER_H

#include "events.h"
#include "event/EventDispatcher.h"

/**
 * 实现此接口来构建请求或响应的主体内容
 */
class IBodyBuilder : public EventDispatcher{
public:
    IBodyBuilder(){}
    virtual ~IBodyBuilder(){}
    
    /**
     * 构建主体数据，实际构建好的数据在onBody中返回
     * 
     * @param dataBuff 本次读到的主体数据
     * @param dataSize 本次读到的主体数据量
     * 
     * @return true, 表示数据已经读完了，false说明还要继续读
     */
    virtual bool build(const char* dataBuff, size_t dataSize) = 0;    
    
protected:    
    /**
     * 在子类型中覆盖此方法来获取主体数据
     * 
     * @param buff 本次读出的数据
     * @param n 本次读出的数据量, 如果n为0表未数据已经读完，-1表示有错
     * @return true，表示操作完成，false, 还未完
     */
    virtual bool onBuildCompletionEvent(const BodyBuildCompletionEvent* e)=0;
    

};

#endif	/* IBODYBUILDER_H */

