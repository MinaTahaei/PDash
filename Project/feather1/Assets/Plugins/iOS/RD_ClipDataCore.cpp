#include "RD_ClipDataCore.h"
#include "TR_NodeTree.h"
#include "TV_NodeTreeView.h"
#include "IO_PersistentStore.h"
#include "STD_Containers.h"
#include "STD_Types.h"
#include <string.h>

#define RD_ClipDataTag             "CLPD"
#define RD_ClipDataMajorVersion    (int)4
#define RD_ClipDataMinorVersion    (int)0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ClipDataCore::Impl
#endif
class RD_ClipDataCore::Impl
{
  MEM_OVERRIDE

  friend class RD_ClipDataCore;

public:

  Impl()
  {
  }

  ~Impl()
  {
  }

private:

  struct ClipData
  {
    MEM_OVERRIDE

    STD_String             _name;
    TR_NodeTree           *_nodeTree;
  };

  typedef STD_Vector< ClipData > NodeTreeViewCol_t;

  struct SoundData
  {
    MEM_OVERRIDE

    STD_String             _soundName;
    float                  _startFrame;
  };

  typedef STD_Vector< SoundData > SoundDataCol_t;

  NodeTreeViewCol_t _nodeTrees;
  SoundDataCol_t    _soundEvents;

