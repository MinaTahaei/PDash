#include "TV_NodeTreeView.h"
#include "TV_BezierCurveView.h"
#include "TV_LinearCurveView.h"
#include "TV_ConstantFloatDataView.h"
#include "TV_PivotDataView.h"
#include "TV_CatmullCurveView.h"

#include "TR_NodeTree.h"
#include "TV_Blending.h"

#include "MT_Matrix4x4.h"
#include "MT_Point3d.h"
#include "STD_Containers.h"
#include "STD_Types.h"

#include <math.h>

namespace
{
  const double PLANE_QUANTUM = 1.0/(1024.0*64.0);
}

#define  LOGNTV(...)  fprintf(stderr, "[%s] ", "NTV" ); fprintf(stderr, __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_NodeTreeView::Impl
#endif
class TV_NodeTreeView::Impl
{
  MEM_OVERRIDE

  friend class TV_NodeTreeView;
  friend class TV_NodeTreeView::BrotherIterator;

public:

  Impl() :
    _nodeTree(0),
    _currentNodeData(0),
    _cachedLocalMatrix(0),
    _cachedModelMatrix(0),
    _cachedRestStartMatrix(0),
    _cachedRestEndMatrix(0),
    _cachedDeformStartMatrix(0),
    _cachedDeformEndMatrix(0),
    _channelMask(eNodeMask)
  {
  }

  ~Impl()
  {
  }

private:

  //! Pointer to node tree.  Not owned by view structure.
  const TR_NodeTree *_nodeTree;
  const TR_NodeDataObject *_currentNodeData;

  //! Cached matrix calculations.
  typedef STD_Pair< float, Math::Matrix4x4 > CachedMatrix_t;

  mutable CachedMatrix_t *_cachedLocalMatrix;
  mutable CachedMatrix_t *_cachedModelMatrix;

  mutable CachedMatrix_t *_cachedRestStartMatrix;
  mutable CachedMatrix_t *_cachedRestEndMatrix;
  mutable CachedMatrix_t *_cachedDeformStartMatrix;
  mutable CachedMatrix_t *_cachedDeformEndMatrix;

  //! Strong references on float channel views.
  typedef STD_Map< TR_Types::CurveChannel_t, TV_FloatDataViewPtr_t > CachedFloatChannelCol_t;
  CachedFloatChannelCol_t _floatChannels;

  //! Strong references on pos3d channel views.
  typedef STD_Map< TR_Types::CurveChannel_t, TV_Pos3dDataViewPtr_t > CachedPos3dChannelCol_t;
  CachedPos3dChannelCol_t _pos3dChannels;

  //! Channel Mask
  enum ChannelMask
  {
    eNodeMask        = 0,
    eAnimatedMask    = 1<<0,
    eDeformMask      = 1<<1
  };

  //! For quick matrix calculations.
  ChannelMask _channelMask;

  //! Strong reference on parent node tree view.
  TV_NodeTreeViewPtr_t _parentNodeTreeView;

  //! Collection of effects applied to current node.
  TV_NodeTreeView::EffectCol_t _effects;

