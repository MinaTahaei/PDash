
#include "RD_RenderScript.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RenderScript
#endif

RD_RenderScript::RD_RenderScript() :
  _meta(new RD_RenderScriptMeta)
{
}

RD_RenderScript::RD_RenderScript(const RD_RenderScriptMetaPtr_t &meta) :
  _meta(meta)
{
}

RD_RenderScript::~RD_RenderScript()
{
}

int RD_RenderScript::createProp( const STD_String &name, const TV_NodeTreeViewPtr_t &nodeTreeView )
{
  lock();
  int propId = _meta->createProp(name, nodeTreeView);
  unlock();

  return propId;
}

void RD_RenderScript::updateProp( int propId, float frame )
{
  lock();

  //  Update meta.
  _meta->updateProp(propId, frame);

  unlock();
}

void RD_RenderScript::anchorProp( int propId, const STD_String &name, const STD_String &nodeName )
{
  lock();
  _meta->anchorProp(propId, name, nodeName);
  cleanup();
  unlock();
}

void RD_RenderScript::unanchorProp( int propId )
{
  lock();
  _meta->unanchorProp(propId);
  cleanup();
  unlock();
}

void RD_RenderScript::deallocateConvexHull( BoundingVertex *convexHullArray )
{
  delete [] convexHullArray;
}

void RD_RenderScript::deallocatePolygons( BoundingVertex *polygonArray, int *subPolygonArray )
{
  delete [] polygonArray;
  delete [] subPolygonArray;
}

const RD_RenderScriptMetaPtr_t &RD_RenderScript::meta() const
{
  return _meta;
}

namespace
{
  struct FindAnchorNodeTreeView
  {
    FindAnchorNodeTreeView(const STD_String &anchorName) :
      _anchorName(anchorName)
    {
    }

    bool operator()(const TV_NodeTreeView &nodeTreeView) const
    {
      //  Only a node with sprites will be displayed in composition
      //  and as such should be favoured when searching for a
      //  valid anchor...
      if ( (nodeTreeView.name().compare(_anchorName) == 0 ) &&
           (nodeTreeView.nSprites() > 0) )
      {
        return true;
      }

      return false;
    }

    STD_String _anchorName;
  };

  struct FindCompositionNode
  {
    FindCompositionNode(const TV_NodeTreeViewPtr_t &nodeTreeView) :
      _nodeTreeView(nodeTreeView)
    {
    }

    bool operator()(const RD_Composition::CompositionNode *compositionNode) const
    {
      if ( *(compositionNode->_nodeTreeView) == *_nodeTreeView )
        return true;

      return false;
    }

    TV_NodeTreeViewPtr_t _nodeTreeView;
  };

  struct FindComposition
  {
    FindComposition(const STD_String &name) :
      _name(name)
    {
    }

    bool operator()(const RD_Composition *composition) const
    {
      return (composition->name().compare( _name ) == 0);
    }

    const STD_String &_name;
  };
}

