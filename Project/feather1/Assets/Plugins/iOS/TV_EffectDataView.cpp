
#include "TV_EffectDataView.h"
#include "TV_NodeTreeView.h"

#include "TR_NodeTree.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_EffectDataView::Impl
#endif
class TV_EffectDataView::Impl
{
  friend class TV_EffectDataView;

public:

  Impl() :
    _nodeTree(0),
    _effectData(0)
  {
  }

  ~Impl()
  {
  }

private:

  const TR_NodeTree              *_nodeTree;
  const TR_EffectDataObject      *_effectData;

  TV_NodeTreeViewPtr_t            _matteNodeTreeView;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - TV_EffectDataView
#endif

TV_EffectDataView::TV_EffectDataView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef, const TV_NodeTreeViewPtr_t &parentNodeTreeView )
{
  _i = new Impl;
  _i->_nodeTree = nodeTree;

  _i->_effectData = _i->_nodeTree->dataObject<TR_EffectDataObject>(dataRef);
  if ( _i->_effectData )
  {
    if ( _i->_effectData->_matteDataOffset != TR_Types::g_nullOffset )
    {
      TR_Types::DataRef_t matteRef = dataRef + _i->_effectData->_matteDataOffset;
      const TR_DataObject *matteData = _i->_nodeTree->dataObject(matteRef);
      if ( matteData->_id == TR_Types::eNodeData )
      {
        TR_Types::DataRef_t matteRef = nodeTree->dataRef(matteData);
        _i->_matteNodeTreeView = new TV_NodeTreeView(nodeTree, matteRef, parentNodeTreeView, 0, _i->_effectData->_effectId);
      }
    }
  }
}

TV_EffectDataView::~TV_EffectDataView()
{
  delete _i;
}

bool TV_EffectDataView::operator==(const TV_EffectDataView &effectDataView) const
{
  if ( this != &effectDataView )
  {
    return ( _i->_effectData == effectDataView._i->_effectData );
  }

  return true;
}

bool TV_EffectDataView::operator<(const TV_EffectDataView &effectDataView) const
{
  return (_i->_effectData < effectDataView._i->_effectData);
}

TV_EffectDataView::EffectId_t TV_EffectDataView::effectId() const
{
  if ( _i->_effectData != 0 )
  {
    return _i->_effectData->_effectId;
  }

  return TR_Types::eNoop;
}

TV_NodeTreeViewPtr_t TV_EffectDataView::matte() const
{
  return _i->_matteNodeTreeView;
}
