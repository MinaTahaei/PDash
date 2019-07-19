#include "TV_ConstantFloatDataView.h"
#include "TR_DataObject.h"
#include "TR_NodeTree.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_BezierCurveView::Impl
#endif
class TV_ConstantFloatDataView::Impl
{
  MEM_OVERRIDE

  friend class TV_ConstantFloatDataView;

public:

  Impl() :
    _nodeTree(0),
    _floatData(0)
  {
  }

  ~Impl()
  {
  }

private:

  const TR_NodeTree              *_nodeTree;
  const TR_FloatDataObject       *_floatData;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_BezierCurveView
#endif

TV_ConstantFloatDataView::TV_ConstantFloatDataView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef ) :
  TV_FloatDataView()
{
  _i = new Impl;
  _i->_nodeTree = nodeTree;

  _i->_floatData = _i->_nodeTree->dataObject<TR_FloatDataObject>( dataRef );
}

TV_ConstantFloatDataView::~TV_ConstantFloatDataView()
{
  delete _i;
}

bool TV_ConstantFloatDataView::getValue( float /*frame*/, float &value ) const
{
  if ( _i->_floatData != 0 )
  {
    value = _i->_floatData->_value;
    return true;
  }

  return false;
}

