
#include "RD_RenderScriptMeta.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RenderScriptMeta
#endif

RD_RenderScriptMeta::RD_RenderScriptMeta() :
  _idCount(0)
{
}

RD_RenderScriptMeta::~RD_RenderScriptMeta()
{
  for ( PropDataCol_t::const_iterator i=_props.begin(), iEnd=_props.end() ; i!=iEnd ; ++i )
  {
    delete i->second;
  }
  _props.clear();

  for ( PropAnchorDataCol_t::const_iterator i=_anchors.begin(), iEnd=_anchors.end() ; i!=iEnd ; ++i )
  {
    delete i->second.second;
  }
  _anchors.clear();
}

int RD_RenderScriptMeta::createProp( const STD_String &name, const TV_NodeTreeViewPtr_t &nodeTreeView )
{
  int propId = ++_idCount;
  _props.insert( STD_MakePair(propId, new PropData(name, nodeTreeView)) );

  return propId;
}

void RD_RenderScriptMeta::updateProp( int propId, float frame )
{
  PropDataCol_t::const_iterator i = _props.find(propId);
  i->second->_frame = frame;
}

void RD_RenderScriptMeta::anchorProp( int propId, const STD_String &name, const STD_String &nodeName )
{
  _anchors[ propId ] = STD_MakePair( _props[propId], new AnchorData(name, nodeName) );
}

void RD_RenderScriptMeta::unanchorProp( int propId )
{
  for ( PropAnchorDataCol_t::const_iterator i=_anchors.begin(), iEnd=_anchors.end() ; i!=iEnd ; ++i )
  {
    if ( i->first == propId )
    {
      delete i->second.second;
      break;
    }
  }
  _anchors.erase(propId);
}

float RD_RenderScriptMeta::frame( int propId ) const
{
  PropDataCol_t::const_iterator i = _props.find(propId);
  if (i!=_props.end())
  {
    return i->second->_frame;
  }

  return 1.0f;
}

const RD_RenderScriptMeta::PropAnchorDataCol_t &RD_RenderScriptMeta::anchors() const
{
  return _anchors;
}

void RD_RenderScriptMeta::setNodeVisibility( const STD_String &/*name*/, const STD_String &/*noneName*/, bool /*visibile*/ )
{
  // TODO! 
}

void RD_RenderScriptMeta::clearVisibilityFlags()
{
  // TODO!
}
