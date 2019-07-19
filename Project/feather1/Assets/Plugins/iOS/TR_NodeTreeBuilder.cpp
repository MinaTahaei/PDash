#include "TR_NodeTreeBuilder.h"
#include "TR_CatmullCompute.h"

#include "STD_Containers.h"
#include "STD_Types.h"

#include <math.h>
#include <stdio.h>

#define DEBUG_TRACE
#ifndef DEBUG_TRACE
#define TRACE(x) ((void)(x))
#else
#define TRACE(x) (void)printf("(%s:%i) ", __FILE__, __LINE__); (void)printf(x)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TR_NodeTreeBuilder::Impl
#endif
class TR_NodeTreeBuilder::Impl
{
  MEM_OVERRIDE

  friend class TR_NodeTreeBuilder;
public:
  Impl() :
    _nodeTree(0),
    _blockOperation(false),
    _currentBezierRef(TR_Types::g_nullOffset),
    _currentCatmullRef(TR_Types::g_nullOffset),
    _currentLinearRef(TR_Types::g_nullOffset),
    _currentPivotRef(TR_Types::g_nullOffset),
    _currentDrawingAnimationRef(TR_Types::g_nullOffset)
  {
  }

  ~Impl()
  {
  }

private:

  TR_NodeTree               *_nodeTree;

  bool                       _blockOperation;

  TR_Types::DataRef_t        _currentBezierRef;
  TR_Types::DataRef_t        _currentCatmullRef;
  TR_Types::DataRef_t        _currentLinearRef;
  TR_Types::DataRef_t        _currentPivotRef;

  TR_Types::DataRef_t        _currentDrawingAnimationRef;

