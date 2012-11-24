/* 
 * File:   StringUtils.h
 * Author: try
 *
 * Created on 2011年6月14日, 下午7:03
 */

#ifndef STRINGUTILS_H
#define	STRINGUTILS_H

/**
 * 串工具
 */
class StringUtils {
public:
    StringUtils();
    virtual ~StringUtils();
    
    /** 将串中字母转换为小写, str必须以'\0'结尾, 注意：str必须是可修改的串空间 */
    static char* toLowerCase(char* str);
    
    /** 将str串中字母转换为小写, str必须以'\0'结尾, 在nstr中返回新串, 返回值同nstr */
    static char* toLowerCase(char* nstr, const char* str);    
    
    /** 将串中字母转换为大写, str必须以'\0'结尾, 注意：str必须是可修改的串空间 */
    static char* toUpperCase(char* str);    
    
    /** 将串中字母转换为大写, str必须以'\0'结尾, 在nstr中返回新串, 返回值同nstr */
    static char* toUpperCase(char* nstr, const char* str);    
    
private:
    //大写转小写数组
    static char lowerCaseChars[];
    //小写转大写数组
    static char upperCaseChars[];
};

#endif	/* STRINGUTILS_H */

