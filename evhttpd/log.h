/* 
 * File:   log.h
 * Author: try
 *
 * Created on 2011年7月1日, 上午12:26
 */

#ifndef LOG_H
#define	LOG_H

#include <iostream>
#include <string>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <stdio.h>


/**
 * 使用宏实现的简单LOG工具
 */

//1 DEBUG, 2 INFO, 3 WARN, 4 ERROR
#define LOG_DEBUG_LEVEL 1
#define LOG_INFO_LEVEL 2
#define LOG_WARN_LEVEL 3
#define LOG_ERROR_LEVEL 4

static char _debug_buff[1024*100];
static std::string getLevelText(int lv){
    switch(lv){
        case LOG_DEBUG_LEVEL:
            return std::string("DEBUG");
            break;
        case LOG_INFO_LEVEL:
            return std::string("INFO");
            break;
        case LOG_WARN_LEVEL:
            return std::string("WARN");
            break;
        case LOG_ERROR_LEVEL:
            return std::string("ERROR");
            break;            
    }
    return std::string("UNKNOWN"); //unknown
}

static double getSystemSecondTime(){
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return tv.tv_sec+(double)tv.tv_usec/1000000.0;
}

static std::string formatSystemTime(){
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    char timestr[64];
    tv.tv_sec += tz.tz_dsttime;
    char* usecPtr = &(timestr[strftime(timestr,32,"%C-%m-%d %H:%M:%S.",gmtime(&tv.tv_sec))]);
    sprintf(usecPtr, "%ld", tv.tv_usec);
//    sprintf(usecPtr, "%s", usecPtr);
    return std::string(timestr);
}

#ifndef LOG_LEVEL
  #define LOG_LEVEL LOG_WARN_LEVEL
#endif 

#define _LOG_(lv, file, line, func, p...)	   \
        sprintf(_debug_buff, p);               \
        std::cout<<"["<<getLevelText(lv)<<"] "<<formatSystemTime()<<" PID:"<<getpid()<<" TID:"<<pthread_self()<<(" ")<<file<<"#"<<func<<"("<<line<<") "<<_debug_buff<<std::endl;             \

//DEBUG
#if LOG_LEVEL <= LOG_DEBUG_LEVEL
  #define LOG_DEBUG(p...)  _LOG_(LOG_DEBUG_LEVEL, __FILE__, __LINE__, __FUNCTION__, p)
#else
  #define LOG_DEBUG(p...)
#endif

//INFO
#if LOG_LEVEL <= LOG_INFO_LEVEL
  #define LOG_INFO(p...)  _LOG_(LOG_INFO_LEVEL, __FILE__, __LINE__, __FUNCTION__, p)
#else
  #define LOG_INFO(p...)
#endif

//WARN
#if LOG_LEVEL <= LOG_WARN_LEVEL
  #define LOG_WARN(p...)  _LOG_(LOG_WARN_LEVEL, __FILE__, __LINE__, __FUNCTION__, p)
#else
  #define LOG_WARN(p...)
#endif

//ERROR
#if LOG_LEVEL <= LOG_ERROR_LEVEL
  #define LOG_ERROR(p...)  _LOG_(LOG_ERROR_LEVEL, __FILE__, __LINE__, __FUNCTION__, p)
#else
  #define LOG_ERROR(p...)
#endif

#endif	/* LOG_H */

