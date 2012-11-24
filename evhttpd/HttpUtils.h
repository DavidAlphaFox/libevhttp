/* 
 * File:   HttpUtils.h
 * Author: try
 *
 * Created on 2011年6月15日, 下午2:41
 */

#ifndef HTTPUTILS_H
#define	HTTPUTILS_H
#include <sys/types.h>
#include "KeyValues.h"
#include "IBodyBuilder.h"


/**
 * Http相关工具
 */
class HttpUtils {
public:
    HttpUtils();
    virtual ~HttpUtils();
    
    /**
     * 读固定长度的Body数据, 此方法主要使用在HttpServlet.onReadBodyEvent事件中，
     * 注意：Body数据不应太大，主要用于application/x-www-form-urlencoded类型form
     * 表单等
     * 
     * @param body 指向存放Body数据的数据区, 注意body数据空间要大于contentLength至少一个字节，方便设置串结束标记
     * @param readBytes 指向已经读出的Body数据量
     * @param contentLength Body数据长度
     * @param dataBuff 本次读到的body数据
     * @param dataSize 本次读到的body数据量
     * 
     * @return true, 表示数据已经读完了，false说明还要继续读
     */
    static bool readBody(char* body, size_t* areadBytes, ssize_t contentLength, const char* dataBuff, size_t dataSize);
    
    
//    /**
//     * 读Chunked格式数据
//     * 
//     * @param bodyBuilder 在IBodyBuilder中组装处理读到的主体数据
//     * @param dataBuff 本次读到的主体数据
//     * @param dataSize 本次读到的主体数据量
//     * 
//     * @return true, 表示数据已经读完了，false说明还要继续读
//     */
//    static bool readChunkedBody(IBodyBuilder* bodyBuilder, const char* dataBuff, size_t dataSize);
    
    /**
     * 分析参数数据，要求data参数数据格式为：name1=1&name2=2&name3=3&name=n1&name=n2&name=n3?
     * 
     * @param data 指向具体数据，注意：data数据区最后要多预留一个字节, 并且data中
     *              key和value末尾会自动加上串结束标记'\0'
     * @param params 将分析好的放入到此KeyValues中
     */
    static void parseParameters(char* data, KeyValues<>* params);
    
    
    
    
private:

};

#endif	/* HTTPUTILS_H */

