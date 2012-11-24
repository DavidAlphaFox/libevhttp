/* 
 * File:   IRunnable.h
 * Author: try
 *
 * Created on 2011年5月27日, 上午10:19
 */

#ifndef _IRUNNABLE_H
#define	_IRUNNABLE_H

#include <stdlib.h>

  /**
   * IRunnable 接口应该由那些打算通过某一线程或进程执行其实例的类来实现。
   */
  class IRunnable {
    public:
      IRunnable() {
      }

      virtual ~IRunnable() {
      }

      /**
       * 使用实现接口 IRunnable 的对象创建一个线程或进程时，
       * 启动该线程或进程将导致在独立执行的线程或进程中调用对象的 run 方法。
       *
       * @param params 初始参数
       */
      virtual int run(void* param=NULL)=0;

  };

#endif	/* _IRUNNABLE_H */