  bool              _fxFlag;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ClipDataCore
#endif

RD_ClipDataCore::RD_ClipDataCore()
{
  _i = new Impl;
}

RD_ClipDataCore::~RD_ClipDataCore()
{
  eraseAll();
  delete _i;
}

TV_NodeTreeViewPtr_t RD_ClipDataCore::nodeTreeView( unsigned idx ) const
{
  if ( idx < _i->_nodeTrees.size() )
  {
    return TV_NodeTreeView::create(_i->_nodeTrees[idx]._nodeTree);
  }

  return TV_NodeTreeViewPtr_t(0);
}

TV_NodeTreeViewPtr_t RD_ClipDataCore::nodeTreeView( const STD_String &name ) const
{
  for ( Impl::NodeTreeViewCol_t::const_iterator i=_i->_nodeTrees.begin(), iEnd=_i->_nodeTrees.end() ; i!=iEnd ; ++i )
  {
    if ( i->_name.compare( name ) == 0 )
    {
      return TV_NodeTreeView::create(i->_nodeTree);
    }
  }

  return TV_NodeTreeViewPtr_t(0);
}

const STD_String &RD_ClipDataCore::name( unsigned idx ) const
{
  if ( idx < _i->_nodeTrees.size() )
  {
    const Impl::ClipData &data = _i->_nodeTrees[idx];
    return data._name;
  }

  static STD_String g_nullString = "";
  return g_nullString;
}

size_t RD_ClipDataCore::count() const
{
  return _i->_nodeTrees.size();
}



void RD_ClipDataCore::store( IO_PersistentStore &store ) const
{
  storeHeader( store );

  store << (int)_i->_nodeTrees.size();

  for ( Impl::NodeTreeViewCol_t::const_iterator i = _i->_nodeTrees.begin(), iEnd = _i->_nodeTrees.end() ; i!=iEnd ; ++i )
  {
    i->_nodeTree->store( store );

    store << i->_name;
  }

  store << (int)_i->_soundEvents.size();

  for ( Impl::SoundDataCol_t::const_iterator i = _i->_soundEvents.begin(), iEnd = _i->_soundEvents.end() ; i!=iEnd ; ++i )
  {
    store << i->_soundName;
    store << i->_startFrame;
  }
}

bool RD_ClipDataCore::load( IO_PersistentStore &store )
{
  if ( !loadHeader( store ) )
    return false;

  int nTrees;
  store >> nTrees;

  beginClipData();

  while ( nTrees-- )
  {
    TR_NodeTree *nodeTree = new TR_NodeTree;
    nodeTree->load( store );

    STD_String name;
    store >> name;

    addNodeTree(nodeTree, name);
  }

  int nSounds;
  store >> nSounds;

  while ( nSounds-- )
  {
    STD_String soundName;
    float startFrame;

    store >> soundName;
    store >> startFrame;

    addSoundEvent(soundName, startFrame);
  }

  endClipData();

  return true;
}

bool RD_ClipDataCore::fxEnabled() const
{
  return _i->_fxFlag;
}

float RD_ClipDataCore::totalDuration() const
{
  float totalDuration = 0.0f;
  for ( Impl::NodeTreeViewCol_t::const_iterator i = _i->_nodeTrees.begin(), iEnd = _i->_nodeTrees.end() ; i!=iEnd ; ++i )
  {
    TV_NodeTreeViewPtr_t nodeTreeView = TV_NodeTreeView::create(i->_nodeTree);
    if ( nodeTreeView->totalDuration() > totalDuration )
      totalDuration = nodeTreeView->totalDuration();
  }

  return totalDuration;
}

void RD_ClipDataCore::beginClipData()
{
  eraseAll();
}

void RD_ClipDataCore::addNodeTree( TR_NodeTree *nodeTree, const STD_String &name )
{
  _i->_nodeTrees.push_back( Impl::ClipData() );
  Impl::ClipData &data = _i->_nodeTrees.back();

  data._name = name;
  data._nodeTree = nodeTree;
}

void RD_ClipDataCore::addSoundEvent( const STD_String &soundName, float startFrame )
{
  _i->_soundEvents.push_back( Impl::SoundData() );
  Impl::SoundData &data = _i->_soundEvents.back();

  data._soundName = soundName;
  data._startFrame = startFrame;
}

void RD_ClipDataCore::endClipData()
{
  //  Iterate through hiearchy and look for effects that will require an
  //  enhanced fx script to render.
  updateFxFlag();
}

void RD_ClipDataCore::storeHeader( IO_PersistentStore &store ) const
{
  store << RD_ClipDataTag[0];
  store << RD_ClipDataTag[1];
  store << RD_ClipDataTag[2];
  store << RD_ClipDataTag[3];

  store << RD_ClipDataMajorVersion;
  store << RD_ClipDataMinorVersion;
}

bool RD_ClipDataCore::loadHeader( IO_PersistentStore &store ) const
{
  char mainTag[5];
  store >> mainTag[0];
  store >> mainTag[1];
  store >> mainTag[2];
  store >> mainTag[3];
  mainTag[4] = '\0';

  if ( strcmp( mainTag, RD_ClipDataTag ) != 0 )
    return false;

  int majorVersion, minorVersion;
  store >> majorVersion;
  store >> minorVersion;

  //  Do not attempt to read file if major version number
  //  is different.
  if ( majorVersion != RD_ClipDataMajorVersion )
    return false;

  return true;
}

void RD_ClipDataCore::updateFxFlag()
{
  _i->_fxFlag = false;

  for ( Impl::NodeTreeViewCol_t::const_iterator i = _i->_nodeTrees.begin(), iEnd = _i->_nodeTrees.end() ; i!=iEnd ; ++i )
  {
    const TR_NodeTree *nodeTree = i->_nodeTree;
    for ( TR_NodeTree::NodeIterator j = nodeTree->nodeBegin(), jEnd = nodeTree->nodeEnd() ; j!=jEnd ; ++j )
    {
      //  Assigned effect data.
      if ( j->_effectDataOffset != TR_Types::g_nullOffset )
      {
        _i->_fxFlag = true;
        return;
      }
    }
  }
}

void RD_ClipDataCore::eraseAll()
{
  for ( Impl::NodeTreeViewCol_t::iterator i = _i->_nodeTrees.begin(), iEnd = _i->_nodeTrees.end() ; i!=iEnd ; ++i )
  {
    delete i->_nodeTree;
  }
  _i->_nodeTrees.clear();

  _i->_soundEvents.clear();
}
