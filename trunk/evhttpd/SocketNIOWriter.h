/* 
 * File:   SocketNIOWriter.h
 * Author: dell
 *
 * Created on 2012年3月17日, 下午2:35
 */

#ifndef SOCKETNIOWRITER_H
#define	SOCKETNIOWRITER_H

#include "EvNIOWriter.h"

class SocketNIOWriter : public EvNIOWriter{
public:
    SocketNIOWriter();
    SocketNIOWriter(FD fd);
    virtual ~SocketNIOWriter();
    
  void setFlags(int flags){
      this->flags = flags;
  }
  
  int getFlags(){
      return flags;
  }    
  
protected:
    /**
     * 真正写操作
     */
    virtual ssize_t realWrite(const char* buff, size_t n, off_t offset = -1);
    
private:
    //send操作的flags参数, 默认为:0
    int flags;
};

#endif	/* SOCKETNIOWRITER_H */

