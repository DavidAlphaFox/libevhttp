/* 
 * File:   SocketNIOReader.h
 * Author: dell
 *
 * Created on 2012年3月15日, 下午11:50
 */

#ifndef SOCKETNIOREADER_H
#define	SOCKETNIOREADER_H

#include "EvNIOReader.h"

class SocketNIOReader : public EvNIOReader{
public:
  SocketNIOReader();
  SocketNIOReader(FD fd);
  virtual ~SocketNIOReader();
  
  void setFlags(int flags){
      this->flags = flags;
  }
  
  int getFlags(){
      return flags;
  }
  
protected:
    /**
     * 真正读操作
     */
    virtual ssize_t realRead(char* buff, size_t n, off_t offset = -1);      
    
    //向EV分发读事件
    virtual void feedReadEvio();    
    
private:
    //recv操作的flags参数, 默认为:0
    int flags;
};

#endif	/* SOCKETNIOREADER_H */

