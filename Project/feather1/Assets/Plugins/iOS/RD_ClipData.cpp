#include "RD_ClipData.h"
#include "TR_NodeTree.h"
#include "TV_NodeTreeView.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ClipData::Impl
#endif
class RD_ClipData::Impl
{
  friend class RD_ClipData;

public:

  Impl()
  {
  }

  ~Impl()
  {
  }

private:

  RD_ClipDataCore *_pClipData;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ClipData
#endif

RD_ClipData::RD_ClipData( RD_ClipDataCore *pClipData )
{
  _i = new Impl;
  _i->_pClipData = pClipData;
}

RD_ClipData::~RD_ClipData()
{
  delete _i->_pClipData;
  delete _i;
}

TV_NodeTreeViewPtr_t RD_ClipData::nodeTreeView( unsigned idx ) const
{
  return _i->_pClipData->nodeTreeView(idx);
}

TV_NodeTreeViewPtr_t RD_ClipData::nodeTreeView( const STD_String &name ) const
{
  return _i->_pClipData->nodeTreeView(name);
}

const STD_String &RD_ClipData::name( unsigned idx ) const
{
  return _i->_pClipData->name(idx);
}

size_t RD_ClipData::count() const
{
  return _i->_pClipData->count();
}

bool RD_ClipData::fxEnabled() const
{
  return _i->_pClipData->fxEnabled();
}

float RD_ClipData::totalDuration() const
{
  return _i->_pClipData->totalDuration();
}