unsigned RD_RenderScript::buildComposition( const RD_ClipDataPtr_t &pClipData,
                                            float frame,
                                            const STD_String &projectFolder,
                                            const STD_String &sheetResolution )
{
  unsigned nPrimitives = 0;

  //  Build main composition...
  for ( unsigned i = 0, iEnd = (unsigned)pClipData->count() ; i<iEnd ; ++i )
  {
    TV_NodeTreeViewPtr_t nodeTreeView = pClipData->nodeTreeView(i);
    const STD_String &name = pClipData->name(i);

    STD_String spriteSheetName;
    if (nodeTreeView->spriteSheet(spriteSheetName))
    {
      RD_SpriteSheetKey key(projectFolder, spriteSheetName, sheetResolution);
      RD_SpriteSheetPtr_t pSpriteSheet = RD_SpriteSheet::createOrLoad( key );

      if (pSpriteSheet.isValid())
      {
        RD_Composition *composition = new RD_Composition( name, pSpriteSheet, nodeTreeView );
        composition->build( frame );

        _compositions.push_back(composition);

        nPrimitives += composition->nodeCount();
      }
    }
  }

  //  Build prop composition based on anchors...
  const RD_RenderScriptMeta::PropAnchorDataCol_t &anchors = _meta->anchors();
  for ( RD_RenderScriptMeta::PropAnchorDataCol_t::const_iterator iAnchor = anchors.begin(), iAnchorEnd = anchors.end() ;
      iAnchor!=iAnchorEnd ; ++iAnchor )
  {
    const RD_RenderScriptMeta::PropData *propData = iAnchor->second.first;
    const RD_RenderScriptMeta::AnchorData *anchorData = iAnchor->second.second;

    CompositionCol_t::const_iterator iComp = std::find_if( _compositions.begin(), _compositions.end(), FindComposition(anchorData->_name) );
    if (iComp == _compositions.end())
      continue;

    TV_NodeTreeViewPtr_t parentNodeTreeView = (*iComp)->nodeTreeView()->findIf( FindAnchorNodeTreeView(anchorData->_nodeName) );
    if (!parentNodeTreeView.isValid())
      continue;

    TV_NodeTreeViewPtr_t propNodeTreeView = propData->_nodeTreeView;
    if (!propNodeTreeView.isValid())
      continue;

    STD_String spriteSheetName;
    if (propNodeTreeView->spriteSheet(spriteSheetName))
    {
      RD_SpriteSheetKey key(projectFolder, spriteSheetName, sheetResolution);
      RD_SpriteSheetPtr_t pSpriteSheet = RD_SpriteSheet::createOrLoad( key );

      if (pSpriteSheet.isValid())
      {
        RD_Composition *composition = new RD_Composition( propData->_name, pSpriteSheet, propNodeTreeView, parentNodeTreeView );
        composition->build( propData->_frame, frame );

        _props.insert( STD_MakePair(iAnchor->first, composition) );

        nPrimitives += composition->nodeCount();
      }
    }
  }

  return nPrimitives;
}

unsigned RD_RenderScript::buildCompositionWithBlending( const RD_ClipDataPtr_t &pClipData,
												float frameFrom,
												float frameTo,
												const STD_String &projectFolder,
												const STD_String &sheetResolution, 
												float fullBlendTime, 
												float currentBlendTime,
												int blendID)
{
  unsigned nPrimitives = 0;

  //  Build main composition...
  for ( unsigned i = 0, iEnd = (unsigned)pClipData->count() ; i<iEnd ; ++i )
  {
    TV_NodeTreeViewPtr_t nodeTreeView = pClipData->nodeTreeView(i);
	STD_String nodeName = nodeTreeView->name();
    const STD_String &name = pClipData->name(i);

    STD_String spriteSheetName;
    if (nodeTreeView->spriteSheet(spriteSheetName))
    {
      RD_SpriteSheetKey key(projectFolder, spriteSheetName, sheetResolution);
      RD_SpriteSheetPtr_t pSpriteSheet = RD_SpriteSheet::createOrLoad( key );

      if (pSpriteSheet.isValid())
      {
        RD_Composition *composition = new RD_Composition( name, pSpriteSheet, nodeTreeView );
        composition->buildWithBlending( frameFrom,frameTo ,fullBlendTime,currentBlendTime,blendID );

        _compositions.push_back(composition);

        nPrimitives += composition->nodeCount();
      }
    }
  }

  //  Build prop composition based on anchors...
  const RD_RenderScriptMeta::PropAnchorDataCol_t &anchors = _meta->anchors();
  for ( RD_RenderScriptMeta::PropAnchorDataCol_t::const_iterator iAnchor = anchors.begin(), iAnchorEnd = anchors.end() ;
      iAnchor!=iAnchorEnd ; ++iAnchor )
  {
    const RD_RenderScriptMeta::PropData *propData = iAnchor->second.first;
    const RD_RenderScriptMeta::AnchorData *anchorData = iAnchor->second.second;

    CompositionCol_t::const_iterator iComp = std::find_if( _compositions.begin(), _compositions.end(), FindComposition(anchorData->_name) );
    if (iComp == _compositions.end())
      continue;

    TV_NodeTreeViewPtr_t parentNodeTreeView = (*iComp)->nodeTreeView()->findIf( FindAnchorNodeTreeView(anchorData->_nodeName) );
    if (!parentNodeTreeView.isValid())
      continue;

    TV_NodeTreeViewPtr_t propNodeTreeView = propData->_nodeTreeView;
    if (!propNodeTreeView.isValid())
      continue;

    STD_String spriteSheetName;
    if (propNodeTreeView->spriteSheet(spriteSheetName))
    {
      RD_SpriteSheetKey key(projectFolder, spriteSheetName, sheetResolution);
      RD_SpriteSheetPtr_t pSpriteSheet = RD_SpriteSheet::createOrLoad( key );

      if (pSpriteSheet.isValid())
      {
        RD_Composition *composition = new RD_Composition( propData->_name, pSpriteSheet, propNodeTreeView, parentNodeTreeView );
		composition->buildWithBlending( propData->_frame,frameTo,fullBlendTime,currentBlendTime,frameFrom);

        _props.insert( STD_MakePair(iAnchor->first, composition) );

        nPrimitives += composition->nodeCount();
      }
    }
  }

  return nPrimitives;
}

