/* 
 * File:   MemoryPool.h
 * Author: try
 *
 * Created on 2011年7月8日, 下午11:48
 */

#ifndef MEMORYPOOL_H
#define	MEMORYPOOL_H
#include "resources.h"
#include <sys/types.h>
#include <ext/hash_map>
#include <ext/hash_set>
#include <stdlib.h>

/**
 * 还未完全实现的简单内存池, 当前分配与释放内存也是直接调用系统函数实现，在此引入
 * Group功能，在Group中分配的内存可批量释放
 */
class MemoryPool {
private:
    MemoryPool();
    virtual ~MemoryPool();
public:
    
    /**
     * 用于标识同一组别内存
     */
    class Group{
    public:
        Group();
        virtual ~Group();
        
        
        /**
         * 申请内存, 注意：只能使用同组中的free或freeAll方法来释放
         */
        void* malloc(size_t size);

        /**
         * 释放内存, 只可以释放同组中的malloc或copy方法分配的内存
         */
        void free(void* ptr);
        
        /**
         * 复制内存, 注意：只能使用同组中的free或freeAll方法来释放
         * 
         * @param newSize 为新内存长度
         * @param copySize 为从src中复制的长度， copySize必须小于等于newSize
         * 
         * @return 返回新的内存地址
         */
        void* copy(const void* src, size_t newSize, size_t copySize);

        /**
         * 复制内存, 注意：只能使用同组中的free或freeAll方法来释放
         */
        void* copy(const char* src, size_t size);
        
        
        /**
         * 释放此组中所有内存
         */
        void freeAll();
        
    private:
        typedef __gnu_cxx::hash_set<void*, PointerHash, PointerCompare> MemoryPtrs;
        MemoryPtrs memoryPtrs;        
        
    };
    
public:
    static MemoryPool instance;

//    /**
//     * @param coreSize 核心内存池大小，单位:字节
//     * @param maxSize 最大内存池大小，单位:字节, maxSize不能小于coreSize，
//     *     maxSize参数小于coreSize，maxSize将被设置为maxSize=coreSize
//     * @param pageSize 内存分页大小，默认使用系统分页大小
//     */
//    static void init(size_t coreSize, size_t maxSize, int pageSize=getpagesize());
    
public:
//    size_t getCoreSize(){
//        return coreSize;
//    }
//    
//    size_t getMaxSize(){
//        return maxSize;
//    }
//    
//    size_t getActiveSize(){
//        return activeSize;
//    }
    
    /**
     * 请求内存, 如果池满，将会直接分配系统内存
     */
    void* malloc(size_t size);
    
    /**
     * 重新请求, 如果池满，将会直接分配系统内存
     */    
    void* realloc(void* ptr, size_t size);
    
    /**
     * 释放内存, 如果此内存指针指向的内存没在池中，将会直接释放
     */
    void free(void* ptr);
    
private:
//    typedef struct Entry{
//        char* ptr;  //指向内存块的指针
//        size_t bytes; //大小, 以字节为单位
//    }Entry;
    
    
    
private:
//    size_t coreSize;
//    size_t maxSize;
//    size_t activeSize; //活动内存大小， 单位：字节
//    int pageSize;
//    bool initFlag;

};

#endif	/* MEMORYPOOL_H */

