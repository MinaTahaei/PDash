#ifndef _TR_NODE_TREE_H_
#define _TR_NODE_TREE_H_

#include "TR_DataObject.h"

#include "MEM_Override.h"
#include "STD_Containers.h"
#include "STD_Types.h"

#include <set>
#include <string>

class IO_PersistentStore;

/*!
 * @class TR_NodeTree
 * Node Tree data structure.
 */
class TR_NodeTree
{
  MEM_OVERRIDE

  friend class TR_NodeTreeBuilder;
  friend class TV_NodeTreeView;

public:

  typedef unsigned char                 Data_t;
  typedef STD_Vector<Data_t >           DataBuffer_t;

  typedef STD_Set<TR_Types::DataRef_t>  DataRefCol_t;

  /*!
   * @class NodeIterator
   * Iterator on the node tree hierarchy.
   */
  class NodeIterator
  {
    MEM_OVERRIDE

    friend class TR_NodeTree;
  public:
    NodeIterator();
    NodeIterator(TR_NodeDataObject *nodeData);
    ~NodeIterator();

    NodeIterator &operator++();
    NodeIterator operator++(int);

    bool operator==(const NodeIterator &it) const;
    bool operator!=(const NodeIterator &it) const;

    const TR_NodeDataObject &operator*() const;
    TR_NodeDataObject &operator*();

    const TR_NodeDataObject *operator->() const;
    TR_NodeDataObject *operator->();

  private:

    struct IterData
    {
      MEM_OVERRIDE

      IterData(TR_NodeDataObject *data) :
        _data(data),
        _visitedBrother(false),
        _visitedChild(false),
        _visitedMatte(false)
      {
      }

      bool operator==(const IterData &iterData) const
      {
        return (_data == iterData._data);
      }

      TR_NodeDataObject *_data;
      bool               _visitedBrother;
      bool               _visitedChild;
      bool               _visitedMatte;
    };

    typedef STD_Vector<IterData> NodeDataStack_t;
    NodeDataStack_t _nodeDataStack;
  };

public:

  TR_NodeTree();
  virtual ~TR_NodeTree();

  //! Retrieve first iterator on a TR_NodeDataObject.  
  NodeIterator nodeBegin() const;
  //! Retrieve last iterator on a TR_NodeDataObject.
  NodeIterator nodeEnd() const;

  //! Build a list of reference offsets to TR_NodeDataObject objects with specified name.  
  void nodeDataRefs(const STD_String &name, DataRefCol_t &dataRefs ) const;
  //! Retrieve first reference offset to TR_NodeDataObject object with specified name.
  TR_Types::DataRef_t nodeDataRef(const STD_String &name) const;

  //! Retrieve reference offset to first TR_NodeDataObject object.
  TR_Types::DataRef_t firstNodeDataRef() const;

  //! Retrieve reference offset to a TR_DataObject object that is child to another one. 
  TR_Types::DataRef_t dataRef(const TR_DataObject *dataObject, TR_Types::DataOffset_t offset) const;
  //! Retrieve reference offset to specified TR_DataObject object.
  TR_Types::DataRef_t dataRef(const TR_DataObject *dataObject) const;

  //! Retrieve an abstract TR_DataObject object from a reference.
  const TR_DataObject *dataObject(TR_Types::DataRef_t dataRef) const;

  //! Retrieve a concrete TR_DataObject object from a reference.
  template<typename T>
  const T             *dataObject(TR_Types::DataRef_t dataRef) const;

protected:

  TR_DataObject *dataObject(TR_Types::DataRef_t dataRef);

  template<typename T>
  T             *dataObject(TR_Types::DataRef_t dataRef);

  bool queryNodeName( const TR_NodeDataObject *nodeData, STD_String &name ) const;
  bool querySpriteName( const TR_DrawingDataObject *drawingData, STD_String &name ) const;

  const STD_String &spriteSheetName() const;

  void setSpriteSheetName( const STD_String &spriteSheetName );

  template<typename T>
  T* addDataObject();

  TR_StringDataObject *addStringDataObject( const STD_String &name );

public:

  //! Store node tree data to stream.
  void store( IO_PersistentStore &store ) const;
  //! Load node tree data from stream.
  void load( IO_PersistentStore &store );

  //! Dump node tree data main entries to stdout.
  void dump() const;

private:

  TR_Types::DataRef_t _firstNodeDataRef;
  STD_String          _spriteSheetName;
  DataBuffer_t        _data;
};

enum { BUFFER_CHUNK_SIZE = 512};

template<typename T>
T *TR_NodeTree::dataObject(TR_Types::DataRef_t dataRef)
{
  TR_DataObject *data = dataObject(dataRef);

  if ( !data || (data->_id != T::Id()) )
    return 0;

  return static_cast<T*>(data);
}

template<typename T>
const T *TR_NodeTree::dataObject(TR_Types::DataRef_t dataRef) const
{
  const TR_DataObject *data = dataObject(dataRef);

  if ( !data || (data->_id != T::Id()) )
    return 0;

  return static_cast<const T*>(data);
}

template<typename T>
T* TR_NodeTree::addDataObject()
{
  if ( _data.size() + sizeof(T) >= _data.capacity() )
  {
    //  Double capacity when overflowing current buffer
    if ( _data.capacity() )
    {
      _data.reserve(_data.capacity() << 1);
    }
    else
    {
      _data.reserve( BUFFER_CHUNK_SIZE );
    }
  }

  _data.resize( _data.size() + sizeof(T) );
  T *t = reinterpret_cast<T*>( &_data.rbegin()[sizeof(T)-1] );
  t = new (t) T;

  return t;
}

#endif /* _TR_NODE_TREE_H_ */
