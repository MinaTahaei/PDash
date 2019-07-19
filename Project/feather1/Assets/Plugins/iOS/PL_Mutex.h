
#ifndef _PL_MUTEX_H_
#define _PL_MUTEX_H_

#include "PL_Configure.h"

#ifdef SUPPORT_CXX11_STANDARD

#include <mutex>
typedef std::mutex PL_Mutex;

#else  // !defined(SUPPORT_CXX11_STANDARD)

class PL_Mutex
{
public:

  PL_Mutex();
  ~PL_Mutex();

  void lock();
  bool try_lock();

  void unlock();

private:

  class Impl;
  Impl *_i;
};

#endif

#endif /* _PL_MUTEX_H_ */
