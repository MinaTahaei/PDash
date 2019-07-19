//========== Copyright ToonBoom Technologies Inc. 2000 ============
//
// SOURCE FILE : UT_SharedPtr.h 
// MODULE NAME : Utilities
// DESCRIPTION : like smartPointers.
//					  This class will keep a refcount of T* an will
//					  delete it when no more reference point on it. The 
//					  class T must derive from UT_ShareBase in order to
//					  to work properly with this class. Doing so will also
//					  make sure that your T* will be restore correctly
//					  in all UT_SharePtr instance.
//
//=================================================================
// Author : JF Couture
// Modif  : 
//
// Creation Date		 : 2000-01-01
//========================VSS Auto=================================
// $Revision: 1.5.4.1.2.2 $
// $Date: 2007/08/03 15:40:11 $
// $Author: begin $
//=================================================================
// REVISION: 
// 
//=========== Copyright ToonBoom Technologies Inc. 2000  ==========
#if !defined(AFX_UT_SHAREDPTR_H__67C08503_C080_11D4_B76A_00A0833723F7__INCLUDED_)
#define AFX_UT_SHAREDPTR_H__67C08503_C080_11D4_B76A_00A0833723F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <assert.h>
#include <cstdlib>

#include "MEM_Override.h"

/*!
This template is a template class that implement the smart pointer paragdigm. The actual reference 
counting object must be provided by the T class.

The T class must implement 3 methods (IncRef,DecRef,GetRefCount ) needed for this smart pointer
to work.  To simplify development, the class may derived from UT_ShareBase to provide an automatic
implement of these 3 methods.

This class is a lightweight smart pointer as no other inner object need to be allocated. The smart pointer
object consist only of a single pointer, and the pointed class has an extra overhead of the reference counting, without
an extra memory allocation.

This class is preferable to UT_SmartPointer in most situation where we control the specification of class T.

This class is also streamable by DatabaseLib, which will correctly manage the registration and the the restoring the 
pointed object.

THREAD SAFE : this class is thread safe as long as IncRef and DecRef are thread safe.

*/
template< class T >
class UT_SharedPtr
{
  MEM_OVERRIDE

public:
  UT_SharedPtr();
  UT_SharedPtr( T* ptr );
  UT_SharedPtr( const UT_SharedPtr& rhs );
  ~UT_SharedPtr();

  UT_SharedPtr& operator=( const UT_SharedPtr& rhs );
  UT_SharedPtr& operator=( T* ptr );
  T* operator->()const;
  T* GetPtr()const;
  T* get() const;
  bool isValid() const;
  T& operator*()const;

  bool operator==( const UT_SharedPtr& rhs )const;
  bool operator!=( const UT_SharedPtr& rhs )const;
  bool operator<( const UT_SharedPtr& rhs )const;
  bool operator>( const UT_SharedPtr& rhs )const;

  operator bool () const { return m_ptr != 0; }
private:
  T* m_ptr;
};

template< class T >
UT_SharedPtr<T>::UT_SharedPtr() :
m_ptr(0)
{
}

template< class T >
UT_SharedPtr<T>::UT_SharedPtr( T* ptr )
{
  m_ptr = ptr;
  if( m_ptr )
    (void)m_ptr->IncRef();
}

template< class T >
UT_SharedPtr<T>::UT_SharedPtr( const UT_SharedPtr<T>& rhs )
{
  m_ptr = rhs.m_ptr;
  if( m_ptr )
    (void)m_ptr->IncRef();
}

template< class T >
UT_SharedPtr<T>::~UT_SharedPtr()
{
  //Remove one ref or delete if no more
  if( m_ptr && m_ptr->DecRef() == 0 )
  {
    delete m_ptr;
  }
}
template< class T >
inline UT_SharedPtr<T>& UT_SharedPtr<T>::operator=( T* ptr )
{
  T *pOld = m_ptr;
  m_ptr = ptr;
  if( m_ptr )
    m_ptr->IncRef();
  if( pOld && pOld->DecRef() == 0 )
    delete pOld;
  return *this;
}

template< class T >
inline UT_SharedPtr<T>& UT_SharedPtr<T>::operator=( const UT_SharedPtr<T>& rhs )
{
  T *pOld = m_ptr;
  m_ptr = rhs.m_ptr;
  if( m_ptr )
    m_ptr->IncRef();
  if( pOld && pOld->DecRef() == 0 )
    delete pOld;
  return *this;
}

template< class T >
inline T* UT_SharedPtr<T>::operator->() const
{
  //You are gonna crash
  assert(m_ptr);
  return m_ptr;
}

template< class T >
inline T* UT_SharedPtr<T>::GetPtr() const
{
  return m_ptr;
}

template< class T >
inline T* UT_SharedPtr<T>::get() const
{
  return m_ptr;
}

template< class T >
inline bool UT_SharedPtr<T>::isValid() const
{
  return m_ptr!=0;
}

template< class T >
T& UT_SharedPtr<T>::operator*()const
{
  //You are gonna crash
  assert(m_ptr);
  return *m_ptr;
}

template< class T >
bool UT_SharedPtr<T>::operator==( const UT_SharedPtr& rhs )const
{
  return m_ptr == rhs.m_ptr;
}

template< class T >
bool UT_SharedPtr<T>::operator!=( const UT_SharedPtr& rhs )const
{
  return m_ptr != rhs.m_ptr;
}

template< class T >
bool UT_SharedPtr<T>::operator<( const UT_SharedPtr& rhs )const
{
  return m_ptr < rhs.m_ptr;
}

template< class T >
bool UT_SharedPtr<T>::operator>( const UT_SharedPtr& rhs )const
{
  return m_ptr > rhs.m_ptr;
}

#endif // !defined(AFX_UT_SHAREDPTR_H__67C08503_C080_11D4_B76A_00A0833723F7__INCLUDED_)