  //! Effect Id for matte hierarchy. 
  TR_Types::EffectId_t _matteId;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_NodeTreeView::ChildIterator
#endif
TV_NodeTreeView::BrotherIterator::BrotherIterator()
{
}

TV_NodeTreeView::BrotherIterator::BrotherIterator(TV_NodeTreeViewPtr_t nodeTreeView) :
  _nodeTreeView(nodeTreeView)
{
}

TV_NodeTreeView::BrotherIterator::~BrotherIterator()
{
}

TV_NodeTreeView::BrotherIterator &TV_NodeTreeView::BrotherIterator::operator++()
{
  if ( _nodeTreeView.isValid() && _nodeTreeView->isValid() )
  {
    TV_NodeTreeViewPtr_t parentNodeTreeView = _nodeTreeView->_i->_parentNodeTreeView;

    const TR_NodeDataObject *nodeData = _nodeTreeView->_i->_currentNodeData;
    const TR_NodeTree *nodeTree = _nodeTreeView->_i->_nodeTree;

    if ( nodeData->_brotherDataOffset != TR_Types::g_nullOffset )
    {
      const TR_DataObject *nextData = reinterpret_cast<const TR_DataObject*>(reinterpret_cast<const TR_NodeTree::Data_t*>(nodeData) + nodeData->_brotherDataOffset);

      if ( nextData->_id == TR_Types::eNodeData )
      {
        const TR_NodeDataObject *nextNodeData = static_cast<const TR_NodeDataObject*>(nextData);
        TR_Types::DataRef_t nodeRef = nodeTree->dataRef(nextNodeData);

        TV_EffectDataViewPtr_t effectDataView;
        if ( nextNodeData->_effectDataOffset != TR_Types::g_nullOffset )
        {
          TR_Types::DataRef_t effectRef = nodeRef + nextNodeData->_effectDataOffset;

          const TR_EffectDataObject *effectData = nodeTree->dataObject<TR_EffectDataObject>(effectRef);
          if ( effectData )
          {
            effectDataView = new TV_EffectDataView( nodeTree, effectRef, parentNodeTreeView );
          }
        }

        _nodeTreeView = TV_NodeTreeViewPtr_t(new TV_NodeTreeView( nodeTree, nodeRef, parentNodeTreeView, effectDataView, parentNodeTreeView->_i->_matteId));

        return *this;
      }
    }
  }

  //  Null pointer, end of iteration.
  _nodeTreeView = TV_NodeTreeViewPtr_t(0);

  return *this;
}

TV_NodeTreeView::BrotherIterator TV_NodeTreeView::BrotherIterator::operator++(int)
{
  BrotherIterator iteratorCopy = *this;
  operator++();
  return iteratorCopy;
}

bool TV_NodeTreeView::BrotherIterator::operator==(const BrotherIterator &it) const
{
  bool isNull1 = !_nodeTreeView.isValid() || (_nodeTreeView.isValid() && (_nodeTreeView->_i->_currentNodeData == 0));
  bool isNull2 = !it._nodeTreeView.isValid() || (it._nodeTreeView.isValid() && (it._nodeTreeView->_i->_currentNodeData == 0));

  if ( isNull1 != isNull2 )
    return false;

  if ( isNull1 && isNull2 )
    return true;

  return ( (_nodeTreeView->_i->_nodeTree == it._nodeTreeView->_i->_nodeTree) &&
           (_nodeTreeView->_i->_currentNodeData == it._nodeTreeView->_i->_currentNodeData) );
}

bool TV_NodeTreeView::BrotherIterator::operator!=(const BrotherIterator &it) const
{
  return !(this->operator==(it));
}

const TV_NodeTreeView &TV_NodeTreeView::BrotherIterator::operator*() const
{
  //ASSERT(_nodeTreeView.isValid())
  return *_nodeTreeView;
}

TV_NodeTreeView &TV_NodeTreeView::BrotherIterator::operator*()
{
  //ASSERT(_nodeTreeView.isValid())
  return *_nodeTreeView;
}

const TV_NodeTreeView *TV_NodeTreeView::BrotherIterator::operator->() const
{
  //ASSERT(_nodeTreeView.isValid())
  return _nodeTreeView.get();
}

TV_NodeTreeView *TV_NodeTreeView::BrotherIterator::operator->()
{
  //ASSERT(_nodeTreeView.isValid())
  return _nodeTreeView.get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_NodeTreeView
#endif

TV_NodeTreeViewPtr_t TV_NodeTreeView::create( const TR_NodeTree *nodeTree )
{
  TV_NodeTreeView *nodeTreeView = new TV_NodeTreeView( nodeTree, nodeTree->firstNodeDataRef(), TV_NodeTreeViewPtr_t(0) );
  return nodeTreeView;
}

TV_NodeTreeView::~TV_NodeTreeView()
{
  delete _i->_cachedLocalMatrix;
  delete _i->_cachedModelMatrix;

  delete _i->_cachedRestStartMatrix;
  delete _i->_cachedRestEndMatrix;
  delete _i->_cachedDeformStartMatrix;
  delete _i->_cachedDeformEndMatrix;

  delete _i;
}

bool TV_NodeTreeView::operator==(const TV_NodeTreeView &nodeTreeView) const
{
  if ( this != &nodeTreeView )
  {
    return ( _i->_currentNodeData == nodeTreeView._i->_currentNodeData );
  }

  return true;
}

bool TV_NodeTreeView::operator<(const TV_NodeTreeView &nodeTreeView) const
{
  return (_i->_currentNodeData < nodeTreeView._i->_currentNodeData);
}

bool TV_NodeTreeView::hasCommonRoot(const TV_NodeTreeView &nodeTreeView) const
{
  return (_i->_nodeTree == nodeTreeView._i->_nodeTree);
}

TV_NodeTreeViewPtr_t TV_NodeTreeView::parent() const
{
  return _i->_parentNodeTreeView;
}

TV_NodeTreeViewPtr_t TV_NodeTreeView::find( const STD_String &nodeName ) const
{
  if ( name().compare( nodeName ) == 0 )
    return TV_NodeTreeViewPtr_t(const_cast<TV_NodeTreeView*>(this));

  for ( BrotherIterator i = childBegin(), iEnd = childEnd() ; i!=iEnd ; ++i )
  {
    TV_NodeTreeViewPtr_t nodeTreeView = i->find( nodeName );
    if ( nodeTreeView.get() )
      return nodeTreeView;
  }

  return TV_NodeTreeViewPtr_t(0);
}

TV_NodeTreeView::BrotherIterator TV_NodeTreeView::childBegin() const
{
  if ( _i->_currentNodeData && (_i->_currentNodeData->_childDataOffset != TR_Types::g_nullOffset) )
  {
    TR_Types::DataRef_t nodeRef = _i->_nodeTree->dataRef(_i->_currentNodeData) + _i->_currentNodeData->_childDataOffset;
    const TR_DataObject *nextData = _i->_nodeTree->dataObject(nodeRef);

    if ( nextData->_id == TR_Types::eNodeData )
    {
      const TR_NodeDataObject *nextNodeData = static_cast<const TR_NodeDataObject*>(nextData);

      TV_EffectDataViewPtr_t effectDataView;
      if ( nextNodeData->_effectDataOffset != TR_Types::g_nullOffset )
      {
        TR_Types::DataRef_t effectRef = nodeRef + nextNodeData->_effectDataOffset;

        const TR_EffectDataObject *effectData = _i->_nodeTree->dataObject<TR_EffectDataObject>(effectRef);
        if ( effectData && (effectData->_id == TR_Types::eEffectData) )
        {
          effectDataView = new TV_EffectDataView( _i->_nodeTree, effectRef, const_cast<TV_NodeTreeView*>(this) );
        }
      }

      TV_NodeTreeViewPtr_t childNodeTreeView = new TV_NodeTreeView(_i->_nodeTree, nodeRef, const_cast<TV_NodeTreeView*>(this), effectDataView, _i->_matteId);

      return BrotherIterator(childNodeTreeView);
    }
  }

  return BrotherIterator();

}

TV_NodeTreeView::BrotherIterator TV_NodeTreeView::childEnd() const
{
  return BrotherIterator();
}

TV_NodeTreeView::TV_NodeTreeView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t nodeRef, TV_NodeTreeViewPtr_t parentNode, TV_EffectDataViewPtr_t effectData, TR_Types::EffectId_t matteId )
{
  _i = new Impl;

  _i->_nodeTree = nodeTree;
  _i->_parentNodeTreeView = parentNode;

  //  Retrieve common node data
  _i->_currentNodeData = _i->_nodeTree->dataObject<TR_NodeDataObject>(nodeRef);

  createChannels();

  //  Append current effect and parent effects into current container.
  if ( effectData.isValid() )
    _i->_effects.push_back( effectData );

  if ( parentNode.isValid() )
    _i->_effects.insert( _i->_effects.end(), parentNode->_i->_effects.begin(), parentNode->_i->_effects.end() );

  _i->_matteId = matteId;
}

bool TV_NodeTreeView::isValid() const
{
  return _i->_nodeTree && _i->_currentNodeData;
}

STD_String TV_NodeTreeView::name() const
{
  static STD_String g_nullName = "";

  if (!isValid())
    return g_nullName;

  STD_String nodeName;
  if ( _i->_nodeTree->queryNodeName( _i->_currentNodeData, nodeName ) )
  {
    return nodeName;
  }

  return g_nullName;
}

float TV_NodeTreeView::totalDuration() const
{
  float currentDuration = 0.0f;
  for ( unsigned idx=0 ; idx < nSprites() ; ++idx )
  {
    STD_String spriteName;
    float start, duration;
    if ( sprite(idx, spriteName, start, duration) )
    {
      float cumul = start + duration - 1;
      if ( cumul > currentDuration )
        currentDuration = cumul;
    }
  }

  for ( BrotherIterator i = childBegin(), iEnd = childEnd() ; i!=iEnd ; ++i )
  {
    float childDuration = i->totalDuration();
    if ( childDuration > currentDuration )
      currentDuration = childDuration;
  }

  return currentDuration;
}

unsigned TV_NodeTreeView::depth() const
{
  unsigned depth = 0u;
  for ( TV_NodeTreeViewPtr_t itv = _i->_parentNodeTreeView ; itv.get() ; itv = itv->parent() )
  {
    ++depth;
  }

  return depth;
}

bool TV_NodeTreeView::hasAnimation() const
{
  return _i->_currentNodeData && (_i->_currentNodeData->_channelDataOffset != TR_Types::g_nullOffset);
}

bool TV_NodeTreeView::hasDeformation() const
{
  return hasAnimation() && (_i->_channelMask & Impl::eDeformMask);
}

TV_FloatDataViewPtr_t TV_NodeTreeView::separateX() const
{
  return floatChannel(TR_Types::eSeparateX);
}

TV_FloatDataViewPtr_t TV_NodeTreeView::separateY() const
{
  return floatChannel(TR_Types::eSeparateY);
}

TV_FloatDataViewPtr_t TV_NodeTreeView::separateZ() const
{
  return floatChannel(TR_Types::eSeparateZ);
}

TV_Pos3dDataViewPtr_t TV_NodeTreeView::position() const
{
  return pos3dChannel(TR_Types::eXYZ);
}

TV_FloatDataViewPtr_t TV_NodeTreeView::scaleX() const
{
  return floatChannel(TR_Types::eScaleX);
}

TV_FloatDataViewPtr_t TV_NodeTreeView::scaleY() const
{
  return floatChannel(TR_Types::eScaleY);
}

TV_FloatDataViewPtr_t TV_NodeTreeView::scaleXY() const
{
  return floatChannel(TR_Types::eScaleXY);
}

TV_FloatDataViewPtr_t TV_NodeTreeView::rotationZ() const
{
  return floatChannel(TR_Types::eRotationZ);
}

TV_FloatDataViewPtr_t TV_NodeTreeView::skew() const
{
  return floatChannel(TR_Types::eSkew);
}

TV_Pos3dDataViewPtr_t TV_NodeTreeView::pivot() const
{
  return pos3dChannel(TR_Types::ePivot);
}

TV_FloatDataViewPtr_t TV_NodeTreeView::opacity() const
{
  return floatChannel(TR_Types::eOpacity);
}

void TV_NodeTreeView::position(float frame, float &offsetx, float &offsety, float &offsetz) const
{
  offsetx = offsety = offsetz = 0.0f;

  TV_Pos3dDataViewPtr_t xyzData = this->position();
  if (xyzData.get())
  {
    xyzData->getValue( frame, offsetx, offsety, offsetz );
  }
  else
  {
    TV_FloatDataViewPtr_t separateXData = this->separateX();
    TV_FloatDataViewPtr_t separateYData = this->separateY();
    TV_FloatDataViewPtr_t separateZData = this->separateZ();

    if (separateXData.get())
      separateXData->getValue( frame, offsetx );
    if (separateYData.get())
      separateYData->getValue( frame, offsety );
    if (separateZData.get())
      separateZData->getValue( frame, offsetz );
  }
}

void TV_NodeTreeView::scale(float frame, float &scalex, float &scaley) const
{
  scalex = scaley = 1.0f;

  TV_FloatDataViewPtr_t scaleXYData = this->scaleXY();
  if (scaleXYData.get())
  {
    scaleXYData->getValue(frame, scalex);
    scaley = scalex;
  }
  else
  {
    TV_FloatDataViewPtr_t scaleXData = this->scaleX();
    TV_FloatDataViewPtr_t scaleYData = this->scaleY();
    if (scaleXData.get())
      scaleXData->getValue(frame, scalex);
    if (scaleYData.get())
      scaleYData->getValue(frame, scaley);
  }
}

void TV_NodeTreeView::rotation(float frame, float &rotationz) const
{
  rotationz = 0.0f;

  TV_FloatDataViewPtr_t rotationZData = this->rotationZ();
  if (rotationZData.get())
    rotationZData->getValue(frame, rotationz);
}

void TV_NodeTreeView::skew(float frame, float &skew) const
{
  skew = 0.0f;

  TV_FloatDataViewPtr_t skewData = this->skew();
  if (skewData.get())
    skewData->getValue(frame, skew);
}

void TV_NodeTreeView::pivot(float frame, float &pivotx, float &pivoty, float &pivotz) const
{
  pivotx = pivoty = pivotz = 0.0f;

  TV_Pos3dDataViewPtr_t pivotData = this->pivot();
  if (pivotData.get())
    pivotData->getValue( frame, pivotx, pivoty, pivotz );
}

void TV_NodeTreeView::opacity(float frame, float &opacity) const
{
  opacity = 1.0f;

  TV_FloatDataViewPtr_t opacityData = this->opacity();
  if (opacityData.get())
    opacityData->getValue(frame, opacity);
}

const Math::Matrix4x4 &TV_NodeTreeView::localMatrix( float frame ) const
{
  //  Use matrix from cache if already calculated.
  if ( _i->_cachedLocalMatrix )
  {
    if (_i->_cachedLocalMatrix->first == frame)
      return _i->_cachedLocalMatrix->second;
  }
  else
  {
    _i->_cachedLocalMatrix = new Impl::CachedMatrix_t;
  }

  _i->_cachedLocalMatrix->first = frame;
  Math::Matrix4x4 &matrix = _i->_cachedLocalMatrix->second;

  matrix.clear();

  //  Node contains animated node channel data.
  if ( _i->_channelMask & Impl::eAnimatedMask )
  {
    float pivotX, pivotY, pivotZ;
    float offsetX, offsetY, offsetZ;
    float scaleX, scaleY;
    float rotationZ;
    float skew;

    this->pivot(frame, pivotX, pivotY, pivotZ);
    this->position(frame, offsetX, offsetY, offsetZ);
    this->scale(frame, scaleX, scaleY);
    this->rotation(frame, rotationZ);
    this->skew(frame, skew);

    double roundOffsetZ = floor( ( (double)offsetZ / PLANE_QUANTUM ) + 0.5 ) * PLANE_QUANTUM;

    matrix.translate( pivotX + offsetX, pivotY + offsetY, pivotZ + roundOffsetZ );
    matrix.rotateRadians( rotationZ );
    matrix.skew( skew );
    matrix.scale( scaleX, scaleY );
    matrix.translate( -pivotX, -pivotY, -pivotZ );
  }

  //  Node contains deform node channel data.
  if ( _i->_channelMask & Impl::eDeformMask )
  {
    float restOffsetX, restOffsetY;
    float restRotationZ;
    float restLength;

    float deformOffsetX, deformOffsetY;
    float deformRotationZ;
    float deformLength;

    this->restOffset(frame, restOffsetX, restOffsetY);
    this->restRotation(frame, restRotationZ);
    this->restLength(frame, restLength);

    this->deformOffset(frame, deformOffsetX, deformOffsetY);
    this->deformRotation(frame, deformRotationZ);
    this->deformLength(frame, deformLength);

    matrix.translate(deformOffsetX, deformOffsetY);
    matrix.rotateRadians(deformRotationZ);
    matrix.translate(deformLength-restLength, 0, 0);
    matrix.rotateRadians(-restRotationZ);
    matrix.translate(-restOffsetX, -restOffsetY);

    for ( TV_NodeTreeViewPtr_t itv = _i->_parentNodeTreeView ; itv.get() ; itv = itv->parent() )
    {
      //  Iterate upward on deformation chain.
      //  Stop once there is no more bones in the chain.
      if ( (itv->_i->_channelMask & Impl::eDeformMask) == 0 )
        break;

      Math::Matrix4x4 preMatrix, postMatrix;

      itv->restOffset(frame, restOffsetX, restOffsetY);
      itv->restRotation(frame, restRotationZ);
      itv->restLength(frame, restLength);

      preMatrix.translate( restOffsetX, restOffsetY );
      preMatrix.rotateRadians( restRotationZ );
      preMatrix.translate( restLength, 0 );

      postMatrix.translate( -restLength, 0 );
      postMatrix.rotateRadians( -restRotationZ );
      postMatrix.translate( -restOffsetX, -restOffsetY );

      matrix = preMatrix * matrix * postMatrix;
    }
  }

  return matrix;
}



const Math::Matrix4x4 &TV_NodeTreeView::localMatrixWithBlending( float frameFrom, float frameTo, float fullBlendTime, float currentBlendTime, int blendID ) const
{
	STD_String nodeName = name();
	TV_NodeTreeViewPtr_t nodeTreeViewToBlendTo = SingletonBlending::getInstance()->BlendingContainer[blendID][nodeName];
  //  Use matrix from cache if already calculated.
  if ( _i->_cachedLocalMatrix )
  {
    if (_i->_cachedLocalMatrix->first == frameFrom)
      return _i->_cachedLocalMatrix->second;
  }
  else
  {
    _i->_cachedLocalMatrix = new Impl::CachedMatrix_t;
  }

  _i->_cachedLocalMatrix->first = frameFrom;
  Math::Matrix4x4 &matrix = _i->_cachedLocalMatrix->second;
  matrix.clear();

  //  Node contains animated node channel data.
  if ( _i->_channelMask & Impl::eAnimatedMask )
  {
    float pivotX, pivotY, pivotZ;
    float offsetX, offsetY, offsetZ;
    float scaleX, scaleY;
    float rotationZ;
    float skew;

	this->pivot(frameFrom, pivotX, pivotY, pivotZ);
	this->position(frameFrom, offsetX, offsetY, offsetZ);
	this->scale(frameFrom, scaleX, scaleY);
	this->rotation(frameFrom, rotationZ);
	this->skew(frameFrom, skew);

	float pivotXBlendTo, pivotYBlendTo, pivotZBlendTo;
    float offsetXBlendTo, offsetYBlendTo, offsetZBlendTo;
    float scaleXBlendTo, scaleYBlendTo;
    float rotationZBlendTo;
    float skewBlendTo;

	pivotXBlendTo = pivotYBlendTo = pivotZBlendTo = offsetXBlendTo = offsetYBlendTo = offsetZBlendTo = scaleXBlendTo = scaleYBlendTo = rotationZBlendTo = skewBlendTo = 0.0;

	if (nodeTreeViewToBlendTo){
			nodeTreeViewToBlendTo->pivot(frameTo, pivotXBlendTo, pivotYBlendTo, pivotZBlendTo);
			nodeTreeViewToBlendTo->position(frameTo, offsetXBlendTo, offsetYBlendTo, offsetZBlendTo);
			nodeTreeViewToBlendTo->scale(frameTo, scaleXBlendTo, scaleYBlendTo);
			nodeTreeViewToBlendTo->rotation(frameTo, rotationZBlendTo);
			nodeTreeViewToBlendTo->skew(frameTo, skewBlendTo);
	}

	pivotX = calculateNewBlendedValue(pivotX,pivotXBlendTo, currentBlendTime, fullBlendTime);
	pivotY = calculateNewBlendedValue(pivotY,pivotYBlendTo, currentBlendTime, fullBlendTime);
	pivotZ = calculateNewBlendedValue(pivotZ,pivotZBlendTo, currentBlendTime, fullBlendTime);

	offsetX = calculateNewBlendedValue(offsetX,offsetXBlendTo, currentBlendTime, fullBlendTime);
	offsetY = calculateNewBlendedValue(offsetY,offsetYBlendTo, currentBlendTime, fullBlendTime);
	offsetZ = calculateNewBlendedValue(offsetZ,offsetZBlendTo, currentBlendTime, fullBlendTime);

	scaleX = calculateNewBlendedValue(scaleX,scaleXBlendTo, currentBlendTime, fullBlendTime);
	scaleY = calculateNewBlendedValue(scaleY,scaleYBlendTo, currentBlendTime, fullBlendTime);

	rotationZ = calculateNewBlendedValue(rotationZ,rotationZBlendTo, currentBlendTime, fullBlendTime, true);

	skew = calculateNewBlendedValue(skew,skewBlendTo, currentBlendTime, fullBlendTime , false);

    double roundOffsetZ = floor( ( (double)offsetZ / PLANE_QUANTUM ) + 0.5 ) * PLANE_QUANTUM;

    matrix.translate( pivotX + offsetX, pivotY + offsetY, pivotZ + roundOffsetZ );
    matrix.rotateRadians( rotationZ );
    matrix.skew( skew );
    matrix.scale( scaleX, scaleY );
    matrix.translate( -pivotX, -pivotY, -pivotZ );
  }

  //  Node contains deform node channel data.
  if ( _i->_channelMask & Impl::eDeformMask)
  {
    float restOffsetX, restOffsetY;
    float restRotationZ;
    float restLength;

    float deformOffsetX, deformOffsetY;
    float deformRotationZ;
    float deformLength;

	this->restOffset(frameFrom, restOffsetX, restOffsetY);
    this->restRotation(frameFrom, restRotationZ);
    this->restLength(frameFrom, restLength);

    this->deformOffset(frameFrom, deformOffsetX, deformOffsetY);
    this->deformRotation(frameFrom, deformRotationZ);
    this->deformLength(frameFrom, deformLength);

	///

	float restOffsetXBlendTo, restOffsetYBlendTo;
    float restRotationZBlendTo;
    float restLengthBlendTo;

    float deformOffsetXBlendTo, deformOffsetYBlendTo;
    float deformRotationZBlendTo;
    float deformLengthBlendTo;

	restOffsetXBlendTo = restOffsetYBlendTo = restRotationZBlendTo = restLengthBlendTo = 0.0;
	deformOffsetXBlendTo = deformOffsetYBlendTo = deformRotationZBlendTo = deformLengthBlendTo = 0.0;

	if (nodeTreeViewToBlendTo){
			nodeTreeViewToBlendTo->restOffset(frameTo, restOffsetXBlendTo, restOffsetYBlendTo);
			nodeTreeViewToBlendTo->restRotation(frameTo, restRotationZBlendTo);
			nodeTreeViewToBlendTo->restLength(frameTo, restLengthBlendTo);

			nodeTreeViewToBlendTo->deformOffset(frameTo, deformOffsetXBlendTo, deformOffsetYBlendTo);
			nodeTreeViewToBlendTo->deformRotation(frameTo, deformRotationZBlendTo);
			nodeTreeViewToBlendTo->deformLength(frameTo, deformLengthBlendTo);
	}

	///
	restOffsetX = calculateNewBlendedValue(restOffsetX,restOffsetXBlendTo, currentBlendTime, fullBlendTime);
	restOffsetY = calculateNewBlendedValue(restOffsetY,restOffsetYBlendTo, currentBlendTime, fullBlendTime);

	restRotationZ = calculateNewBlendedValue(restRotationZ,restRotationZBlendTo, currentBlendTime, fullBlendTime,true);

	restLength = calculateNewBlendedValue(restLength,restLengthBlendTo, currentBlendTime, fullBlendTime);
	///

	deformOffsetX = calculateNewBlendedValue(deformOffsetX,deformOffsetXBlendTo, currentBlendTime, fullBlendTime);
	deformOffsetY = calculateNewBlendedValue(deformOffsetY,deformOffsetYBlendTo, currentBlendTime, fullBlendTime);

	deformRotationZ = calculateNewBlendedValue(deformRotationZ,deformRotationZBlendTo, currentBlendTime, fullBlendTime,true);

	deformLength = calculateNewBlendedValue(deformLength,deformLengthBlendTo, currentBlendTime, fullBlendTime);

    matrix.translate(deformOffsetX, deformOffsetY);
    matrix.rotateRadians(deformRotationZ);
    matrix.translate(deformLength-restLength, 0, 0);
    matrix.rotateRadians(-restRotationZ);
    matrix.translate(-restOffsetX, -restOffsetY);


	TV_NodeTreeViewPtr_t itvToBlendToNode = 0;
	STD_String itvToBlendToName = "";
    for ( TV_NodeTreeViewPtr_t itv = _i->_parentNodeTreeView ; itv.get() ; itv = itv->parent() )
    {
      //  Iterate upward on deformation chain.
      //  Stop once there is no more bones in the chain.
      if ( (itv->_i->_channelMask & Impl::eDeformMask) == 0 )
        break;

      Math::Matrix4x4 preMatrix, postMatrix;

      itv->restOffset(frameFrom, restOffsetX, restOffsetY);
      itv->restRotation(frameFrom, restRotationZ);
      itv->restLength(frameFrom, restLength);

	 // 
		itvToBlendToName = itv->name();
		itvToBlendToNode = SingletonBlending::getInstance()->BlendingContainer[blendID][itvToBlendToName];

		restOffsetXBlendTo = restOffsetYBlendTo = restRotationZBlendTo = restLengthBlendTo = 0.0;
		deformOffsetXBlendTo = deformOffsetYBlendTo = deformRotationZBlendTo = deformLengthBlendTo = 0.0;

		if (itvToBlendToNode){
			itvToBlendToNode->restOffset(frameTo, restOffsetXBlendTo, restOffsetYBlendTo);
			itvToBlendToNode->restRotation(frameTo, restRotationZBlendTo);
			itvToBlendToNode->restLength(frameTo, restLengthBlendTo);
		}

		restOffsetX = calculateNewBlendedValue(restOffsetX,restOffsetXBlendTo, currentBlendTime, fullBlendTime);
		restOffsetY = calculateNewBlendedValue(restOffsetY,restOffsetYBlendTo, currentBlendTime, fullBlendTime);

		restRotationZ = calculateNewBlendedValue(restRotationZ,restRotationZBlendTo, currentBlendTime, fullBlendTime,true);

		restLength = calculateNewBlendedValue(restLength,restLengthBlendTo, currentBlendTime, fullBlendTime);


      preMatrix.translate( restOffsetX, restOffsetY );
      preMatrix.rotateRadians( restRotationZ );
      preMatrix.translate( restLength, 0 );

      postMatrix.translate( -restLength, 0 );
      postMatrix.rotateRadians( -restRotationZ );
      postMatrix.translate( -restOffsetX, -restOffsetY );

      matrix = preMatrix * matrix * postMatrix;
    }
  }

  return matrix;
}

const Math::Matrix4x4 &TV_NodeTreeView::modelMatrix( float frame ) const
{
  //  Use matrix from cache if already calculated.
  if ( _i->_cachedModelMatrix )
  {
    if (_i->_cachedModelMatrix->first == frame)
      return _i->_cachedModelMatrix->second;
  }
  else
  {
    _i->_cachedModelMatrix = new Impl::CachedMatrix_t;
  }

  _i->_cachedModelMatrix->first = frame;
  Math::Matrix4x4 &modelMatrix = _i->_cachedModelMatrix->second;

  if ( _i->_parentNodeTreeView.get() )
  {
    modelMatrix = localMatrix(frame);

    //  Recursively multiply local matrices.  Avoid recursive functions by using local matrix.
    for ( TV_NodeTreeViewPtr_t itv = _i->_parentNodeTreeView ; itv.get() ; itv = itv->parent() )
    {
      modelMatrix = itv->localMatrix(frame) * modelMatrix;
    }

    //  Multiply with parent node tree view model matrix.  Now that model matrix is cached,
    //  the recursive function will quickly return a cached results most of the time.
    //modelMatrix = _i->_parentNodeTreeView->modelMatrix(frame) * localMatrix(frame);

    return modelMatrix;
  }

  modelMatrix = localMatrix(frame);
  return modelMatrix;
}

const Math::Matrix4x4 &TV_NodeTreeView::modelMatrixWithBlending( float frameFrom, float frameTo, float fullBlendTime, float currentBlendTime, int blendID ) const
{
  ////  Use matrix from cache if already calculated.
  if ( _i->_cachedModelMatrix )
  {
    if (_i->_cachedModelMatrix->first == frameFrom)
      return _i->_cachedModelMatrix->second;
  }
  else
  {
    _i->_cachedModelMatrix = new Impl::CachedMatrix_t;
  }

  _i->_cachedModelMatrix->first = frameFrom;
  Math::Matrix4x4 &modelMatrix = _i->_cachedModelMatrix->second;

  if ( _i->_parentNodeTreeView.get() )
  {
    modelMatrix = localMatrixWithBlending(frameFrom,frameTo,fullBlendTime,currentBlendTime,blendID);

    //  Recursively multiply local matrices.  Avoid recursive functions by using local matrix.
    for ( TV_NodeTreeViewPtr_t itv = _i->_parentNodeTreeView ; itv.get() ; itv = itv->parent() )
    {
      modelMatrix = itv->localMatrixWithBlending(frameFrom,frameTo,fullBlendTime,currentBlendTime,blendID) * modelMatrix;
    }

    //  Multiply with parent node tree view model matrix.  Now that model matrix is cached,
    //  the recursive function will quickly return a cached results most of the time.
    //modelMatrix = _i->_parentNodeTreeView->modelMatrix(frame) * localMatrix(frame);

    return modelMatrix;
  }

  modelMatrix = localMatrixWithBlending(frameFrom,frameTo,fullBlendTime,currentBlendTime,blendID);
  return modelMatrix;
}

bool TV_NodeTreeView::hasDrawingAnimation() const
{
  return (drawingAnimationData() != 0);
}

bool TV_NodeTreeView::spriteSheet( STD_String &spriteSheetName ) const
{
  if (!isValid())
    return false;

  spriteSheetName = _i->_nodeTree->spriteSheetName();
  return true;
}

unsigned TV_NodeTreeView::nSprites() const
{
  if (!isValid())
    return 0u;

  const TR_DrawingAnimationDataObject *drawingAnimationData = this->drawingAnimationData();
  if (drawingAnimationData == 0)
    return 0;

  return drawingAnimationData->_nDrawings;
}

bool TV_NodeTreeView::sprite( unsigned idx, STD_String &spriteName, float &start, float &duration ) const
{
  unsigned nSprites = this->nSprites();
  if (nSprites == 0)
    return false;

  const TR_DrawingDataObject *drawingData = drawingAt(idx);
  if (drawingData)
  {
    if ( _i->_nodeTree->querySpriteName( drawingData, spriteName ) )
    {
      start = drawingData->_frame;
      duration = drawingData->_repeat;
      return true;
    }
  }

  return false;
}

bool TV_NodeTreeView::sprite( float frame, STD_String &spriteName ) const
{
  if (!isValid())
    return false;

  const TR_DrawingDataObject *drawingData = drawingAt(frame);

  if ( drawingData )
  {
    if ( _i->_nodeTree->querySpriteName( drawingData, spriteName ) )
    {
      return true;
    }
  }

  return false;
}

TV_NodeTreeView::EffectIterator TV_NodeTreeView::effectBegin() const
{
  return _i->_effects.begin();
}

TV_NodeTreeView::EffectIterator TV_NodeTreeView::effectEnd() const
{
  return _i->_effects.end();
}

void TV_NodeTreeView::restOffset( float frame, float &offsetx, float &offsety ) const
{
  offsetx = offsety = 0.0f;

  TV_FloatDataViewPtr_t restOffsetXData = floatChannel(TR_Types::eRestOffsetX);
  TV_FloatDataViewPtr_t restOffsetYData = floatChannel(TR_Types::eRestOffsetY);

  if (restOffsetXData.get())
    restOffsetXData->getValue( frame, offsetx );
  if (restOffsetYData.get())
    restOffsetYData->getValue( frame, offsety );
}

void TV_NodeTreeView::restRotation( float frame, float &rotationz ) const
{
  rotationz = 0.0f;

  TV_FloatDataViewPtr_t restRotationData = floatChannel(TR_Types::eRestRotation);
  if (restRotationData.get())
    restRotationData->getValue(frame, rotationz);
}

void TV_NodeTreeView::restLength( float frame, float &length ) const
{
  length = 0.0f;

  TV_FloatDataViewPtr_t restLengthData = floatChannel(TR_Types::eRestLength);
  if (restLengthData.get())
    restLengthData->getValue(frame, length);
}

void TV_NodeTreeView::deformOffset( float frame, float &offsetx, float &offsety ) const
{
  offsetx = offsety = 0.0f;

  TV_FloatDataViewPtr_t deformOffsetXData = floatChannel(TR_Types::eDeformOffsetX);
  TV_FloatDataViewPtr_t deformOffsetYData = floatChannel(TR_Types::eDeformOffsetY);

  if (deformOffsetXData.get())
    deformOffsetXData->getValue( frame, offsetx );
  if (deformOffsetYData.get())
    deformOffsetYData->getValue( frame, offsety );
}

void TV_NodeTreeView::deformRotation( float frame, float &rotationz ) const
{
  rotationz = 0.0f;

  TV_FloatDataViewPtr_t deformRotationData = floatChannel(TR_Types::eDeformRotation);
  if (deformRotationData.get())
    deformRotationData->getValue(frame, rotationz);
}

void TV_NodeTreeView::deformLength( float frame, float &length ) const
{
  length = 0.0f;

  TV_FloatDataViewPtr_t deformLengthData = floatChannel(TR_Types::eDeformLength);
  if (deformLengthData.get())
    deformLengthData->getValue(frame, length);
}

const Math::Matrix4x4 &TV_NodeTreeView::restStartMatrix( float frame ) const
{
  //  Use matrix from cache if already calculated.
  if ( _i->_cachedRestStartMatrix )
  {
    if (_i->_cachedRestStartMatrix->first == frame)
      return _i->_cachedRestStartMatrix->second;
  }
  else
  {
    _i->_cachedRestStartMatrix = new Impl::CachedMatrix_t;
  }

  //  Calculate new matrix.
  _i->_cachedRestStartMatrix->first = frame;
  Math::Matrix4x4 &matrix = _i->_cachedRestStartMatrix->second;

  //  Current hierarchy is the matte hierarchy of a deformation effect.
  bool inDeformation = (_i->_matteId == TR_Types::eDeformation);

  //  Node containing deformation data.
  if ( inDeformation )
  {
    float offsetx, offsety;
    restOffset(frame, offsetx, offsety);

    float rotationz;
    restRotation(frame, rotationz);

    matrix.clear();
    matrix.translate(offsetx, offsety);
    matrix.rotateRadians(rotationz);

    if ( _i->_parentNodeTreeView.get() )
    {
      matrix = _i->_parentNodeTreeView->restEndMatrix(frame) * matrix;
    }
  }
  //  Regular animated node.
  else
  {
    matrix = modelMatrix(frame);
  }

  return matrix;
}

const Math::Matrix4x4 &TV_NodeTreeView::restEndMatrix( float frame ) const
{
  //  Use matrix from cache if already calculated.
  if ( _i->_cachedRestEndMatrix )
  {
    if (_i->_cachedRestEndMatrix->first == frame)
      return _i->_cachedRestEndMatrix->second;
  }
  else
  {
    _i->_cachedRestEndMatrix = new Impl::CachedMatrix_t;
  }

  //  Calculate new matrix.
  _i->_cachedRestEndMatrix->first = frame;
  Math::Matrix4x4 &matrix = _i->_cachedRestEndMatrix->second;

  matrix = restStartMatrix(frame);

  //  Current hierarchy is the matte hierarchy of a deformation effect.
  bool inDeformation = (_i->_matteId == TR_Types::eDeformation);

  //  Node containing deformation data.
  if ( inDeformation )
  {
    float length;
    restLength(frame, length);

    matrix.translate( length, 0, 0 );
  }

  return matrix;
}

const Math::Matrix4x4 &TV_NodeTreeView::deformStartMatrix( float frame ) const
{
  //  Use matrix from cache if already calculated.
  if ( _i->_cachedDeformStartMatrix )
  {
    if (_i->_cachedDeformStartMatrix->first == frame)
      return _i->_cachedDeformStartMatrix->second;
  }
  else
  {
    _i->_cachedDeformStartMatrix = new Impl::CachedMatrix_t;
  }

  //  Calculate new matrix.
  _i->_cachedDeformStartMatrix->first = frame;
  Math::Matrix4x4 &matrix = _i->_cachedDeformStartMatrix->second;

  //  Current hierarchy is the matte hierarchy of a deformation effect.
  //bool inDeformation = (_i->_matteId == TR_Types::eDeformation);

  //  Node containing deformation data.
  if ( _i->_channelMask & Impl::eDeformMask )
  {
    float offsetx, offsety;
    deformOffset(frame, offsetx, offsety);

    float rotationz;
    deformRotation(frame, rotationz);

    matrix.clear();
    matrix.translate(offsetx, offsety);
    matrix.rotateRadians(rotationz);

    if ( _i->_parentNodeTreeView.get() )
    {
      matrix = _i->_parentNodeTreeView->deformEndMatrix(frame) * matrix;
    }
  }
  //  Regular animated node.  Forward calculations to modelMatrix.  
  else
  {
    matrix = modelMatrix(frame);
  }

  return matrix;
}

const Math::Matrix4x4 &TV_NodeTreeView::deformEndMatrix( float frame ) const
{
  //  Use matrix from cache if already calculated.
  if ( _i->_cachedDeformEndMatrix )
  {
    if (_i->_cachedDeformEndMatrix->first == frame)
      return _i->_cachedDeformEndMatrix->second;
  }
  else
  {
    _i->_cachedDeformEndMatrix = new Impl::CachedMatrix_t;
  }

  //  Calculate new matrix.
  _i->_cachedDeformEndMatrix->first = frame;
  Math::Matrix4x4 &matrix = _i->_cachedDeformEndMatrix->second;

  matrix = deformStartMatrix(frame);

  //  Current hierarchy is the matte hierarchy of a deformation effect.
  //bool inDeformation = (_i->_matteId == TR_Types::eDeformation);

  //  Node containing deformation data.
  if ( _i->_channelMask & Impl::eDeformMask )
  {
    float length;
    deformLength(frame, length);

    matrix.translate( length, 0, 0 );
  }

  return matrix;
}

///for Blending///

const Math::Matrix4x4 &TV_NodeTreeView::restStartMatrixWithBlending( float frameFrom,float frameTo, float fullBlendTime, float currentBlendTime, int blendID) const
{
  //  Use matrix from cache if already calculated.
  if ( _i->_cachedRestStartMatrix )
  {
    if (_i->_cachedRestStartMatrix->first == frameFrom)
      return _i->_cachedRestStartMatrix->second;
  }
  else
  {
    _i->_cachedRestStartMatrix = new Impl::CachedMatrix_t;
  }

  //  Calculate new matrix.
  _i->_cachedRestStartMatrix->first = frameFrom;
  Math::Matrix4x4 &matrix = _i->_cachedRestStartMatrix->second;

  STD_String nodeName = name();
  TV_NodeTreeViewPtr_t nodeTreeViewToBlendTo = SingletonBlending::getInstance()->BlendingContainer[blendID][nodeName];

  //  Current hierarchy is the matte hierarchy of a deformation effect.
  bool inDeformation = (_i->_matteId == TR_Types::eDeformation);

  //  Node containing deformation data.
  if ( inDeformation )
  {
    float offsetx, offsety;
    restOffset(frameFrom, offsetx, offsety);

    float rotationz;
    restRotation(frameFrom, rotationz);

	//BlendTo
	float offsetxBlendTo, offsetyBlendTo;
	float rotationzBlendTo;
	offsetxBlendTo = offsetyBlendTo = rotationzBlendTo = 0.0f;
	if (nodeTreeViewToBlendTo){
		nodeTreeViewToBlendTo->restOffset(frameTo, offsetxBlendTo, offsetyBlendTo);
		nodeTreeViewToBlendTo->restRotation(frameTo, rotationzBlendTo);
	}

	offsetx = calculateNewBlendedValue(offsetx,offsetxBlendTo,currentBlendTime,fullBlendTime);
	offsety = calculateNewBlendedValue(offsety,offsetyBlendTo,currentBlendTime,fullBlendTime);

	rotationz = calculateNewBlendedValue(rotationz,rotationzBlendTo,currentBlendTime,fullBlendTime,true);
	//

    matrix.clear();
    matrix.translate(offsetx, offsety);
    matrix.rotateRadians(rotationz);

    if ( _i->_parentNodeTreeView.get() )
    {
      matrix = _i->_parentNodeTreeView->restEndMatrixWithBlending(frameFrom,frameTo,fullBlendTime,currentBlendTime,blendID) * matrix;
    }
  }
  //  Regular animated node.
  else
  {
	  matrix = modelMatrixWithBlending(frameFrom,frameTo,fullBlendTime,currentBlendTime,blendID);
    //matrix = modelMatrix(frame);
  }

  return matrix;
}

const Math::Matrix4x4 &TV_NodeTreeView::restEndMatrixWithBlending( float frameFrom,float frameTo, float fullBlendTime, float currentBlendTime, int blendID ) const
{
  //  Use matrix from cache if already calculated.
  if ( _i->_cachedRestEndMatrix )
  {
    if (_i->_cachedRestEndMatrix->first == frameFrom)
      return _i->_cachedRestEndMatrix->second;
  }
  else
  {
    _i->_cachedRestEndMatrix = new Impl::CachedMatrix_t;
  }

  //  Calculate new matrix.
  _i->_cachedRestEndMatrix->first = frameFrom;
  Math::Matrix4x4 &matrix = _i->_cachedRestEndMatrix->second;

  matrix = restStartMatrixWithBlending(frameFrom,frameTo,fullBlendTime,currentBlendTime,blendID);

  STD_String nodeName = name();
	TV_NodeTreeViewPtr_t nodeTreeViewToBlendTo = SingletonBlending::getInstance()->BlendingContainer[blendID][nodeName];

  //  Current hierarchy is the matte hierarchy of a deformation effect.
  bool inDeformation = (_i->_matteId == TR_Types::eDeformation);

  //  Node containing deformation data.
  if ( inDeformation )
  {
    float length;
    restLength(frameFrom, length);

	//BlendTo
	float lengthBlendTo = 0.0f;
	if (nodeTreeViewToBlendTo){
		nodeTreeViewToBlendTo->restLength(frameTo, lengthBlendTo);
	}

	length = calculateNewBlendedValue(length,lengthBlendTo,currentBlendTime,fullBlendTime);
	//

    matrix.translate( length, 0, 0 );
  }

  return matrix;
}

const Math::Matrix4x4 &TV_NodeTreeView::deformStartMatrixWithBlending( float frameFrom,float frameTo, float fullBlendTime, float currentBlendTime, int blendID ) const
{
  //  Use matrix from cache if already calculated.
  if ( _i->_cachedDeformStartMatrix )
  {
    if (_i->_cachedDeformStartMatrix->first == frameFrom)
      return _i->_cachedDeformStartMatrix->second;
  }
  else
  {
    _i->_cachedDeformStartMatrix = new Impl::CachedMatrix_t;
  }

  //  Calculate new matrix.
  _i->_cachedDeformStartMatrix->first = frameFrom;
  Math::Matrix4x4 &matrix = _i->_cachedDeformStartMatrix->second;

  //  Current hierarchy is the matte hierarchy of a deformation effect.
  //bool inDeformation = (_i->_matteId == TR_Types::eDeformation);

  STD_String nodeName = name();
	TV_NodeTreeViewPtr_t nodeTreeViewToBlendTo = SingletonBlending::getInstance()->BlendingContainer[blendID][nodeName];

  //  Node containing deformation data.
  if ( _i->_channelMask & Impl::eDeformMask)
  {
    float offsetx, offsety;
    deformOffset(frameFrom, offsetx, offsety);

    float rotationz;
    deformRotation(frameFrom, rotationz);

	//BlendTo
	float offsetxBlendTo, offsetyBlendTo;
	float rotationzBlendTo;

	offsetxBlendTo = offsetyBlendTo = rotationzBlendTo = 0.0f;

	if (nodeTreeViewToBlendTo){
		nodeTreeViewToBlendTo->deformOffset(frameTo, offsetxBlendTo, offsetyBlendTo);
		nodeTreeViewToBlendTo->deformRotation(frameTo, rotationzBlendTo);
	}

	offsetx = calculateNewBlendedValue(offsetx,offsetxBlendTo,currentBlendTime,fullBlendTime);
	offsety = calculateNewBlendedValue(offsety,offsetyBlendTo,currentBlendTime,fullBlendTime);
	rotationz = calculateNewBlendedValue(rotationz,rotationzBlendTo,currentBlendTime,fullBlendTime,true);
	//

    matrix.clear();
    matrix.translate(offsetx, offsety);
    matrix.rotateRadians(rotationz);

    if ( _i->_parentNodeTreeView.get() )
    {
      matrix = _i->_parentNodeTreeView->deformEndMatrixWithBlending(frameFrom,frameTo,fullBlendTime,currentBlendTime,blendID) * matrix;
    }
  }
  //  Regular animated node.  Forward calculations to modelMatrix.  
  else
  {
    matrix = modelMatrixWithBlending(frameFrom,frameTo,fullBlendTime,currentBlendTime,blendID);
  }

  return matrix;
}

const Math::Matrix4x4 &TV_NodeTreeView::deformEndMatrixWithBlending( float frameFrom,float frameTo, float fullBlendTime, float currentBlendTime, int blendID ) const
{
  //  Use matrix from cache if already calculated.
  if ( _i->_cachedDeformEndMatrix )
  {
    if (_i->_cachedDeformEndMatrix->first == frameFrom)
      return _i->_cachedDeformEndMatrix->second;
  }
  else
  {
    _i->_cachedDeformEndMatrix = new Impl::CachedMatrix_t;
  }

  //  Calculate new matrix.
  _i->_cachedDeformEndMatrix->first = frameFrom;
  Math::Matrix4x4 &matrix = _i->_cachedDeformEndMatrix->second;

  matrix = deformStartMatrixWithBlending(frameFrom,frameTo,fullBlendTime,currentBlendTime,blendID);

  //  Current hierarchy is the matte hierarchy of a deformation effect.
  //bool inDeformation = (_i->_matteId == TR_Types::eDeformation);

  	STD_String nodeName = name();
	TV_NodeTreeViewPtr_t nodeTreeViewToBlendTo = SingletonBlending::getInstance()->BlendingContainer[blendID][nodeName];

  //  Node containing deformation data.
  if ( _i->_channelMask & Impl::eDeformMask)
  {
    float length;
    deformLength(frameFrom, length);

	//BlendTo
	float lengthBlendTo = 0.0f;
	if (nodeTreeViewToBlendTo){
		nodeTreeViewToBlendTo->deformLength(frameTo, lengthBlendTo);
	}

	length = calculateNewBlendedValue(length,lengthBlendTo,currentBlendTime,fullBlendTime);

    matrix.translate( length, 0, 0 );
  }

  return matrix;
}

///------------///



void TV_NodeTreeView::createChannels()
{
  if (!isValid())
    return;

  if (_i->_currentNodeData->_channelDataOffset == TR_Types::g_nullOffset)
    return;

  //  Iterate on all channels and cache curve data view for quick use.
  TR_Types::DataRef_t dataRef = _i->_nodeTree->dataRef( _i->_currentNodeData );
  TR_Types::DataOffset_t dataOffset = _i->_currentNodeData->_channelDataOffset;

  while ( dataOffset != TR_Types::g_nullOffset )
  {
    dataRef = dataRef + dataOffset;

    const TR_ChannelDataObject *channelData = _i->_nodeTree->dataObject<TR_ChannelDataObject>(dataRef);
    if ( channelData )
    {
      if ( channelData->_linkedDataOffset != TR_Types::g_nullOffset )
      {
        createChannel( channelData->_channelType, dataRef + channelData->_linkedDataOffset );
      }

      dataOffset = channelData->_nextChannelDataOffset;
    }
    else
    {
      dataOffset = TR_Types::g_nullOffset;
    }
  }

  //  Once all channels have been created, set velocity in catmull data view if applicable.
  TV_Pos3dDataViewPtr_t xyzData = this->position();
  if (xyzData.get())
  {
    TV_FloatDataViewPtr_t velocityData = floatChannel(TR_Types::eVelocity);
    if (velocityData.get())
    {
      TV_CatmullCurveView *catmullData = static_cast<TV_CatmullCurveView*>(xyzData.get());
      catmullData->setVelocity(velocityData);
    }
  }
}

bool TV_NodeTreeView::createChannel( TR_Types::CurveChannel_t channelType, TR_Types::DataRef_t dataRef )
{
  const TR_DataObject *data = _i->_nodeTree->dataObject(dataRef);
  if ( data==0 )
    return false;

  //  Float channel
  TV_FloatDataView *floatDataView = 0;
  if ( data->_id == TR_Types::eBezierCurveData )
  {
    floatDataView = new TV_BezierCurveView(_i->_nodeTree, dataRef);
  }
  else if ( data->_id == TR_Types::eLinearCurveData )
  {
    floatDataView = new TV_LinearCurveView(_i->_nodeTree, dataRef);
  }
  else if ( data->_id == TR_Types::eFloatData )
  {
    floatDataView = new TV_ConstantFloatDataView(_i->_nodeTree, dataRef);
  }

  if ( floatDataView != 0 )
  {
    _i->_floatChannels.insert( STD_MakePair(channelType, floatDataView) );

    if ( (channelType >= TR_Types::eSeparateX) && (channelType <= TR_Types::ePivot) )
      _i->_channelMask = Impl::ChannelMask(_i->_channelMask | Impl::eAnimatedMask);
    else if ( (channelType >= TR_Types::eRestOffsetX) && (channelType <= TR_Types::eDeformRotation) )
      _i->_channelMask = Impl::ChannelMask(_i->_channelMask | Impl::eDeformMask);

    return true;
  }

  //  Pos3d channel
  TV_Pos3dDataView *pos3dDataView = 0;
  if ( data->_id == TR_Types::eCatmullCurveData )
  {
    pos3dDataView = new TV_CatmullCurveView(_i->_nodeTree, dataRef, TV_FloatDataViewPtr_t(0) /* empty velocity .. will be filled in later */ );
  }
  else if ( data->_id == TR_Types::eAnimatedPivotData )
  {
    pos3dDataView = new TV_PivotDataView(_i->_nodeTree, dataRef);
  }

  if ( pos3dDataView != 0 )
  {
    _i->_pos3dChannels.insert( STD_MakePair(channelType, pos3dDataView) );
    _i->_channelMask = Impl::ChannelMask(_i->_channelMask | Impl::eAnimatedMask);
    return true;
  }

  return false;
}

const TR_DrawingAnimationDataObject *TV_NodeTreeView::drawingAnimationData() const
{
  if (!isValid())
    return 0;

  if (_i->_currentNodeData->_drawingDataOffset == TR_Types::g_nullOffset)
    return 0;

  TR_Types::DataRef_t dataRef = _i->_nodeTree->dataRef(_i->_currentNodeData, _i->_currentNodeData->_drawingDataOffset);
  const TR_DrawingAnimationDataObject *drawingAnimationData = _i->_nodeTree->dataObject<TR_DrawingAnimationDataObject>(dataRef);
  return drawingAnimationData;
}

TV_FloatDataViewPtr_t TV_NodeTreeView::floatChannel( TR_Types::CurveChannel_t channel ) const
{
  Impl::CachedFloatChannelCol_t::const_iterator iChannel = _i->_floatChannels.find( channel );
  if ( iChannel != _i->_floatChannels.end() )
  {
    return iChannel->second;
  }

  return 0;
}

TV_Pos3dDataViewPtr_t TV_NodeTreeView::pos3dChannel( TR_Types::CurveChannel_t channel ) const
{
  Impl::CachedPos3dChannelCol_t::const_iterator iChannel = _i->_pos3dChannels.find( channel );
  if ( iChannel != _i->_pos3dChannels.end() )
  {
    return iChannel->second;
  }

  return 0;
}

const TR_DrawingDataObject *TV_NodeTreeView::drawingAt( float frame ) const
{
  const TR_DrawingAnimationDataObject *drawingAnimationData = this->drawingAnimationData();
  if (drawingAnimationData == 0)
    return 0;

  for ( unsigned idx = 0 ; idx < drawingAnimationData->_nDrawings ; ++idx )
  {
    const TR_DrawingDataObject *drawingData = drawingAt(idx);
    if (drawingData)
    {
      if ( frame >= drawingData->_frame )
      {
        if ( frame < (drawingData->_frame + drawingData->_repeat) )
          return drawingData;
      }
    }
  }

  return 0;
}

const TR_DrawingDataObject *TV_NodeTreeView::drawingAt( unsigned idx ) const
{
  const TR_DrawingAnimationDataObject *drawingAnimationData = this->drawingAnimationData();
  if (drawingAnimationData == 0)
    return 0;

  TR_Types::DataOffset_t offset = (TR_Types::DataOffset_t)
    ( TR_DataObject::g_dataObjectSizeTable[TR_Types::eDrawingAnimationData] +
      (TR_DataObject::g_dataObjectSizeTable[TR_Types::eDrawingData] * idx) );
  TR_Types::DataRef_t dataRef = _i->_nodeTree->dataRef(drawingAnimationData, offset);

  const TR_DrawingDataObject *drawingData = _i->_nodeTree->dataObject<TR_DrawingDataObject>(dataRef);
  return drawingData;
}
