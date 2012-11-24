/* 
 * File:   MemoryPool.cpp
 * Author: try
 * 
 * Created on 2011年7月8日, 下午11:48
 */

#include "MemoryPool.h"

MemoryPool MemoryPool::instance;

MemoryPool::MemoryPool(){
    
}

MemoryPool::~MemoryPool() {
}

/**
 * 请求内存
 */
void* MemoryPool::malloc(size_t size){
    return ::malloc(size);
}

/**
 * 重新分配内存
 */
void* MemoryPool::realloc(void* ptr, size_t size){
    return ::realloc(ptr, size);
}

/**
 * 释放内存
 */
void MemoryPool::free(void* ptr){
    if(!ptr){
        LOG_ERROR("is NULL")
        return;
    }
    ::free(ptr);
}


MemoryPool::Group::Group(){

}
MemoryPool::Group::~Group(){
   freeAll();
}
        
        
/**
 * 申请内存
 */
void* MemoryPool::Group::malloc(size_t size){
    void* ptr = MemoryPool::instance.malloc(size);
    memoryPtrs.insert(ptr);
    return ptr;
}

/**
 * 释放内存
 */
void MemoryPool::Group::free(void* ptr){
    if(!ptr){
        LOG_ERROR("Group, ptr is NULL")
        return;
    }    
    MemoryPtrs::iterator it = memoryPtrs.find(ptr);
    if(it != memoryPtrs.end()){
        memoryPtrs.erase(it);
        MemoryPool::instance.free(ptr);
    }else{
        LOG_ERROR("Group, the ptr not in Group")
    }
}

/**
 * 复制内存
 * 
 * newlen 为新内存长度
 * copylen 为从src中复制的长度， copylen必须小于等于newlen
 * 
 * @return 返回新的内存地址
 */
void* MemoryPool::Group::copy(const void* src, size_t newSize, size_t copySize){
    void* ptr = MemoryPool::instance.malloc(newSize);
    memcpy(ptr, src, copySize);
    memoryPtrs.insert(ptr);
    return ptr;
}

/**
 * 复制内存
 */
void* MemoryPool::Group::copy(const char* src, size_t size){
    void* ptr = MemoryPool::instance.malloc(size);
    memcpy(ptr, src, size);
    memoryPtrs.insert(ptr);
    return ptr;
}


/**
 * 释放此组中所有内存
 */
void MemoryPool::Group::freeAll(){
    MemoryPtrs::iterator it = memoryPtrs.begin();
    for(; it != memoryPtrs.end(); it++){
        MemoryPool::instance.free(*it);
    }
    memoryPtrs.clear();
}

