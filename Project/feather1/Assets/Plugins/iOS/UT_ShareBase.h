
/*
 * Copyright Toon Boom Animation Inc. 
 * This file is part of the Toon Boom Animation SDK
 */

#if !defined(_UT_SHAREBASE_H_)
#define _UT_SHAREBASE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MEM_Override.h"

/*!
 * Base class for ref counted objects.
 * It can be used in collaboration with UT_SharedPtr
 */
class UT_ShareBase
{
  MEM_OVERRIDE

public:
  UT_ShareBase() : m_nRef(0) {}
  virtual ~UT_ShareBase() {};

  /*! Increments the ref count */
  unsigned IncRef() const { return m_nRef++; }

  /*! Decrements the ref count */
  unsigned DecRef() const { return (--m_nRef); }

  /*! Returns the ref count */
  unsigned GetRefCount()const { return m_nRef; }

  UT_ShareBase&  operator=( const UT_ShareBase & ) { return *this; }

private:
  mutable unsigned m_nRef;

  /*! Making this constructor private prevents from
   *  copying refcounted objects.  It guarantees that
   *  copy will be done only by pointer
   */
  UT_ShareBase( const UT_ShareBase & ); //	no copy
};

#endif // !defined(_UT_SHAREBASE_H_)
