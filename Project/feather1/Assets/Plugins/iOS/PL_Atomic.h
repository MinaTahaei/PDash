
#ifndef _PL_ATOMIC_H_
#define _PL_ATOMIC_H_

#include "PL_Configure.h"

#ifdef SUPPORT_CXX11_STANDARD

#include <atomic>

//template <typename T>
//using PL_Atomic = std::atomic<T>;
//typedef PL_Atomic<int32_t> PL_AtomicInt32;

typedef std::atomic<int32_t> PL_AtomicInt32;

#else // !defined(SUPPORT_CXX11_STANDARD)

#if defined(TARGET_WIN32)
#include <cstdint>
#else
#include <sys/types.h>
#endif

class PL_AtomicInt32
{
public:

  PL_AtomicInt32();
  PL_AtomicInt32(int32_t);
  ~PL_AtomicInt32();

  int32_t operator++();
  int32_t operator++(int);
  int32_t operator--();
  int32_t operator--(int);

  int32_t operator+=(int32_t);
  int32_t operator-=(int32_t);

  //  non thread-safe.
  int32_t operator=(int32_t);
  operator int32_t() const;

private:

  /* not implemented */
  int32_t operator=( const PL_AtomicInt32 & );

private:

  class Impl;
  Impl *_i;
};

#endif

#endif /* _PL_ATOMIC_H_ */
