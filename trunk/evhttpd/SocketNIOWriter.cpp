/* 
 * File:   SocketNIOWriter.cpp
 * Author: dell
 * 
 * Created on 2012年3月17日, 下午2:35
 */

#include "SocketNIOWriter.h"

SocketNIOWriter::SocketNIOWriter():EvNIOWriter(),flags(0) {
}

SocketNIOWriter::SocketNIOWriter(FD fd):EvNIOWriter(fd),flags(0){
}

SocketNIOWriter::~SocketNIOWriter() {
}

ssize_t SocketNIOWriter::realWrite(const char* buff, size_t n, off_t offset){
    return ::send(fd, buff, n, flags);
}



