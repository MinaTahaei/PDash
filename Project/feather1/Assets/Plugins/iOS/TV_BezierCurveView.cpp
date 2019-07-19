#include "TV_BezierCurveView.h"
#include "TR_DataObject.h"
#include "TR_NodeTree.h"

#include <cmath>

// #define DISABLE_DICHOTOMIC_SEARCH

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_BezierCurveView::Impl
#endif
class TV_BezierCurveView::Impl
{
  MEM_OVERRIDE

  friend class TV_BezierCurveView;

public:

  Impl() :
    _nodeTree(0),
    _bezierCurveData(0),
    _firstPoint(0),
    _lastPoint(0)
  {
  }

  ~Impl()
  {
  }

private:

  const TR_NodeTree              *_nodeTree;
  const TR_BezierCurveDataObject *_bezierCurveData;

  //  Often reused during calculation.
  const TR_BezierPointDataObject *_firstPoint;
  const TR_BezierPointDataObject *_lastPoint;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_BezierCurveView
#endif

TV_BezierCurveView::TV_BezierCurveView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef ) :
  TV_FloatDataView()
{
  _i = new Impl;
  _i->_nodeTree = nodeTree;

  _i->_bezierCurveData = _i->_nodeTree->dataObject<TR_BezierCurveDataObject>( dataRef );
  if ( _i->_bezierCurveData )
  {
    unsigned nPoints = _i->_bezierCurveData->_nPoints;
    _i->_firstPoint = pointAt(0);
    _i->_lastPoint = pointAt(nPoints-1);
  }
}

TV_BezierCurveView::~TV_BezierCurveView()
{
  delete _i;
}

bool TV_BezierCurveView::getValue( float frame, float &value ) const
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

  const TR_BezierPointDataObject *leftPoint;
  const TR_BezierPointDataObject *rightPoint;
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
    else if ( leftPoint->_constSeg )
    {
      value = leftPoint->_y;
    }
    else
    {
      float u = findU(frame, leftPoint, rightPoint);
      value = getValueY( u, leftPoint, rightPoint );
    }

    return true;
  }

  return false;
}

unsigned TV_BezierCurveView::nPoints() const
{
  if ( _i->_bezierCurveData == 0 )
    return 0;

  unsigned nPoints = _i->_bezierCurveData->_nPoints;
  return nPoints;
}

const TR_BezierPointDataObject *TV_BezierCurveView::pointAt( unsigned idx ) const
{
  if ( _i->_bezierCurveData == 0 )
    return 0;

  TR_Types::DataOffset_t offset = (TR_Types::DataOffset_t)(TR_DataObject::g_dataObjectSizeTable[TR_Types::eBezierCurveData] + (TR_DataObject::g_dataObjectSizeTable[TR_Types::eBezierPointData] * idx));
  TR_Types::DataRef_t dataRef = _i->_nodeTree->dataRef(_i->_bezierCurveData, offset);

  return _i->_nodeTree->dataObject<TR_BezierPointDataObject>(dataRef);
}

bool TV_BezierCurveView::startIndex( float frame, unsigned &idx0, const TR_BezierPointDataObject *&pt0 ) const
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

bool TV_BezierCurveView::findSegment( float frame, const TR_BezierPointDataObject *&leftPoint, const TR_BezierPointDataObject *&rightPoint ) const
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

float TV_BezierCurveView::linearInterpol( const float &u, const float &a, const float &b ) const
{
  return(a + u * (b - a));
}

float TV_BezierCurveView::findU( float x, const TR_BezierPointDataObject *leftPoint, const TR_BezierPointDataObject *rightPoint ) const
{
  const double EPSILON  = 5e-10;
  const int    MAX_ITER = 52;

  if (leftPoint->_x == x)
  {
    return(0.0);
  }
  else if (rightPoint->_x == x)
  {
    return(1.0);
  }
  else
  {
    int    i(0);
    float  u, v,
           u1(0.0),
           u2(1.0);

    do
    {
      u = 0.5f * (u1 + u2);
      v = getValueX(u, leftPoint, rightPoint);

      if (v < x)
      {
        u1 = u;
      }
      else
      {
        u2 = u;
      }
    }
    while((fabsf(v - x) > EPSILON) && (++i < MAX_ITER));

    return(u);
  }
}

float TV_BezierCurveView::getValueX( float u, const TR_BezierPointDataObject *leftPoint, const TR_BezierPointDataObject *rightPoint ) const
{
  float a,b,c,d,e,f;
  a = linearInterpol(u, leftPoint->_x,   leftPoint->_rightx);
  b = linearInterpol(u, leftPoint->_rightx,  rightPoint->_leftx);
  c = linearInterpol(u, rightPoint->_leftx, rightPoint->_x);
  d = linearInterpol(u, a, b);
  e = linearInterpol(u, b, c);
  f = linearInterpol(u, d, e);

  return f;
}

float TV_BezierCurveView::getValueY( float u, const TR_BezierPointDataObject *leftPoint, const TR_BezierPointDataObject *rightPoint ) const
{
  float a,b,c,d,e,f;

  a = linearInterpol(u, leftPoint->_y,   leftPoint->_righty);
  b = linearInterpol(u, leftPoint->_righty,  rightPoint->_lefty);
  c = linearInterpol(u, rightPoint->_lefty, rightPoint->_y);
  d = linearInterpol(u, a, b);
  e = linearInterpol(u, b, c);
  f = linearInterpol(u, d, e);

  return f;
}

