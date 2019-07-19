#include "TV_LinearCurveView.h"
#include "TR_DataObject.h"
#include "TR_NodeTree.h"

#include <cmath>

// #define DISABLE_DICHOTOMIC_SEARCH

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_LinearCurveView::Impl
#endif
class TV_LinearCurveView::Impl
{
  MEM_OVERRIDE

  friend class TV_LinearCurveView;

public:

  Impl() :
    _nodeTree(0),
    _linearCurveData(0),
    _firstPoint(0),
    _lastPoint(0)
  {
  }

  ~Impl()
  {
  }

private:

  const TR_NodeTree              *_nodeTree;

  const TR_LinearCurveDataObject *_linearCurveData;

  //  Often reused during calculation.
  const TR_LinearPointDataObject *_firstPoint;
  const TR_LinearPointDataObject *_lastPoint;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_LinearCurveView
#endif

TV_LinearCurveView::TV_LinearCurveView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef ) :
  TV_FloatDataView()
{
  _i = new Impl;
  _i->_nodeTree = nodeTree;

  _i->_linearCurveData = _i->_nodeTree->dataObject<TR_LinearCurveDataObject>( dataRef );
  if ( _i->_linearCurveData )
  {
    unsigned nPoints = _i->_linearCurveData->_nPoints;
    _i->_firstPoint = pointAt(0);
    _i->_lastPoint = pointAt(nPoints-1);
  }
}

TV_LinearCurveView::~TV_LinearCurveView()
{
  delete _i;
}

bool TV_LinearCurveView::getValue( float frame, float &value ) const
{
  unsigned nPoints = this->nPoints();
  if ( nPoints == 0 )
    return false;

  if ( !_i->_firstPoint || !_i->_lastPoint )
    return false;

  //  Only a single key frame in curve.
  if ( _i->_firstPoint == _i->_lastPoint )
  {
    value = _i->_firstPoint->_y;
    return true;
  }

  //  Before first key frame.
  if ( frame < _i->_firstPoint->_x )
  {
    value = _i->_firstPoint->_y;
    return true;
  }

  //  After last key frame.
  if ( frame > _i->_lastPoint->_x )
  {
    value = _i->_lastPoint->_y;
    return true;
  }

  const TR_LinearPointDataObject *leftPoint;
  const TR_LinearPointDataObject *rightPoint;
  if ( findSegment( frame, leftPoint, rightPoint ) )
  {
    if ( rightPoint == _i->_firstPoint )
    {
      value = rightPoint->_y;
    }
    else if ( leftPoint == rightPoint )
    {
      value = rightPoint->_y;
    }
    else
    {
      value = linearInterpol( (frame-leftPoint->_x) / (rightPoint->_x-leftPoint->_x), leftPoint->_y, rightPoint->_y );
    }

    return true;
  }

  return false;
}

unsigned TV_LinearCurveView::nPoints() const
{
  if ( _i->_linearCurveData == 0 )
    return 0;

  unsigned nPoints = _i->_linearCurveData->_nPoints;
  return nPoints;
}

const TR_LinearPointDataObject *TV_LinearCurveView::pointAt( unsigned idx ) const
{
  if ( _i->_linearCurveData == 0 )
    return 0;

  TR_Types::DataOffset_t offset = (TR_Types::DataOffset_t)(TR_DataObject::g_dataObjectSizeTable[TR_Types::eLinearCurveData] + (TR_DataObject::g_dataObjectSizeTable[TR_Types::eLinearPointData] * idx));
  TR_Types::DataRef_t dataRef = _i->_nodeTree->dataRef(_i->_linearCurveData, offset);

  return _i->_nodeTree->dataObject<TR_LinearPointDataObject>(dataRef);
}

bool TV_LinearCurveView::startIndex( float frame, unsigned &idx0, const TR_LinearPointDataObject *&pt0 ) const
{
  unsigned nPoints = this->nPoints();
#ifdef DISABLE_DICHOTOMIC_SEARCH
  unsigned nRecur = 0u;
#else
  unsigned nRecur = nPoints / 15;
#endif

  if (nRecur == 0u)
  {
    idx0 = 0u;
    pt0 = pointAt(idx0);

    return true;
  }

  idx0 = 0u;
  unsigned endIdx = nPoints;
  unsigned middleIdx;

  while( nRecur-- )
  {
    middleIdx = (idx0+endIdx) >> 1;

    pt0 = pointAt(middleIdx);
    if (pt0 == 0)
      return false;

    if (pt0->_x < frame)
      idx0 = middleIdx;
    else
      endIdx = middleIdx;
  }

  if (idx0 != middleIdx)
  {
    pt0 = pointAt(idx0);
  }

  return true;
}

bool TV_LinearCurveView::findSegment( float frame, const TR_LinearPointDataObject *&leftPoint, const TR_LinearPointDataObject *&rightPoint ) const
{
  unsigned nPoints = this->nPoints();
  if ( nPoints < 2 )
    return false;

  unsigned idx0;
  if (startIndex( frame, idx0, leftPoint ))
  {
    for ( unsigned idx = idx0+1 ; idx < nPoints ; ++idx )
    {
      rightPoint = pointAt(idx);
      //ASSERT(after != 0);

      if (rightPoint && (rightPoint->_x > frame))
        break;

      leftPoint = rightPoint;
    }
  }

  return true;
}

float TV_LinearCurveView::linearInterpol( const float &u, const float &a, const float &b ) const
{
  return(a + u * (b - a));
}

