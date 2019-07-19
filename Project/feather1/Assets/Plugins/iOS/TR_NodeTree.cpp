#include "TR_NodeTree.h"
#include "TR_Utils.h"
#include "IO_PersistentStore.h"
#include "STD_Types.h"

#include <algorithm>
#include <string.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TR_NodeTree::Iterator
#endif
TR_NodeTree::NodeIterator::NodeIterator()
{
}

TR_NodeTree::NodeIterator::NodeIterator(TR_NodeDataObject *nodeData)
{
  IterData iterData(nodeData);
  _nodeDataStack.push_back(iterData);
}

TR_NodeTree::NodeIterator::~NodeIterator()
{
  _nodeDataStack.clear();
}

TR_NodeTree::NodeIterator &TR_NodeTree::NodeIterator::operator++()
{
  while(!_nodeDataStack.empty())
  {
    IterData &iterData = _nodeDataStack.back();
    TR_NodeDataObject *nodeData = iterData._data;

    //  Go to next child if exists
    if ( !iterData._visitedChild && (nodeData->_childDataOffset != TR_Types::g_nullOffset) )
    {
      iterData._visitedChild = true;

      TR_DataObject *nextData = reinterpret_cast<TR_DataObject*>(reinterpret_cast<Data_t*>(nodeData) + nodeData->_childDataOffset);

      //ASSERT(nextData->_type == eNodeData);
      TR_NodeDataObject *nextNodeData = static_cast<TR_NodeDataObject*>(nextData);

      IterData nextIterData(nextNodeData);
      _nodeDataStack.push_back(nextIterData);

      return *this;
    }

    //  Go to next matte if exists
    if ( !iterData._visitedMatte && (nodeData->_effectDataOffset != TR_Types::g_nullOffset) )
    {
      iterData._visitedMatte = true;

      TR_DataObject *data = reinterpret_cast<TR_DataObject*>(reinterpret_cast<Data_t*>(nodeData) + nodeData->_effectDataOffset);

      if (data->_id == TR_Types::eEffectData)
      {
        TR_EffectDataObject *effectData = static_cast<TR_EffectDataObject*>(data);

        if ( effectData->_matteDataOffset != TR_Types::g_nullOffset )
        {
          TR_DataObject *nextData = reinterpret_cast<TR_DataObject*>(reinterpret_cast<Data_t*>(effectData) + effectData->_matteDataOffset);

          //ASSERT(nextData->_type == eNodeData);
          TR_NodeDataObject *nextNodeData = static_cast<TR_NodeDataObject*>(nextData);

          IterData nextIterData(nextNodeData);
          _nodeDataStack.push_back(nextIterData);
          return *this;
        }
      }
    }

    //  Go to next brother if exists
    if ( !iterData._visitedBrother && (nodeData->_brotherDataOffset != TR_Types::g_nullOffset) )
    {
      iterData._visitedBrother = true;

      TR_DataObject *nextData = reinterpret_cast<TR_DataObject*>(reinterpret_cast<Data_t*>(nodeData) + nodeData->_brotherDataOffset);

      //ASSERT(nextData->_type == eNodeData);
      TR_NodeDataObject *nextNodeData = static_cast<TR_NodeDataObject*>(nextData);

      IterData nextIterData(nextNodeData);
      _nodeDataStack.push_back(nextIterData);

      return *this;
    }

    _nodeDataStack.pop_back();
  }

  return *this;
}

TR_NodeTree::NodeIterator TR_NodeTree::NodeIterator::operator++(int)
{
  NodeIterator iteratorCopy = *this;
  operator++();
  return iteratorCopy;
}

bool TR_NodeTree::NodeIterator::operator==(const NodeIterator &it) const
{
  if (_nodeDataStack.empty() != it._nodeDataStack.empty())
    return false;

  if (_nodeDataStack.empty() && it._nodeDataStack.empty())
    return true;

  return (_nodeDataStack.back() == it._nodeDataStack.back());
}

bool TR_NodeTree::NodeIterator::operator!=(const NodeIterator &it) const
{
  return !(this->operator==(it));
}

const TR_NodeDataObject &TR_NodeTree::NodeIterator::operator*() const
{
  //ASSERT(!_nodeDataStack.empty())
  return *_nodeDataStack.back()._data;
}

TR_NodeDataObject &TR_NodeTree::NodeIterator::operator*()
{
  //ASSERT(!_nodeDataStack.empty())
  return *_nodeDataStack.back()._data;
}

const TR_NodeDataObject *TR_NodeTree::NodeIterator::operator->() const
{
  //ASSERT(!_nodeDataStack.empty())
  return _nodeDataStack.back()._data;
}

