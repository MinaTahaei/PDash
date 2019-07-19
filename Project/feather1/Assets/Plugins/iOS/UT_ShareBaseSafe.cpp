
#include "UT_ShareBaseSafe.h"
#include "PL_Atomic.h"

class UT_ShareBaseSafe::Impl
{
public:
  mutable PL_AtomicInt32 _nRef;
};

UT_ShareBaseSafe::UT_ShareBaseSafe()
: _i( new Impl() )
{
  _i->_nRef = 0u;
}


UT_ShareBaseSafe::~UT_ShareBaseSafe()
{
  delete _i;
}


unsigned UT_ShareBaseSafe::IncRef() const
{
  return _i->_nRef++;
}

unsigned UT_ShareBaseSafe::DecRef() const
{
  return (--_i->_nRef);
}

unsigned UT_ShareBaseSafe::GetRefCount() const
{
  return _i->_nRef;
}
