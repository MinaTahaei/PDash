#include "TV_PivotDataView.h"
#include "TR_DataObject.h"
#include "TR_NodeTree.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_PivotDataView::Impl
#endif
class TV_PivotDataView::Impl
{
  MEM_OVERRIDE

  friend class TV_PivotDataView;

public:

  Impl() :
    _nodeTree(0),
    _pivotData(0),
    _firstPoint(0),
    _lastPoint(0)
  {
  }

  ~Impl()
  {
  }

private:

  const TR_NodeTree                  *_nodeTree;

  const TR_AnimatedPivotDataObject   *_pivotData;

  //  Often reused during calculation.
  const TR_PivotPointDataObject      *_firstPoint;
  const TR_PivotPointDataObject      *_lastPoint;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_PivotDataView
#endif

TV_PivotDataView::TV_PivotDataView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef ) :
  TV_Pos3dDataView()
{
  _i = new Impl;
  _i->_nodeTree = nodeTree;

  _i->_pivotData = _i->_nodeTree->dataObject<TR_AnimatedPivotDataObject>( dataRef );
  if ( _i->_pivotData )
  {
    unsigned nPoints = _i->_pivotData->_nPoints;
    _i->_firstPoint = pointAt(0);
    _i->_lastPoint = pointAt(nPoints-1);
  }
}

TV_PivotDataView::~TV_PivotDataView()
{
  delete _i;
}

bool TV_PivotDataView::getValue( float frame, float &x, float &y, float &z ) const
{
  unsigned nPoints = this->nPoints();
  if ( nPoints == 0 )
    return false;

  if ( !_i->_firstPoint || !_i->_lastPoint )
    return false;

  if ( frame < _i->_firstPoint->_frame )
  {
    x = _i->_firstPoint->_x;
    y = _i->_firstPoint->_y;
    z = _i->_firstPoint->_z;
    return true;
  }

  if ( frame > _i->_lastPoint->_frame )
  {
    x = _i->_lastPoint->_x;
    y = _i->_lastPoint->_y;
    z = _i->_lastPoint->_z;
    return true;
  }

  const TR_PivotPointDataObject *prevPoint = _i->_firstPoint;
  for ( unsigned idx = 1 ; idx < nPoints ; ++idx )
  {
    const TR_PivotPointDataObject *curPoint = pointAt(idx);
    if ( curPoint && (frame < curPoint->_frame) )
      break;

    prevPoint = curPoint;
  }

  x = prevPoint->_x;
  y = prevPoint->_y;
  z = prevPoint->_z;
  return true;
}

unsigned TV_PivotDataView::nPoints() const
{
  if ( _i->_pivotData != 0 )
  {
    return _i->_pivotData->_nPoints;
  }

  return 0u;
}

const TR_PivotPointDataObject *TV_PivotDataView::pointAt(int idx) const
{
  if ( _i->_pivotData == 0 )
    return 0;

  TR_Types::DataOffset_t offset = (TR_Types::DataOffset_t)
    ( TR_DataObject::g_dataObjectSizeTable[TR_Types::eAnimatedPivotData] +
      (TR_DataObject::g_dataObjectSizeTable[TR_Types::ePivotPointData] * idx) );
  TR_Types::DataRef_t dataRef = _i->_nodeTree->dataRef(_i->_pivotData, offset);

  return _i->_nodeTree->dataObject<TR_PivotPointDataObject>(dataRef);
}

