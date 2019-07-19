
#include "PL_Atomic.h"
#include "PL_Mutex.h"

#if !defined(SUPPORT_CXX11_STANDARD) && (defined(TARGET_LINUX) || defined(TARGET_ANDROID) || defined(TARGET_WEBGL))

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - PL_AtomicInt32::Impl
#endif
class PL_AtomicInt32::Impl
{
  friend class PL_AtomicInt32;

public:

  Impl()
  {
  }

  ~Impl()
  {
  }

private:
  int32_t   _value;

  // not an efficient way of implementing atomic, but could not find an other way
  // on unix systems.  Use c++11 instead if you want full atomic support.
  PL_Mutex  _mutex;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - PL_AtomicInt32
#endif

PL_AtomicInt32::PL_AtomicInt32()
{
  _i = new Impl;
  _i->_value = 0;
}

PL_AtomicInt32::PL_AtomicInt32(int32_t value)
{
  _i = new Impl;
  _i->_value = value;
}

PL_AtomicInt32::~PL_AtomicInt32()
{
  delete _i;
}

int32_t PL_AtomicInt32::operator++()
{
  _i->_mutex.lock();
  int32_t ret = ++_i->_value;
  _i->_mutex.unlock();

  return ret;
}

int32_t PL_AtomicInt32::operator++(int)
{
  _i->_mutex.lock();
  int32_t ret = _i->_value++;
  _i->_mutex.unlock();

  return ret;
}

int32_t PL_AtomicInt32::operator--()
{
  _i->_mutex.lock();
  int32_t ret = --_i->_value;
  _i->_mutex.unlock();

  return ret;
}

int32_t PL_AtomicInt32::operator--(int)
{
  _i->_mutex.lock();
  int32_t ret = _i->_value--;
  _i->_mutex.unlock();

  return ret;
}

int32_t PL_AtomicInt32::operator+=(int32_t value)
{
  _i->_mutex.lock();
  _i->_value += value;
  int32_t ret = _i->_value;
  _i->_mutex.unlock();

  return ret;
}

int32_t PL_AtomicInt32::operator-=(int32_t value)
{
  _i->_mutex.lock();
  _i->_value -= value;
  int32_t ret = _i->_value;
  _i->_mutex.unlock();

  return ret;
}

int32_t PL_AtomicInt32::operator=(int32_t value)
{
  _i->_value = value;
  return _i->_value;
}

PL_AtomicInt32::operator int32_t() const
{
  return _i->_value;
}

#endif
