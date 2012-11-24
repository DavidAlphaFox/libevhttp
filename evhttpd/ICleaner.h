/*
 * ICleaner.h
 *
 *  Created on: 2010-4-18
 *      Author: try
 */

#ifndef ICLEANER_H_
#define ICLEANER_H_

  /**
   * 实现此接口，便于进行资源清理，将实现了些接口的类型实例加入到CleanerTimer中，
   * 将会定时调用cleanup()方法。
   */
  class ICleaner {
    public:
      ICleaner(){}
      virtual ~ICleaner(){}

      /**
       * 通过实现此方法进行资源清理工作
       */
      virtual void cleanup()=0;
  };

#endif /* ICLEANER_H_ */
