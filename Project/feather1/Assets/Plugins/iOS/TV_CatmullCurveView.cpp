#include "TV_CatmullCurveView.h"
#include "TR_DataObject.h"
#include "TR_CatmullCompute.h"
#include "TR_NodeTree.h"

#include <math.h>

// #define DISABLE_DICHOTOMIC_SEARCH

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_PivotDataView::Impl
#endif
class TV_CatmullCurveView::Impl
{
  MEM_OVERRIDE

  friend class TV_CatmullCurveView;

public:

  Impl() :
    _nodeTree(0),
    _catmullData(0)
  {
  }

  ~Impl()
  {
  }

private:

  const TR_NodeTree                  *_nodeTree;
  const TR_CatmullCurveDataObject    *_catmullData;

  //  Often reused during calculation.
  const TR_CatmullPointDataObject    *_firstPoint;
  const TR_CatmullPointDataObject    *_lastPoint;

  TV_FloatDataViewPtr_t               _veloDataView;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_PivotDataView
#endif

TV_CatmullCurveView::TV_CatmullCurveView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef, const TV_FloatDataViewPtr_t &veloDataView ) :
  TV_Pos3dDataView()
{
  _i = new Impl;
  _i->_nodeTree = nodeTree;
  _i->_veloDataView = veloDataView;

  _i->_catmullData = _i->_nodeTree->dataObject<TR_CatmullCurveDataObject>( dataRef );
  if ( _i->_catmullData )
  {
    unsigned nPoints = _i->_catmullData->_nPoints;
    _i->_firstPoint = pointAt(0);
    _i->_lastPoint = pointAt(nPoints-1);
  }
}

TV_CatmullCurveView::~TV_CatmullCurveView()
{
  delete _i;
}

bool TV_CatmullCurveView::getValue( float frame, float &x, float &y, float &z ) const
{
  unsigned nPoints = this->nPoints();
  if ( nPoints == 0 )
    return false;

  if ( !_i->_firstPoint || !_i->_lastPoint )
    return false;

  if ( frame < _i->_firstPoint->_frame )
  {
    x = _i->_firstPoint->_x * _i->_catmullData->_scaleX;
    y = _i->_firstPoint->_y * _i->_catmullData->_scaleY;
    z = _i->_firstPoint->_z * _i->_catmullData->_scaleZ;
    return true;
  }

  if ( frame > _i->_lastPoint->_frame )
  {
    x = _i->_lastPoint->_x * _i->_catmullData->_scaleX;
    y = _i->_lastPoint->_y * _i->_catmullData->_scaleY;
    z = _i->_lastPoint->_z * _i->_catmullData->_scaleZ;
    return true;
  }

  float length = _i->_lastPoint->_distance;
  if ( length == 0.0f )
  {
    x = _i->_lastPoint->_x * _i->_catmullData->_scaleX;
    y = _i->_lastPoint->_y * _i->_catmullData->_scaleY;
    z = _i->_lastPoint->_z * _i->_catmullData->_scaleZ;
  }

  float f = 0.0f;
  if ( _i->_veloDataView.get() )
  {
    _i->_veloDataView->getValue( frame, f );
  }
  else
  {
    f = (_i->_lastPoint->_frame > 1.0f) ? ((frame-1.0f) / (_i->_lastPoint->_frame-1.0f)) : 0.0f;
  }

  bool ret = getValueU( f * length, x, y, z );

  //  Debugging
#if 0
  for ( int idx = 0 ; idx < nPoints ; ++idx )
  {
    const TR_CatmullPointDataObject *curPoint = pointAt(idx);
    if (curPoint->_frame == frame)
    {
      fprintf( stderr, "%p frame: %f velocity: %f position: %f %f %f length: %f\n", this, frame, f, x, y, z, length );
    }
  }
#endif

  x *= _i->_catmullData->_scaleX;
  y *= _i->_catmullData->_scaleY;
  z *= _i->_catmullData->_scaleZ;

  return ret;
}

void TV_CatmullCurveView::setVelocity( const TV_FloatDataViewPtr_t &veloDataView )
{
  _i->_veloDataView = veloDataView;
}

unsigned TV_CatmullCurveView::nPoints() const
{
  if ( _i->_catmullData != 0 )
  {
    return _i->_catmullData->_nPoints;
  }

  return 0u;
}

bool TV_CatmullCurveView::startIndex( float distance, unsigned &idx0, const TR_CatmullPointDataObject *&pt0 ) const
{
  unsigned nPoints = this->nPoints();
#ifdef DISABLE_DICHOTOMIC_SEARCH
  unsigned nRecur = 0u;
#else
  unsigned nRecur = nPoints / 30;
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
    middleIdx = (endIdx+idx0) >> 1;

    pt0 = pointAt(middleIdx);
    if (pt0 == 0)
      return false;

    if (pt0->_distance < distance)
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

bool TV_CatmullCurveView::getValueU( float distance, float &x, float &y, float &z ) const
{
  unsigned nPoints = this->nPoints();
  if (nPoints > 0)
  {
    const TR_CatmullPointDataObject *catmullPoint1;
    unsigned idx0;
    if (startIndex( distance, idx0, catmullPoint1 ))
    {
      for ( unsigned idx=idx0+1 ; idx<nPoints ; ++idx )
      {
        const TR_CatmullPointDataObject *catmullPoint2 = pointAt(idx);
        if (catmullPoint2)
        {
          if ( distance < catmullPoint2->_distance)
          {
            TR_CatmullCompute::Discreet point;
            TR_CatmullCompute::getDiscreetValue( catmullPoint1, catmullPoint2, distance, &point);

            x = (float)point._x;
            y = (float)point._y;
            z = (float)point._z;

            return true;
          }

          catmullPoint1 = catmullPoint2;
        }
      }

      x = catmullPoint1->_x;
      y = catmullPoint1->_y;
      z = catmullPoint1->_z;

      return true;
    }
  }

  return false;
}

const TR_CatmullPointDataObject *TV_CatmullCurveView::pointAt(unsigned idx) const
{
  if ( _i->_catmullData == 0 )
    return 0;

  TR_Types::DataOffset_t offset = (TR_Types::DataOffset_t)
    ( TR_DataObject::g_dataObjectSizeTable[TR_Types::eCatmullCurveData] +
      (TR_DataObject::g_dataObjectSizeTable[TR_Types::eCatmullPointData] * idx) );
  TR_Types::DataRef_t dataRef = _i->_nodeTree->dataRef(_i->_catmullData, offset);

  return _i->_nodeTree->dataObject<TR_CatmullPointDataObject>(dataRef);
}
