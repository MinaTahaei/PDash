#ifndef UT_SHAREBASESAFE_H
#define UT_SHAREBASESAFE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class UT_ShareBaseSafe
{
public:
  UT_ShareBaseSafe();
  virtual ~UT_ShareBaseSafe();

  //! increment reference and return the original value.
  unsigned IncRef() const;

  //! decrement the reference, return the value after the operation.
  unsigned DecRef() const;

  unsigned GetRefCount() const;

  // avoid compilation error, for now!!!! make it do nothing.
  UT_ShareBaseSafe & operator=( const UT_ShareBaseSafe & ) { return *this; };
private:
  class Impl;
  Impl *_i;

  UT_ShareBaseSafe( const UT_ShareBaseSafe & );	//	no copy
};

#endif