  typedef STD_Map< TR_Types::DataRef_t, STD_String > SpriteNameCol_t;
  SpriteNameCol_t            _currentSpriteMapping;

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TR_DataObject
#endif

TR_NodeTreeBuilder::TR_NodeTreeBuilder( TR_NodeTree *nodeTree )
{
  _i = new Impl;
  _i->_nodeTree = nodeTree;
}

TR_NodeTreeBuilder::~TR_NodeTreeBuilder()
{
  delete _i;
}

TR_Types::DataRef_t TR_NodeTreeBuilder::addNode( const STD_String &name )
{
  //  Add to default root node.
  return addNode( name, _i->_nodeTree->firstNodeDataRef() /*root index*/ );
}

TR_Types::DataRef_t TR_NodeTreeBuilder::addNode( const STD_String &name, TR_Types::DataRef_t parentNodeRef )
{
  //  Do not allow atomic operation during block operations.
  if (_i->_blockOperation)
  {
    TRACE( "Cannot perform atomic operation inside block operation.\n" );
    return TR_Types::g_nullOffset;
  }

  TR_NodeDataObject *nodeData = _i->_nodeTree->addDataObject<TR_NodeDataObject>();
  TR_Types::DataRef_t nodeDataRef = _i->_nodeTree->dataRef(nodeData);

  TR_StringDataObject *stringData = _i->_nodeTree->addStringDataObject( name );

  //  Retrieve back from memory as it might have changed during malloc.
  nodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(nodeDataRef);
  if (nodeData == 0)
  {
    TRACE( "Could not find node data.\n" );
    return TR_Types::g_nullOffset;
  }

  nodeData->_nameOffset = (TR_Types::DataOffset_t)( reinterpret_cast<Data_t*>(stringData) - reinterpret_cast<Data_t*>(nodeData) );

  //addNodePriv( nodeDataRef, parentNodeRef );

  //  Add new node information in parent node.
  TR_NodeDataObject *parentNodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(parentNodeRef);
  if (parentNodeData == 0)
  {
    TRACE( "Could not find parent node data.\n" );
    return TR_Types::g_nullOffset;
  }

  //  If parent node does not have any child, add current node as first child.
  if ( parentNodeData->_childDataOffset == TR_Types::g_nullOffset )
  {
    TR_Types::DataOffset_t offset = (TR_Types::DataOffset_t)( reinterpret_cast<Data_t*>(nodeData) - reinterpret_cast<Data_t*>(parentNodeData) );
    parentNodeData->_childDataOffset = offset;
  }
  //  Otherwise, add to brothers of first child.
  else
  {
    //  Retrieve first brother.
    TR_Types::DataRef_t brotherNodeRef = parentNodeRef + parentNodeData->_childDataOffset;
    TR_NodeDataObject *brotherNodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(brotherNodeRef);
    if (brotherNodeData == 0)
    {
      TRACE( "Could not find brother node data.\n" );
      return TR_Types::g_nullOffset;
    }

    //  Iterate up to last brother.
    while ( brotherNodeData->_brotherDataOffset != TR_Types::g_nullOffset )
    {
      brotherNodeRef = brotherNodeRef + brotherNodeData->_brotherDataOffset;
      brotherNodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(brotherNodeRef);
      if (brotherNodeData == 0)
      {
        TRACE( "Could not find brother node data.\n" );
        return TR_Types::g_nullOffset;
      }
    }

    TR_Types::DataOffset_t offset = (TR_Types::DataOffset_t)( reinterpret_cast<Data_t*>(nodeData) - reinterpret_cast<Data_t*>(brotherNodeData) );
    brotherNodeData->_brotherDataOffset = offset;
  }

  return nodeDataRef;
}

void TR_NodeTreeBuilder::linkNodeAnimation( TR_Types::DataRef_t srcNodeRef, TR_Types::DataRef_t dstNodeRef )
{
  //  Warning.  This code will replace link to animation data but will not remove any existing animation data that
  //  might have been created before hand.
  TR_NodeDataObject *srcNodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(srcNodeRef);
  if (srcNodeData == 0)
  {
    TRACE( "No valid source node data.\n" );
    return;
  }

  TR_NodeDataObject *dstNodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(dstNodeRef);
  if (dstNodeData == 0)
  {
    TRACE( "No valid destination node data.\n" );
    return;
  }

  TR_Types::DataRef_t channelDataRef = _i->_nodeTree->dataRef( srcNodeData, srcNodeData->_channelDataOffset );

  dstNodeData->_channelDataOffset = channelDataRef - _i->_nodeTree->dataRef( dstNodeData );
}

void TR_NodeTreeBuilder::linkChannel( TR_Types::DataRef_t srcChannelRef, TR_Types::DataRef_t dstChannelRef )
{
  //  Retrieve src Channel Data.
  TR_ChannelDataObject *srcChannelData = _i->_nodeTree->dataObject<TR_ChannelDataObject>(srcChannelRef);
  if (srcChannelData == 0)
  {
    TRACE( "No valid source channel data.\n" );
    return;
  }

  //  Retrieve src Channel Data.
  TR_ChannelDataObject *dstChannelData = _i->_nodeTree->dataObject<TR_ChannelDataObject>(dstChannelRef);
  if (dstChannelData == 0)
  {
    TRACE( "No valid destination channel data.\n" );
    return;
  }

  TR_Types::DataRef_t functionDataRef = _i->_nodeTree->dataRef( srcChannelData, srcChannelData->_linkedDataOffset );
  dstChannelData->_linkedDataOffset = functionDataRef - dstChannelRef;
}

void TR_NodeTreeBuilder::linkNodeDrawingAnimation( TR_Types::DataRef_t srcNodeRef, TR_Types::DataRef_t dstNodeRef )
{
  //  Warning.  This code will replace link to animation data but will not remove any existing animation data that
  //  might have been created before hand.
  TR_NodeDataObject *srcNodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(srcNodeRef);
  if (srcNodeData == 0)
  {
    TRACE( "No valid source node data.\n" );
    return;
  }

  if (srcNodeData->_drawingDataOffset == TR_Types::g_nullOffset)
  {
    TRACE( "No drawing animation data to link from.\n" );
    return;
  }

  TR_NodeDataObject *dstNodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(dstNodeRef);
  if (dstNodeData == 0)
  {
    TRACE( "No valid destination node data.\n" );
    return;
  }

  if (dstNodeData->_drawingDataOffset != TR_Types::g_nullOffset)
  {
    //TRACE( "Destination already has drawing animation data.\n" );
    return;
  }

  TR_Types::DataRef_t drawingDataRef = _i->_nodeTree->dataRef( srcNodeData, srcNodeData->_drawingDataOffset );

  dstNodeData->_drawingDataOffset = drawingDataRef - _i->_nodeTree->dataRef( dstNodeData );
}

TR_Types::DataRef_t TR_NodeTreeBuilder::createEmptyChannel( TR_Types::CurveChannel_t channelType, TR_Types::DataRef_t nodeRef )
{
  //  Do not allow atomic operation during block operations.
  if (_i->_blockOperation)
  {
    TRACE( "Cannot perform atomic operation inside block operation.\n" );
    return TR_Types::g_nullOffset;
  }

  //  Create new Channel Data.
  TR_Types::DataRef_t channelRef = createChannelData( channelType, nodeRef );

  return channelRef;
}

bool TR_NodeTreeBuilder::createEffect( TR_Types::DataRef_t nodeRef, TR_Types::EffectId_t effectId )
{
  //  Do not allow atomic operation during block operations.
  if (_i->_blockOperation)
  {
    TRACE( "Cannot perform atomic operation inside block operation.\n" );
    return false;
  }

  TR_NodeDataObject *nodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(nodeRef);
  if (nodeData == 0)
    return false;

  if ( nodeData->_effectDataOffset == TR_Types::g_nullOffset )
  {
    TR_EffectDataObject *effectData = _i->_nodeTree->addDataObject<TR_EffectDataObject>();
    effectData->_effectId = effectId;

    //  Retrieve back from memory as it might have changed during malloc.
    nodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(nodeRef);
    if (nodeData)
    {
      nodeData->_effectDataOffset = (TR_Types::DataOffset_t)(reinterpret_cast<Data_t*>(effectData) - reinterpret_cast<Data_t*>(nodeData));
    }
  }

  return true;
}

TR_Types::DataRef_t TR_NodeTreeBuilder::addMatteNode( const STD_String &name, TR_Types::DataRef_t parentNodeRef )
{
  //  Do not allow atomic operation during block operations.
  if (_i->_blockOperation)
  {
    TRACE( "Cannot perform atomic operation inside block operation.\n" );
    return TR_Types::g_nullOffset;
  }

  TR_NodeDataObject *nodeData = _i->_nodeTree->addDataObject<TR_NodeDataObject>();
  TR_Types::DataRef_t nodeDataRef = _i->_nodeTree->dataRef(nodeData);

  TR_StringDataObject *stringData = _i->_nodeTree->addStringDataObject( name );

  //  Retrieve back from memory as it might have changed during malloc.
  nodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(nodeDataRef);
  if (nodeData == 0)
  {
    TRACE( "Could not find node data.\n" );
    return TR_Types::g_nullOffset;
  }

  nodeData->_nameOffset = (TR_Types::DataOffset_t)( reinterpret_cast<Data_t*>(stringData) - reinterpret_cast<Data_t*>(nodeData) );

  TR_EffectDataObject *effectData = retrieveEffectData( parentNodeRef );
  if (effectData == 0)
  {
    TRACE( "Could not retrieve effect data.\n" );
    return TR_Types::g_nullOffset;
  }

  if ( effectData->_matteDataOffset != TR_Types::g_nullOffset )
  {
    TRACE( "Effect already has a matte.\n" );
    return TR_Types::g_nullOffset;
  }

  TR_Types::DataOffset_t offset = (TR_Types::DataOffset_t)(reinterpret_cast<Data_t*>(nodeData) - reinterpret_cast<Data_t*>(effectData));
  effectData->_matteDataOffset = offset;

  return nodeDataRef;
}

TR_Types::DataRef_t TR_NodeTreeBuilder::beginBezierCurve( TR_Types::CurveChannel_t channelType, TR_Types::DataRef_t nodeRef )
{
  //  Do not allow nested curve building
  if (_i->_blockOperation)
  {
    TRACE( "Already inside block operation.  Nested block operations are not permitted.\n" );
    return TR_Types::g_nullOffset;
  }

  //  Create new Channel Data.
  TR_Types::DataRef_t channelDataRef = createChannelData( channelType, nodeRef );

  //  Add bezier curve object to node tree.
  TR_BezierCurveDataObject *bezierData = _i->_nodeTree->addDataObject<TR_BezierCurveDataObject>();

  //  Retrieve channel data.  Should exist now.
  TR_ChannelDataObject *channelData = _i->_nodeTree->dataObject<TR_ChannelDataObject>(channelDataRef);
  if (channelData == 0)
  {
    TRACE( "Could not retrieve channel data.\n" );
    return TR_Types::g_nullOffset;
  }

  channelData->_linkedDataOffset = (TR_Types::DataOffset_t)( reinterpret_cast<Data_t*>(bezierData) - reinterpret_cast<Data_t*>(channelData) );

  _i->_currentBezierRef = _i->_nodeTree->dataRef(bezierData);
  _i->_blockOperation = true;

  return channelDataRef;
}

void TR_NodeTreeBuilder::endBezierCurve()
{
  if ( _i->_currentBezierRef == TR_Types::g_nullOffset )
  {
    TRACE( "No block operation on bezier curve active at this point.\n" );
    return;
  }

  _i->_blockOperation = false;
  _i->_currentBezierRef = TR_Types::g_nullOffset;
}

void TR_NodeTreeBuilder::addBezierPoint( float x, float y, float leftx, float lefty, float rightx, float righty, bool constSeg )
{
  if (_i->_currentBezierRef == TR_Types::g_nullOffset)
  {
    TRACE( "No block operation on bezier curve active at this point.\n" );
    return;
  }

  //  Each new point will be added next to previous in data buffer memory.
  TR_BezierPointDataObject *bezierPointData = _i->_nodeTree->addDataObject<TR_BezierPointDataObject>();
  bezierPointData->_x = x;
  bezierPointData->_y = y;
  bezierPointData->_leftx = leftx;
  bezierPointData->_lefty = lefty;
  bezierPointData->_rightx = rightx;
  bezierPointData->_righty = righty;
  bezierPointData->_constSeg = constSeg;

  TR_BezierCurveDataObject *bezierData = _i->_nodeTree->dataObject<TR_BezierCurveDataObject>(_i->_currentBezierRef);
  if (bezierData == 0)
  {
    TRACE( "Could not retrieve  bezier curve data.\n" );
    return;
  }

  //  Increment number of points in curve
  ++bezierData->_nPoints;
}

TR_Types::DataRef_t TR_NodeTreeBuilder::beginCatmullCurve( TR_Types::CurveChannel_t channel, TR_Types::DataRef_t nodeRef, float scaleX, float scaleY, float scaleZ )
{
  //  Do not allow nested curve building
  if (_i->_blockOperation)
  {
    TRACE( "Already inside block operation.  Nested block operations are not permitted.\n" );
    return TR_Types::g_nullOffset;
  }

  //  Create new Channel Data.
  TR_Types::DataRef_t channelDataRef = createChannelData( channel, nodeRef );

  //  Add catmull curve object to node tree.
  TR_CatmullCurveDataObject *catmullData = _i->_nodeTree->addDataObject<TR_CatmullCurveDataObject>();
  catmullData->_scaleX = scaleX;
  catmullData->_scaleY = scaleY;
  catmullData->_scaleZ = scaleZ;

  //  Retrieve channel data.  Should exist now.
  TR_ChannelDataObject *channelData = _i->_nodeTree->dataObject<TR_ChannelDataObject>(channelDataRef);
  if (channelData == 0)
  {
    TRACE( "Could not retrieve channel data.\n" );
    return TR_Types::g_nullOffset;
  }

  channelData->_linkedDataOffset = (TR_Types::DataOffset_t)( reinterpret_cast<Data_t*>(catmullData) - reinterpret_cast<Data_t*>(channelData) );

  _i->_currentCatmullRef = _i->_nodeTree->dataRef(catmullData);
  _i->_blockOperation = true;

  return channelDataRef;
}

void TR_NodeTreeBuilder::endCatmullCurve()
{
  if ( _i->_currentCatmullRef == TR_Types::g_nullOffset )
  {
    TRACE( "No block operation on catmull curve active at this point.\n" );
    return;
  }

  computeCatmullDerivatives(_i->_currentCatmullRef);
  computeCatmullDistances(_i->_currentCatmullRef);

  _i->_blockOperation = false;
  _i->_currentCatmullRef = TR_Types::g_nullOffset;
}

void TR_NodeTreeBuilder::addCatmullPoint( float frame, float x, float y, float z, float tension, float continuity, float bias )
{
  if (_i->_currentCatmullRef == TR_Types::g_nullOffset)
  {
    TRACE( "No block operation on catmull curve active at this point.\n" );
    return;
  }

  //  Each new point will be added next to previous in data buffer memory.
  TR_CatmullPointDataObject *catmullPointData = _i->_nodeTree->addDataObject<TR_CatmullPointDataObject>();
  catmullPointData->_frame = frame;
  catmullPointData->_x = x;
  catmullPointData->_y = y;
  catmullPointData->_z = z;
  catmullPointData->_tension = tension;
  catmullPointData->_continuity = continuity;
  catmullPointData->_bias = bias;

  //  Retrieve catmull curve container.
  TR_CatmullCurveDataObject *catmullData = _i->_nodeTree->dataObject<TR_CatmullCurveDataObject>(_i->_currentCatmullRef);
  if (catmullData == 0)
  {
    TRACE( "Could not retrieve catmull curve data.\n" );
    return;
  }

  //  Increment number of points in curve
  ++catmullData->_nPoints;
}

TR_Types::DataRef_t TR_NodeTreeBuilder::beginLinearCurve( TR_Types::CurveChannel_t channel, TR_Types::DataRef_t nodeRef )
{
  //  Do not allow nested curve building
  if (_i->_blockOperation)
  {
    TRACE( "Already inside block operation.  Nested block operations are not permitted.\n" );
    return TR_Types::g_nullOffset;
  }

  //  Create new Channel Data.
  TR_Types::DataRef_t channelDataRef = createChannelData( channel, nodeRef );

  //  Add linear curve object to node tree.
  TR_LinearCurveDataObject *linearData = _i->_nodeTree->addDataObject<TR_LinearCurveDataObject>();

  //  Retrieve channel data.  Should exist now.
  TR_ChannelDataObject *channelData = _i->_nodeTree->dataObject<TR_ChannelDataObject>(channelDataRef);
  if (channelData == 0)
  {
    TRACE( "Could not retrieve channel data.\n" );
    return TR_Types::g_nullOffset;
  }

  channelData->_linkedDataOffset = (TR_Types::DataOffset_t)( reinterpret_cast<Data_t*>(linearData) - reinterpret_cast<Data_t*>(channelData) );

  _i->_currentLinearRef = _i->_nodeTree->dataRef(linearData);
  _i->_blockOperation = true;

  return channelDataRef;
}

void TR_NodeTreeBuilder::endLinearCurve()
{
  if ( _i->_currentLinearRef == TR_Types::g_nullOffset )
  {
    TRACE( "No block operation on linear curve active at this point.\n" );
    return;
  }

  _i->_blockOperation = false;
  _i->_currentLinearRef = TR_Types::g_nullOffset;
}

void TR_NodeTreeBuilder::addLinearPoint( float x, float y )
{
  if (_i->_currentLinearRef == TR_Types::g_nullOffset)
  {
    TRACE( "No block operation on linear curve active at this point.\n" );
    return;
  }

  //  Each new point will be added next to previous in data buffer memory.
  TR_LinearPointDataObject *linearPointData = _i->_nodeTree->addDataObject<TR_LinearPointDataObject>();
  linearPointData->_x = x;
  linearPointData->_y = y;

  TR_LinearCurveDataObject *linearData = _i->_nodeTree->dataObject<TR_LinearCurveDataObject>(_i->_currentLinearRef);
  if (linearData == 0)
  {
    TRACE( "Could not retrieve linear curve data.\n" );
    return;
  }

  //  Increment number of points in curve
  ++linearData->_nPoints;
}

TR_Types::DataRef_t TR_NodeTreeBuilder::beginAnimatedPivot( TR_Types::CurveChannel_t channel, TR_Types::DataRef_t nodeRef )
{
  //  Do not allow nested curve building
  if (_i->_blockOperation)
  {
    TRACE( "Already inside block operation.  Nested block operations are not permitted.\n" );
    return TR_Types::g_nullOffset;
  }

  //  Create new Channel Data.
  TR_Types::DataRef_t channelDataRef = createChannelData( channel, nodeRef );

  //  Add animated pivot curve object to node tree.
  TR_AnimatedPivotDataObject *pivotData = _i->_nodeTree->addDataObject<TR_AnimatedPivotDataObject>();

  //  Retrieve channel data.  Should exist now.
  TR_ChannelDataObject *channelData = _i->_nodeTree->dataObject<TR_ChannelDataObject>(channelDataRef);
  if (channelData == 0)
  {
    TRACE( "Could not retrieve channel data.\n" );
    return TR_Types::g_nullOffset;
  }

  channelData->_linkedDataOffset = (TR_Types::DataOffset_t)( reinterpret_cast<Data_t*>(pivotData) - reinterpret_cast<Data_t*>(channelData) );

  _i->_currentPivotRef = _i->_nodeTree->dataRef(pivotData);
  _i->_blockOperation = true;

  return channelDataRef;
}

void TR_NodeTreeBuilder::endAnimatedPivot()
{
  if ( _i->_currentPivotRef == TR_Types::g_nullOffset )
  {
    TRACE( "No block operation on animated pivot active at this point.\n" );
    return;
  }

  _i->_blockOperation = false;
  _i->_currentPivotRef = TR_Types::g_nullOffset;
}

void TR_NodeTreeBuilder::addPivotPoint( float frame, float x, float y, float z )
{
  if (_i->_currentPivotRef == TR_Types::g_nullOffset)
  {
    TRACE( "No block operation on animated pivot active at this point.\n" );
    return;
  }

  //  Each new point will be added next to previous in data buffer memory.
  TR_PivotPointDataObject *pivotPointData = _i->_nodeTree->addDataObject<TR_PivotPointDataObject>();
  pivotPointData->_frame = frame;
  pivotPointData->_x = x;
  pivotPointData->_y = y;
  pivotPointData->_z = z;

  TR_AnimatedPivotDataObject *pivotData = _i->_nodeTree->dataObject<TR_AnimatedPivotDataObject>(_i->_currentPivotRef);
  if (pivotData == 0)
  {
    TRACE( "Could not retrieve animated pivot data.\n" );
    return;
  }

  //  Increment number of points in curve
  ++pivotData->_nPoints;
}

TR_Types::DataRef_t TR_NodeTreeBuilder::addConstantValue( TR_Types::CurveChannel_t channel, TR_Types::DataRef_t nodeRef, float value )
{
  //  Do not allow nested curve building
  if (_i->_blockOperation)
  {
    TRACE( "Cannot perform atomic operation inside block operation.\n" );
    return TR_Types::g_nullOffset;
  }

  //  Create new Channel Data.
  TR_Types::DataRef_t channelDataRef = createChannelData( channel, nodeRef );

  //  Add constant float data object to node tree.
  TR_FloatDataObject *constantData = _i->_nodeTree->addDataObject<TR_FloatDataObject>();
  constantData->_value = value;

  //  Retrieve channel data.  Should exist now.
  TR_ChannelDataObject *channelData = _i->_nodeTree->dataObject<TR_ChannelDataObject>(channelDataRef);
  if (channelData == 0)
  {
    TRACE( "Could not retrieve channel data.\n" );
    return TR_Types::g_nullOffset;
  }

  channelData->_linkedDataOffset = (TR_Types::DataOffset_t)( reinterpret_cast<Data_t*>(constantData) - reinterpret_cast<Data_t*>(channelData) );

  return channelDataRef;
}

bool TR_NodeTreeBuilder::beginDrawingSequence( TR_Types::DataRef_t nodeRef )
{
  //  Do not allow nested curve building
  if (_i->_blockOperation)
  {
    TRACE( "Already inside block operation.  Nested block operations are not permitted.\n" );
    return false;
  }

  //  Do not allow a new drawing sequence when one has already been set.
  if ( retrieveDrawingAnimationData(nodeRef) != TR_Types::g_nullOffset )
  {
    //TRACE( "Drawing animation already created for node.\n" );
    return false;
  }

  //  Create Drawing Animation Data.
  _i->_currentDrawingAnimationRef = createDrawingAnimationData( nodeRef );
  _i->_currentSpriteMapping.clear();
  _i->_blockOperation = true;

  return true;
}

void TR_NodeTreeBuilder::endDrawingSequence()
{
  if ( _i->_currentDrawingAnimationRef == TR_Types::g_nullOffset )
  {
    TRACE( "No block operation on animated pivot active at this point.\n" );
    return;
  }

  //  Add sprite names to drawing data.
  for ( Impl::SpriteNameCol_t::const_iterator i = _i->_currentSpriteMapping.begin(), iEnd = _i->_currentSpriteMapping.end() ; i!=iEnd ; ++i )
  {
    TR_StringDataObject *stringData = _i->_nodeTree->addStringDataObject( i->second );

    TR_DrawingDataObject *drawingData = _i->_nodeTree->dataObject<TR_DrawingDataObject>(i->first);
    if ( drawingData == 0 )
    {
      TRACE( "Could not find drawing data.\n" );
      continue;
    }

    drawingData->_drawingNameOffset = (TR_Types::DataOffset_t)( reinterpret_cast<Data_t*>(stringData) - reinterpret_cast<Data_t*>(drawingData) );
  }

  _i->_blockOperation = false;
  _i->_currentSpriteMapping.clear();
  _i->_currentDrawingAnimationRef = TR_Types::g_nullOffset;
}

void TR_NodeTreeBuilder::addDrawing( const STD_String &spriteSheetName, const STD_String &spriteName, float frame, float repeat )
{
  if (_i->_currentDrawingAnimationRef == TR_Types::g_nullOffset)
  {
    TRACE( "No block operation on drawing animation active at this point.\n" );
    return;
  }

  STD_String currentSpriteSheetName = _i->_nodeTree->spriteSheetName();
  if ( !currentSpriteSheetName.empty() &&
       (currentSpriteSheetName.compare(spriteSheetName) != 0) )
  {
    TRACE( "A single node tree cannot refer to multiple sprite sheets.\n" );
    return;
  }

  if (currentSpriteSheetName.empty())
    _i->_nodeTree->setSpriteSheetName(spriteSheetName);

  //  Each new point will be added next to previous in data buffer memory.
  TR_DrawingDataObject *drawingData = _i->_nodeTree->addDataObject<TR_DrawingDataObject>();
  TR_Types::DataRef_t drawingDataRef = _i->_nodeTree->dataRef(drawingData);
  drawingData->_frame = frame;
  drawingData->_repeat = repeat;

  //  Insert sprite name to be added at end of drawing animation sequence.
  _i->_currentSpriteMapping.insert( STD_MakePair( drawingDataRef, spriteName ) );

  TR_DrawingAnimationDataObject *drawingAnimationData = _i->_nodeTree->dataObject<TR_DrawingAnimationDataObject>(_i->_currentDrawingAnimationRef);
  if (drawingAnimationData == 0)
  {
    TRACE( "Could not retrieve drawing animation data.\n" );
    return;
  }

  //  Increment number of drawings
  ++drawingAnimationData->_nDrawings;
}

TR_EffectDataObject *TR_NodeTreeBuilder::retrieveEffectData( TR_Types::DataRef_t nodeRef ) const
{
  TR_NodeDataObject *nodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(nodeRef);
  if (nodeData == 0)
    return 0;

  if ( nodeData->_effectDataOffset == TR_Types::g_nullOffset )
    return 0;

  TR_Types::DataRef_t effectRef = nodeRef + nodeData->_effectDataOffset;
  return _i->_nodeTree->dataObject<TR_EffectDataObject>(effectRef);
}

TR_Types::DataRef_t TR_NodeTreeBuilder::createChannelData( TR_Types::CurveChannel_t channelType, TR_Types::DataRef_t nodeRef )
{
  //  Warning.  This code does not handle duplicate channels.  A duplicate channel will be added in the channel list, but
  //  might never be parsed if searching for a unique channel.
  TR_NodeDataObject *nodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(nodeRef);
  if (nodeData == 0)
    return TR_Types::g_nullOffset;

  TR_Types::DataRef_t dataRef = nodeRef;
  TR_Types::DataRef_t channelOffset = nodeData->_channelDataOffset;

  //  First channel data in node data, directly hook it up.
  if ( channelOffset == TR_Types::g_nullOffset )
  {
    //  Add new channel data.
    TR_ChannelDataObject *channelData = _i->_nodeTree->addDataObject<TR_ChannelDataObject>();
    channelData->_channelType = channelType;

    //  Retrieve back from memory as it might have changed during malloc.
    TR_NodeDataObject *nodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(nodeRef);
    if ( nodeData )
    {
      nodeData->_channelDataOffset = (TR_Types::DataOffset_t)(reinterpret_cast<Data_t*>(channelData) - reinterpret_cast<Data_t*>(nodeData));
    }

    return _i->_nodeTree->dataRef(channelData);
  }
  //  Iterate to last channel data, and append new channel data to it.
  else
  {
    while ( channelOffset != TR_Types::g_nullOffset )
    {
      TR_DataObject *dataObject = _i->_nodeTree->dataObject( dataRef + channelOffset );
      if ( dataObject->_id == TR_Types::eChannelData )
      {
        TR_ChannelDataObject *channelData = static_cast<TR_ChannelDataObject*>(dataObject);

        dataRef = _i->_nodeTree->dataRef(channelData);
        channelOffset = channelData->_nextChannelDataOffset;
      }
      else
      {
        dataRef = channelOffset = TR_Types::g_nullOffset;
        break;
      }
    }

    if ( dataRef != TR_Types::g_nullOffset )
    {
      //  Add new channel data.
      TR_ChannelDataObject *channelData = _i->_nodeTree->addDataObject<TR_ChannelDataObject>();
      channelData->_channelType = channelType;

      //  Retrieve back from memory as it might have changed during malloc.
      TR_DataObject *dataObject = _i->_nodeTree->dataObject( dataRef );
      if ( dataObject->_id == TR_Types::eChannelData )
      {
        TR_ChannelDataObject *prevChannelData = static_cast<TR_ChannelDataObject*>(dataObject);

        prevChannelData->_nextChannelDataOffset = (TR_Types::DataOffset_t)(reinterpret_cast<Data_t*>(channelData) - reinterpret_cast<Data_t*>(prevChannelData));

        return _i->_nodeTree->dataRef(channelData);
      }
    }
  }

  return TR_Types::g_nullOffset;
}


TR_Types::DataRef_t TR_NodeTreeBuilder::retrieveDrawingAnimationData( TR_Types::DataRef_t nodeRef )
{
  TR_NodeDataObject *nodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(nodeRef);
  if (nodeData == 0)
    return TR_Types::g_nullOffset;

  return nodeData->_drawingDataOffset;
}

TR_Types::DataRef_t TR_NodeTreeBuilder::createDrawingAnimationData( TR_Types::DataRef_t nodeRef )
{
  TR_NodeDataObject *nodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(nodeRef);
  if (nodeData == 0)
    return TR_Types::g_nullOffset;

  if ( nodeData->_drawingDataOffset == TR_Types::g_nullOffset )
  {
    TR_DrawingAnimationDataObject *drawingAnimationData = _i->_nodeTree->addDataObject<TR_DrawingAnimationDataObject>();

    //  Retrieve back from memory as it might have changed during malloc.
    nodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(nodeRef);
    if (nodeData)
    {
      nodeData->_drawingDataOffset = (TR_Types::DataOffset_t)(reinterpret_cast<Data_t*>(drawingAnimationData) - reinterpret_cast<Data_t*>(nodeData));
    }
  }

  if (nodeData)
    return ( nodeRef + nodeData->_drawingDataOffset );

  return TR_Types::g_nullOffset;
}

TR_CatmullPointDataObject *TR_NodeTreeBuilder::pointAt(TR_CatmullCurveDataObject *catmullData, unsigned idx)
{
  if ( catmullData == 0 )
    return 0;

  TR_Types::DataOffset_t offset = (TR_Types::DataOffset_t)(TR_DataObject::g_dataObjectSizeTable[TR_Types::eCatmullCurveData] + (TR_DataObject::g_dataObjectSizeTable[TR_Types::eCatmullPointData] * idx));
  TR_Types::DataRef_t dataRef = _i->_nodeTree->dataRef(catmullData, offset);

  return _i->_nodeTree->dataObject<TR_CatmullPointDataObject>(dataRef);
}

void TR_NodeTreeBuilder::computeCatmullDistances( TR_Types::DataRef_t curveRef )
{
  TR_CatmullCurveDataObject *catmullData = _i->_nodeTree->dataObject<TR_CatmullCurveDataObject>(curveRef);
  if (catmullData == 0)
  {
    TRACE( "Could not retrieve catmull curve data.\n" );
    return;
  }

  if (catmullData->_nPoints == 0)
    return; // empty - nothing to do.

  TR_CatmullPointDataObject *p0, *p1;

  p0 = pointAt( catmullData, 0);
  double totalDistance = 0.0;

  for ( unsigned idx = 1; idx < catmullData->_nPoints ; ++idx )
  {
    p1 = pointAt( catmullData, idx );

    double distance = TR_CatmullCompute::getDistance( p0, p1 );

    totalDistance += distance;
    p1->_distance = (float)totalDistance;

    p0 = p1;
  }
}

void TR_NodeTreeBuilder::computeCatmullDerivatives( TR_Types::DataRef_t curveRef )
{
  TR_CatmullCurveDataObject *catmullData = _i->_nodeTree->dataObject<TR_CatmullCurveDataObject>(curveRef);
  if (catmullData == 0)
  {
    TRACE( "Could not retrieve catmull curve data.\n" );
    return;
  }

  if (catmullData->_nPoints == 0)
    return; // empty - nothing to do.

  TR_CatmullPointDataObject *p0, *p1, *p2;

  double dist1x, dist2x,
         dist1y, dist2y,
         dist1z, dist2z;

  double a1, a2, denom;

  for ( unsigned idx = 0 ; idx < catmullData->_nPoints ; ++idx )
  {
    p0 = pointAt( catmullData, idx );

    if ( (idx+1) >= catmullData->_nPoints )
      p1 = p0;
    else
      p1 = pointAt( catmullData, idx+1 );

    if ( (idx+2) >= catmullData->_nPoints )
      p2 = p1;
    else
      p2 = pointAt( catmullData, idx+2 );

    dist1x = (double)p1->_x - (double)p0->_x;
    dist1y = (double)p1->_y - (double)p0->_y;
    dist1z = (double)p1->_z - (double)p0->_z;

    dist2x = (double)p2->_x - (double)p1->_x;
    dist2y = (double)p2->_y - (double)p1->_y;
    dist2z = (double)p2->_z - (double)p1->_z;

    dist1y = dist1y * 0.75;
    dist1z = dist1z * 2.0;
    dist2y = dist2y * 0.75;
    dist2z = dist2z * 2.0;

    a1 = sqrt(dist1x * dist1x +
      dist1y * dist1y +
      dist1z * dist1z);

    a2 = sqrt(dist2x * dist2x +
              dist2y * dist2y +
              dist2z * dist2z);

    denom = a2 + a1;

    if (denom == 0.0)
    {
      p1->_d_in_x  = p1->_d_in_y  = p1->_d_in_z  = 0.0;
      p1->_d_out_x = p1->_d_out_y = p1->_d_out_z = 0.0;
    }
    else
    {
      double t, c, b;

      t = p1->_tension;
      c = p1->_continuity;
      b = p1->_bias;

      /* 
      Ref: equation 8 and 9 of Kochanek paper with in-house
      modification to avoid the loop problem when 2 points are
      too close to each other. 
      */
      p1->_d_in_x = (float) ( ((1-t) * (1-c) * (1+b) * (dist1x*a2)
        + (1-t) * (1+c) * (1-b) * (dist2x*a1)) / denom );
      p1->_d_in_y = (float) ( ((1-t) * (1-c) * (1+b) * (dist1y*a2)
        + (1-t) * (1+c) * (1-b) * (dist2y*a1)) / denom );
      p1->_d_in_z = (float) ( ((1-t) * (1-c) * (1+b) * (dist1z*a2)
        + (1-t) * (1+c) * (1-b) * (dist2z*a1)) / denom );

      p1->_d_out_x = (float) ( ((1-t) * (1+c) * (1+b) * (dist1x*a2)
        + (1-t) * (1-c) * (1-b) * (dist2x*a1)) / denom );
      p1->_d_out_y = (float) ( ((1-t) * (1+c) * (1+b) * (dist1y*a2)
        + (1-t) * (1-c) * (1-b) * (dist2y*a1)) / denom );
      p1->_d_out_z = (float) ( ((1-t) * (1+c) * (1+b) * (dist1z*a2)
        + (1-t) * (1-c) * (1-b) * (dist2z*a1)) / denom );

      p1->_d_in_y = p1->_d_in_y / 0.75f;
      p1->_d_in_z = p1->_d_in_z / 2.0f;
      p1->_d_out_y = p1->_d_out_y/ 0.75f;
      p1->_d_out_z = p1->_d_out_z / 2.0f;
    }
  }

  /*
  Special case: first and last control point. We will compute a 
  derivative based on a second degree curve instead of a third
  degree curve. 
  */

  if ( catmullData->_nPoints > 1 )
  {
    //  first and last point
    p0 = pointAt( catmullData, 0 );
    p1 = pointAt( catmullData, 1 );

    p0->_d_in_x  = p0->_d_in_y = p0->_d_in_z = 0.0f;
    p0->_d_out_x = -p1->_d_in_x + 2 * p1->_x - 2 * p0->_x;
    p0->_d_out_y = -p1->_d_in_y + 2 * p1->_y - 2 * p0->_y;
    p0->_d_out_z = -p1->_d_in_z + 2 * p1->_z - 2 * p0->_z;

    //  2 last points
    p0 = pointAt( catmullData, catmullData->_nPoints-2);
    p1 = pointAt( catmullData, catmullData->_nPoints-1);

    p1->_d_in_x = -p0->_d_out_x + 2 * p1->_x - 2 * p0->_x;
    p1->_d_in_y = -p0->_d_out_y + 2 * p1->_y - 2 * p0->_y;
    p1->_d_in_z = -p0->_d_out_z + 2 * p1->_z - 2 * p0->_z;
    p1->_d_out_x = p1->_d_out_y = p1->_d_out_z = 0.0f;
  }
  else
  {
    // function has a single control point - reset all values.
    // set some default value to first point.
    p0 = pointAt( catmullData, 0 );
    p0->_d_in_x  = p0->_d_in_y = p0->_d_in_z = 0.0f;
    p0->_d_in_x  = p0->_d_in_y = p0->_d_in_z = 0.0f;
  }
}