TR_NodeDataObject *TR_NodeTree::NodeIterator::operator->()
{
  //ASSERT(!_nodeDataStack.empty())
  return _nodeDataStack.back()._data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TR_NodeTree
#endif

TR_NodeTree::TR_NodeTree()
{
  //  Add default root node to tree
  TR_NodeDataObject *dataObject = addDataObject<TR_NodeDataObject>();
  _firstNodeDataRef = dataRef(dataObject);
}

TR_NodeTree::~TR_NodeTree()
{
}

TR_NodeTree::NodeIterator TR_NodeTree::nodeBegin() const
{
  const TR_DataObject *data = reinterpret_cast<const TR_DataObject*>(&_data[0] + _firstNodeDataRef);
  if (data->_id != TR_Types::eNodeData)
    return NodeIterator();

  const TR_NodeDataObject *nodeData = static_cast<const TR_NodeDataObject*>(data);
  return NodeIterator(const_cast<TR_NodeDataObject*>(nodeData));
}

TR_NodeTree::NodeIterator TR_NodeTree::nodeEnd() const
{
  return NodeIterator();
}

void TR_NodeTree::nodeDataRefs(const STD_String &name, DataRefCol_t &dataRefs) const
{
  dataRefs.clear();

  for ( NodeIterator i = nodeBegin(), iEnd = nodeEnd() ; i!=iEnd ; ++i )
  {
    STD_String nodeName;
    if ( queryNodeName( &(*i), nodeName ) )
    {
      if ( nodeName.compare( name ) == 0 )
      {
        dataRefs.insert( dataRef(&(*i)) );
      }
    }
  }
}

TR_Types::DataRef_t TR_NodeTree::nodeDataRef(const STD_String &name) const
{
  DataRefCol_t dataRefs;
  nodeDataRefs(name, dataRefs);

  if ( !dataRefs.empty() )
  {
    return *dataRefs.begin();
  }

  return TR_Types::g_nullOffset;
}

TR_Types::DataRef_t TR_NodeTree::firstNodeDataRef() const
{
  return _firstNodeDataRef;
}

TR_Types::DataRef_t TR_NodeTree::dataRef(const TR_DataObject *dataObject, TR_Types::DataOffset_t offset) const
{
  return (TR_Types::DataRef_t)(reinterpret_cast<const Data_t*>(dataObject) - &_data[0] + offset);
}

TR_Types::DataRef_t TR_NodeTree::dataRef(const TR_DataObject *dataObject) const
{
  return (TR_Types::DataRef_t)(reinterpret_cast<const Data_t*>(dataObject) - &_data[0]);
}

const TR_DataObject *TR_NodeTree::dataObject(TR_Types::DataRef_t dataRef) const
{
  if ( (dataRef == TR_Types::g_nullOffset) || (dataRef >= (int)_data.size()) )
    return 0;

  return reinterpret_cast<const TR_DataObject*>( &_data[0] + dataRef );
}

TR_DataObject *TR_NodeTree::dataObject(TR_Types::DataRef_t dataRef)
{
  if ( (dataRef == TR_Types::g_nullOffset) || (dataRef >= (int)_data.size()) )
    return 0;

  return reinterpret_cast<TR_DataObject*>( &_data[0] + dataRef );
}

bool TR_NodeTree::queryNodeName( const TR_NodeDataObject *nodeData, STD_String &name ) const
{
  if ( nodeData->_nameOffset != TR_Types::g_nullOffset )
  {
    TR_Types::DataRef_t stringDataRef = dataRef( nodeData, nodeData->_nameOffset );
    if ( stringDataRef != TR_Types::g_nullOffset )
    {
      const TR_StringDataObject *stringData = dataObject<TR_StringDataObject>(stringDataRef);
      if ( stringData )
      {
        if ( stringData->_nChars > 0 )
        {
          //name = STD_String( &stringData->_value );
          name = STD_String( reinterpret_cast<const char*>(stringData) + TR_DataObject::g_dataObjectSizeTable[TR_Types::eStringData]);
        }
        else
        {
          name = "";  // empty character string.
        }
        return true;
      }
    }
  }

  return false;
}

bool TR_NodeTree::querySpriteName( const TR_DrawingDataObject *drawingData, STD_String &name ) const
{
  if ( drawingData->_drawingNameOffset != TR_Types::g_nullOffset )
  {
    TR_Types::DataRef_t stringDataRef = dataRef( drawingData, drawingData->_drawingNameOffset );
    if ( stringDataRef != TR_Types::g_nullOffset )
    {
      const TR_StringDataObject *stringData = dataObject<TR_StringDataObject>(stringDataRef);
      if ( stringData )
      {
        if ( stringData->_nChars > 0 )
        {
          //name = STD_String( &stringData->_value );
          name = STD_String( reinterpret_cast<const char*>(stringData) + TR_DataObject::g_dataObjectSizeTable[TR_Types::eStringData]);
        }
        else
        {
          name = ""; // empty character string.
        }
        return true;
      }
    }
  }

  return false;
}

const STD_String &TR_NodeTree::spriteSheetName() const
{
  return _spriteSheetName;
}

void TR_NodeTree::setSpriteSheetName( const STD_String &spriteSheetName )
{
  _spriteSheetName = spriteSheetName;
}

TR_StringDataObject *TR_NodeTree::addStringDataObject( const STD_String &str )
{
  size_t sizeOfString = str.size();

  //  string data size + string size + \0
  size_t sizeOfStringData = TR_DataObject::g_dataObjectSizeTable[TR_Types::eStringData] + sizeOfString + 1;

  //  Align char array in 32 bits chunks.
  unsigned roundTo = 4;
  size_t remainder = sizeOfStringData % roundTo;
  if ( remainder != 0 )
    sizeOfStringData += roundTo - remainder;

  if ( _data.size() + sizeOfStringData >= _data.capacity() )
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

  _data.resize( _data.size() + sizeOfStringData );

  TR_StringDataObject *stringData = reinterpret_cast<TR_StringDataObject*>( &_data.rbegin()[ sizeOfStringData-1 ] );

  stringData = new (stringData) TR_StringDataObject;
  stringData->_nChars = (unsigned int)sizeOfString;

  //  Copy string in allocated extra buffer.
  char *dstString = reinterpret_cast<char*>(stringData) + TR_DataObject::g_dataObjectSizeTable[TR_Types::eStringData];
  strcpy( dstString , str.c_str() );

  return stringData;
}

void TR_NodeTree::store( IO_PersistentStore &store ) const
{
  store << _firstNodeDataRef;

  store << _spriteSheetName;

  store << (int)_data.size();
  store.storeBuffer( &_data[0], _data.size() );
}

void TR_NodeTree::load( IO_PersistentStore &store )
{
  store >> _firstNodeDataRef;

  store >> _spriteSheetName;

  int bufferSize;
  store >> bufferSize;

  _data.resize( bufferSize );

  store.loadBuffer( &_data[0], bufferSize );
}

void TR_NodeTree::dump() const
{
  for ( NodeIterator i = nodeBegin(), iEnd = nodeEnd() ; i!=iEnd ; ++i )
  {
    TR_NodeDataObject &nodeData = *i;

    STD_String nodeName = "nil";
    queryNodeName( &nodeData, nodeName );
    //queryFromNameTable(nodeData._nameId, nodeName);

    printf( "[%s] offset: %i ", nodeName.c_str(), dataRef( &nodeData ) );

    if (nodeData._channelDataOffset != TR_Types::g_nullOffset)
    {
      printf( "\n  channels( " );

      TR_Types::DataRef_t ref = dataRef( &nodeData );
      TR_Types::DataOffset_t offset = nodeData._channelDataOffset;

      while( offset != TR_Types::g_nullOffset )
      {
        ref = ref + offset;

        const TR_ChannelDataObject *channelData = dataObject<TR_ChannelDataObject>(ref);
        if ( channelData )
        {
          TR_Utils::NameCol_t names;
          if ( TR_Utils::channelToNames( channelData->_channelType, names ) )
          {
            printf( "%s ", names.begin()->c_str() );
          }

          offset = channelData->_nextChannelDataOffset;
        }
        else
        {
          offset = TR_Types::g_nullOffset;
        }
      }

      printf( ") ");
    }


#if 0
    if (nodeData._animationDataOffset != TR_Types::g_nullOffset)
    {
      const TR_DataObject *data = dataObject( dataRef(&nodeData) + nodeData._animationDataOffset );
      if ((data != 0) && (data->type() == TR_Types::eAnimationData))
      {
        printf( "\n  channels( " );
        const TR_AnimationDataObject *animationData = static_cast<const TR_AnimationDataObject*>(data);
        for ( unsigned i=0 ; i < TR_Types::MAX_CURVE_CHANNELS ; ++i )
        {
          if (animationData->_channels[i] != TR_Types::g_nullOffset)
          {
            TR_Utils::NameCol_t names;
            if ( TR_Utils::channelToNames( TR_Types::CurveChannel_t(i), names ) )
            {
              printf( "%s ", names.begin()->c_str() );
            }
          }
        }
        printf( ") ");
      }
    }
#endif

    if (nodeData._drawingDataOffset != TR_Types::g_nullOffset)
    {
      const TR_DrawingAnimationDataObject *drawingAnimationData = dataObject<TR_DrawingAnimationDataObject>( dataRef( &nodeData, nodeData._drawingDataOffset ) );
      if (drawingAnimationData && (drawingAnimationData->_nDrawings > 0) )
      {
        printf( "\n  drawings( " );
        const TR_DataObject *dataIt = reinterpret_cast<const TR_DataObject*>(reinterpret_cast<const Data_t*>(drawingAnimationData) + sizeof(TR_DrawingAnimationDataObject));
        for ( unsigned i=0 ; i < drawingAnimationData->_nDrawings ; ++i )
        {
          //  Error while parsing data
          if ( dataIt->_id != TR_Types::eDrawingData )
            break;

          const TR_DrawingDataObject *drawingData = static_cast<const TR_DrawingDataObject*>(dataIt);

          STD_String sprite;
          if (querySpriteName( drawingData, sprite ))
          {
            printf( "%s:%s ", spriteSheetName().c_str(), sprite.c_str() );
          }
          else
          {
            printf( "nil:nil ");
          }

          dataIt = reinterpret_cast<const TR_DataObject*>(reinterpret_cast<const Data_t*>(dataIt) + sizeof(TR_DrawingDataObject));
        }
        printf( ") ");
      }
    }

    printf("\n");

  }

  printf("\nTotal data size: %lu bytes\n", _data.size() );
}