void RD_RenderScript::updateComposition( float frame )
{
  for ( CompositionCol_t::const_iterator i = _compositions.begin(), iEnd = _compositions.end() ; i!=iEnd ; ++i )
  {
    (*i)->update( frame );
  }

  for ( PropCompositionCol_t::const_iterator i = _props.begin(), iEnd = _props.end() ; i!=iEnd ; ++i )
  {
    i->second->update( _meta->frame(i->first), frame );
  }

}

void RD_RenderScript::updateCompositionWithBlending( float frameFrom, float frameTo, float fullBlendTime, float currentBlendTime,int blendID )
{
  for ( CompositionCol_t::const_iterator i = _compositions.begin(), iEnd = _compositions.end() ; i!=iEnd ; ++i )
  {
    (*i)->updateWithBlending( frameFrom,frameTo, 0.0f, fullBlendTime, currentBlendTime,blendID);
  }

  for ( PropCompositionCol_t::const_iterator i = _props.begin(), iEnd = _props.end() ; i!=iEnd ; ++i )
  {
    //i->second->update( _meta->frame(i->first), frame );
	i->second->updateWithBlending( _meta->frame(i->first),frameTo, frameFrom, fullBlendTime, currentBlendTime,blendID);
  }

}

void RD_RenderScript::updateRequests()
{
  //  OPTIMIZE ME!!!
  _requests.clear();

  //  Create a list of batch request using composition collection.
  //  Create in reverse order to match Harmony rendering order. 
  for ( CompositionCol_t::const_reverse_iterator i = _compositions.rbegin(), iEnd = _compositions.rend() ; i!=iEnd ; ++i )
  {
    _requests.push_back( RenderBatchRequest(*i) );
  }

  //  Refine request list by inserting props in batch request.  Split request in two sub requests and insert
  //  prop request in between.
  for ( PropCompositionCol_t::const_iterator i = _props.begin(), iEnd = _props.end() ; i!=iEnd ; ++i )
  {
    RD_Composition *propComposition = i->second;
    TV_NodeTreeViewPtr_t parentNodeTreeView = propComposition->parentNodeTreeView();

    if ( !parentNodeTreeView )
      continue;

    for ( RenderBatchRequestCol_t::iterator j = _requests.begin(), jEnd = _requests.end() ; j!=jEnd ; ++j )
    {
      if ( j->_composition->nodeTreeView()->hasCommonRoot( *parentNodeTreeView ) )
      {
        RenderBatchRequest::Iterator_t k = STD_FindIf( j->_range.first, j->_range.second, FindCompositionNode(parentNodeTreeView) );
        if ( k != j->_range.second )
        {
          RD_Composition *composition = j->_composition;

          RenderBatchRequest::Iterator_t i1 = j->_range.first;
          RenderBatchRequest::Iterator_t i2 = k;
          RenderBatchRequest::Iterator_t i3 = j->_range.second;

          // 1. create batch that renders before prop if necessary.
          if ( i1!=i2 )
          {
            _requests.insert( j, RenderBatchRequest(composition, i1, i2) );
          }

          // 2. insert prop
          _requests.insert( j, RenderBatchRequest(propComposition) );

          // 3. modify batch that renders after prop.
          j->_range.first = i2;
          j->_range.second = i3;

          break;
        }
      }
    }
  }
}

void RD_RenderScript::lock()
{
  _mutex.lock();
}

void RD_RenderScript::unlock()
{
  _mutex.unlock();
}

void RD_RenderScript::cleanup()
{
  //  Cleanup compositions.
  for ( CompositionCol_t::const_iterator i = _compositions.begin(), iEnd = _compositions.end() ; i!=iEnd ; ++i )
  {
    delete *i;
  }
  _compositions.clear();

  //  Cleanup props.
  for ( PropCompositionCol_t::const_iterator i = _props.begin(), iEnd = _props.end() ; i!=iEnd ; ++i )
  {
    delete i->second;
  }
  _props.clear();
}
