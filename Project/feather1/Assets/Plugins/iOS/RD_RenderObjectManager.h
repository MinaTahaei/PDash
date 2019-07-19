
#ifndef _RD_RENDER_SCRIPT_MANAGER_H_
#define _RD_RENDER_SCRIPT_MANAGER_H_

#include "UT_SharedPtr.h"
#include "PL_Mutex.h"
#include "STD_Containers.h"

/*! 
 * @class RD_RenderObjectManager
 * Thread-safe key-value container.
 */
template <typename K, typename T>
class RD_RenderObjectManager
{
public:

  static RD_RenderObjectManager *instance();
  virtual ~RD_RenderObjectManager();

  //! Add new object to container.
  void            addObject( const K &key, const UT_SharedPtr<T> &obj );
  //! Remove object from container.
  UT_SharedPtr<T> eraseObject( const K &key );
  //! Remove all objects from container.
  void            eraseAll();

  //! Remove first object from container.
  UT_SharedPtr<T> popFirst();
  //! Remove last object from container.
  UT_SharedPtr<T> popLast();

  //! Retrieve object with specified key.
  UT_SharedPtr<T> object( const K &key ) const;
  //! Retrieve object at index.
  UT_SharedPtr<T> objectAt( unsigned idx ) const;
  //! Validate that object with specified key exists in container.
  bool            objectExists( const K &key ) const;

  //! Retrieve container size.
  size_t          size() const;

private:

  RD_RenderObjectManager();

private:

  typedef STD_Map<K, UT_SharedPtr<T> > ObjectCol_t;
  ObjectCol_t _objects;

  mutable PL_Mutex _mutex;
};

template <typename K, typename T>
RD_RenderObjectManager<K,T> *RD_RenderObjectManager<K,T>::instance()
{
  static RD_RenderObjectManager<K,T> *g_instance = 0;

  if ( g_instance == 0 )
  {
    g_instance = new RD_RenderObjectManager<K,T>();
  }

  return g_instance;
}

template <typename K, typename T>
RD_RenderObjectManager<K,T>::RD_RenderObjectManager()
{
}

template <typename K, typename T>
RD_RenderObjectManager<K,T>::~RD_RenderObjectManager()
{
}

template <typename K, typename T>
inline void RD_RenderObjectManager<K,T>::addObject( const K &key, const UT_SharedPtr<T> &obj )
{
  _mutex.lock();

  if (_objects.find( key ) == _objects.end())
    _objects.insert( STD_MakePair(key, obj) );

  _mutex.unlock();
}

template <typename K, typename T>
inline UT_SharedPtr<T> RD_RenderObjectManager<K,T>::eraseObject( const K &key )
{
  UT_SharedPtr<T> pRet;

  _mutex.lock();

  typename ObjectCol_t::iterator iObject = _objects.find( key );
  if ( iObject != _objects.end() )
  {
    pRet = iObject->second;
    _objects.erase(iObject);
  }

  _mutex.unlock();

  return pRet;
}

template <typename K, typename T>
inline void RD_RenderObjectManager<K,T>::eraseAll()
{
  _mutex.lock();
  _objects.clear();
  _mutex.unlock();
}

template <typename K, typename T>
inline UT_SharedPtr<T> RD_RenderObjectManager<K,T>::popFirst()
{
  UT_SharedPtr<T> pRet;

  _mutex.lock();

  if ( !_objects.empty() )
  {
    typename ObjectCol_t::iterator iObject = _objects.begin();
    pRet = iObject->second;
    _objects.erase(iObject);
  }

  _mutex.unlock();

  return pRet;
}

template <typename K, typename T>
inline UT_SharedPtr<T> RD_RenderObjectManager<K,T>::popLast()
{
  UT_SharedPtr<T> pRet;

  _mutex.lock();

  if ( !_objects.empty() )
  {
    typename ObjectCol_t::reverse_iterator iObject = _objects.rbegin();
    pRet = iObject->second;
    _objects.erase(iObject);
  }

  _mutex.unlock();

  return pRet;
}

template <typename K, typename T>
inline UT_SharedPtr<T> RD_RenderObjectManager<K,T>::object( const K &key ) const
{
  UT_SharedPtr<T> pRet;

  _mutex.lock();

  typename ObjectCol_t::const_iterator iObject = _objects.find( key );
  if ( iObject != _objects.end() )
  {
    pRet = iObject->second;
  }

  _mutex.unlock();

  return pRet;
}

template <typename K, typename T>
inline UT_SharedPtr<T> RD_RenderObjectManager<K,T>::objectAt( unsigned idx ) const
{
  UT_SharedPtr<T> pRet;

  _mutex.lock();

  if ( idx < _objects.size() )
  {
    typename ObjectCol_t::const_iterator iObject = _objects.begin();
    STD_Advance(iObject, idx);

    if ( iObject != _objects.end() )
    {
      pRet = iObject->second;
    }
  }

  _mutex.unlock();

  return pRet;
}

template <typename K, typename T>
inline bool RD_RenderObjectManager<K,T>::objectExists( const K &key ) const
{
  bool ret = false;

  _mutex.lock();

  typename ObjectCol_t::const_iterator iObject = _objects.find( key );
  ret = ( iObject != _objects.end() );

  _mutex.unlock();

  //  There is no guarantee that object has not been deleted between moment
  //  where flag was set and will be tested.
  return ret;
}

template <typename K, typename T>
inline size_t RD_RenderObjectManager<K,T>::size() const
{
  size_t arraySize = 0;

  _mutex.lock();
  arraySize = _objects.size();
  _mutex.unlock();

  //  Retrieved value is not thread safe in itself, and thus, size might
  //  change between moment where value is retrieved and used.
  return arraySize;
}

#endif /* _RD_RENDER_SCRIPT_MANAGER_H_ */
