/* 
 * File:   SocketNIOReader.cpp
 * Author: dell
 * 
 * Created on 2012年3月15日, 下午11:50
 */

#include "SocketNIOReader.h"

SocketNIOReader::SocketNIOReader():EvNIOReader(),flags(0) {
}

SocketNIOReader::SocketNIOReader(FD fd):EvNIOReader(fd),flags(0) {
}

SocketNIOReader::~SocketNIOReader() {

}


ssize_t SocketNIOReader::realRead(char* buff, size_t n, off_t offset){
    LOG_DEBUG("fd is %d", fd);
    ssize_t rsize = ::recv(fd, buff, n, flags);
    LOG_DEBUG("rsize is %d", rsize);
    return rsize;
}


void SocketNIOReader::feedReadEvio(){
    LOG_DEBUG("fd is %d", fd);
    if(isClosed())return;
    if(!readevio.is_active()){
        readevio.start();
    }
}
