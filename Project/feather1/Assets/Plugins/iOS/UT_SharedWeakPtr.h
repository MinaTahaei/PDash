#ifndef _UT_SHARED_WEAK_PTR_HEADER_
#define _UT_SHARED_WEAK_PTR_HEADER_

#include "UT_SharedPtr.h"
#include "UT_ShareBaseSafe.h"

/*
Example:
class Toto : public UT_SharedWeakBase
{
}
Toto* toto(new Toto());

typedef UT_SharedWeakPtr<Toto> TotoWeakPtr_t;
TotoWeakPtr_t totoWeakPtr(toto);

printf("toto valid: %d\n", totoWeakPtr.isValid());
delete toto;
printf("toto valid: %d\n", totoWeakPtr.isValid());
*/


class UT_SharedWeakBase;

/*!
 * Holder of the weak ref. This object is shared
 * among all the weak references to a particular object.
 * This object is internal to the UT_SharedWeakPtr and cannot
 * be manipulated directly. The members are public because
 * the friend directive cannnot be called for templates
 * e.g. in this example, we would need:
 * friend class UT_SharedWeakPtr<Type>; 
 * the compiler would barf on that...
 */
class UT_SharedWeakPtrHolder : public UT_ShareBaseSafe
{
public:
  UT_SharedWeakPtrHolder(UT_SharedWeakBase *ptr);
  virtual ~UT_SharedWeakPtrHolder();

  UT_SharedWeakBase *_ptr;
};

/*!
 * The user of a weak reference to an object has to call
 * get() or isValid() and check the return value before
 * accessing the pointer. through the -> operator
 */
class UT_SharedWeakPtrBase
{
public:
  UT_SharedWeakPtrBase();
  UT_SharedWeakPtrBase(const UT_SharedWeakPtrBase& ptr);

protected:
  void assign(UT_SharedWeakBase* ptr);

  typedef UT_SharedPtr<UT_SharedWeakPtrHolder> SharedWeakPtrHolder_t;
  SharedWeakPtrHolder_t _holder;
};


/*!
 * Add one level of abstraction to perform a friend
 * class implementation. The class UT_SharedWeakBase
 * will have a refence to this class but will export only
 * the base class. The UT_SharedWeakBase will thus
 * have access to the protected members of UT_SharedWeakPtr
 * through the UT_SharedWeakPtrImpl class
 */
class UT_SharedWeakPtrImpl : public UT_SharedWeakPtrBase
{
public:
  UT_SharedWeakPtrImpl();
  virtual ~UT_SharedWeakPtrImpl();

  void setPtr(UT_SharedWeakBase *ptr);
};


/*!
 * Classes for which we need weak pointers must derive from this class
 */
class UT_SharedWeakBase : public UT_ShareBaseSafe
{
public:
  UT_SharedWeakBase();
  virtual ~UT_SharedWeakBase();

  /*!
   * Invalidates all weakPtr to this object.
   * This can be called if an object has not been deleted
   * but the rest of the application should consider it as deleted
   * e.g. the object was put in the undo queue. 
   */
  void disconnectWeakPtrs();

  /*!
   * Make all the weakPtr to this object valid once again.
   * This can be called after a disconnectWeakPtr() was done.
   * e.g. An operation that put an object in the undo queue was undone.
   */
  void reconnectWeakPtrs();

  /*!
   * Returns a weak reference on this object. Internal use only.
   */
  const UT_SharedWeakPtrBase& weakPtr() const;

private:
  UT_SharedWeakPtrImpl _weakPtr;
};


/*!
 * The user of a weak reference to an object has to call
 * get() or isValid() and check the return value before
 * accessing the pointer. through the -> operator
 */
template <class Type>
class UT_SharedWeakPtr : UT_SharedWeakPtrBase
{
public:

  UT_SharedWeakPtr()
  {
  }

  UT_SharedWeakPtr( const UT_SharedWeakPtr& ptr )
  : UT_SharedWeakPtrBase(ptr)
  {
  }

  UT_SharedWeakPtr(Type* ptr)
  {
    assign(static_cast<UT_SharedWeakBase*>(ptr));
  }

  explicit UT_SharedWeakPtr(const UT_SharedPtr<Type>& ptr)
  {
    assign(static_cast<UT_SharedWeakBase*>(ptr.get()));
  }

  UT_SharedWeakPtr& operator=( Type* rhs )
  {
    assign(static_cast<UT_SharedWeakBase*>(rhs));
    return *this;
  }

  UT_SharedWeakPtr& operator=( const UT_SharedWeakPtr& rhs )
  {
    _holder = rhs._holder;
    return *this;
  }

  UT_SharedWeakPtr& operator=( const UT_SharedPtr<Type>& rhs )
  {
    assign(static_cast<UT_SharedWeakBase*>(rhs.get()));
    return *this;
  }

  /*! Returns true if this weak ptr is a null pointer, false otherwise. 
      At any time, a weak pointer is in one and only one of the following three states:
        -null
        -disconnected 
        -valid
   */
  bool isNull() const { return _holder.get() == 0; }

  /*! Returns true if this weak ptr is a disconnected non-null pointer, false otherwise. 
      At any time, a weak pointer is in one and only one of the following three states:
        -null
        -disconnected 
        -valid
   */
  bool isDisconnected() const { return _holder.get() ? _holder->_ptr == 0 : false; }

  /*! Returns true if this weak ptr is a valid non-null pointer, false otherwise.
      At any time, a weak pointer is in one and only one of the following three states:
        -null
        -disconnected 
        -valid
   */
  bool isValid() const { return _holder.get() ? _holder->_ptr != 0 : false; }

  Type* get() const { return _holder.get() ? static_cast<Type*>(_holder->_ptr) : 0; }

  Type* operator->() const { return _holder.get() ? static_cast<Type*>(_holder->_ptr) : 0; }

  UT_SharedPtr<Type> strongPtr() const
  {
    return UT_SharedPtr<Type>(get());
  }

  bool operator==(const UT_SharedWeakPtr& rhs) const
  {
    return this == &rhs || _holder.get() == rhs._holder.get();
  }

  bool operator!=(const UT_SharedWeakPtr& rhs) const
  {
    return !(*this == rhs);
  }

  bool operator<(const UT_SharedWeakPtr& rhs) const
  {
    return _holder.get() < rhs._holder.get();
  }

  bool operator>(const UT_SharedWeakPtr& rhs) const
  {
    return _holder.get() > rhs._holder.get();
  }
};

inline void UT_SharedWeakBase::disconnectWeakPtrs()
{
  _weakPtr.setPtr(0);
}

inline void UT_SharedWeakBase::reconnectWeakPtrs()
{
  _weakPtr.setPtr(this);
}

inline const UT_SharedWeakPtrBase& UT_SharedWeakBase::weakPtr() const
{
  return _weakPtr;
}

inline void UT_SharedWeakPtrBase::assign(UT_SharedWeakBase* ptr)
{
  _holder = (ptr != 0 ? ptr->weakPtr()._holder : SharedWeakPtrHolder_t());
}


#endif // _UT_SAFE_WEAK_REF_HEADER_
