/* 
 * File:   SocketUtils.h
 * Author: try
 *
 * Created on 2011年5月28日, 下午10:37
 */

#ifndef _SOCKETUTILS_H
#define	_SOCKETUTILS_H
#include "resources.h"

/**
 * Socket相关工具方法
 */
class SocketUtils {
public:
    /** 设置为非阻塞模式 */
    static int setNonblock(FD fd);

    static bool setReuseaddr(FD fd);

    static bool setNodelay(FD fd);
    
    static bool setSocketOpt(FD fd, int level, int opt, int value);
    
private:
    SocketUtils();
    virtual ~SocketUtils();
};

#endif	/* _SOCKETUTILS_H */

