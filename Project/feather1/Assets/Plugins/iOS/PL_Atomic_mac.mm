
#include "PL_Atomic.h"

#if !defined(SUPPORT_CXX11_STANDARD) && (defined(TARGET_IOS) || defined(TARGET_MAC))

#include <libkern/OSAtomic.h>

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
  int32_t _value;
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
  return OSAtomicIncrement32(&_i->_value);
}

int32_t PL_AtomicInt32::operator++(int)
{
  int32_t ret = OSAtomicIncrement32(&_i->_value);
  return (ret-1);
}

int32_t PL_AtomicInt32::operator--()
{
  return OSAtomicDecrement32(&_i->_value);
}

int32_t PL_AtomicInt32::operator--(int)
{
  int32_t ret = OSAtomicDecrement32(&_i->_value);
  return (ret+1);
}

int32_t PL_AtomicInt32::operator+=(int32_t value)
{
  int32_t ret = OSAtomicAdd32(value, &_i->_value);
  return ret;
}

int32_t PL_AtomicInt32::operator-=(int32_t value)
{
  int32_t ret = OSAtomicAdd32(-value, &_i->_value);
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
