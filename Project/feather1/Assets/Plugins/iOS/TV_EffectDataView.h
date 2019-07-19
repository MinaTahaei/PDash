
#ifndef _TV_EFFECT_DATA_VIEW_H_
#define _TV_EFFECT_DATA_VIEW_H_

#include "TR_Types.h"

#include "UT_ShareBase.h"
#include "UT_SharedPtr.h"

class TR_NodeTree;
class TV_NodeTreeView;
class TV_EffectDataView;

typedef UT_SharedPtr<TV_EffectDataView> TV_EffectDataViewPtr_t;

/*!
 *  @class TV_EffectDataView
 */
class TV_EffectDataView : public UT_ShareBase
{
public:

  typedef TR_Types::EffectId_t EffectId_t;
  typedef UT_SharedPtr<TV_NodeTreeView> NodeTreeViewPtr_t;

public:

  TV_EffectDataView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef, const NodeTreeViewPtr_t &parentNodeTreeView );
  virtual ~TV_EffectDataView();

  bool       operator==(const TV_EffectDataView &effectDataView) const;
  bool       operator<(const TV_EffectDataView &effectDataView) const;

  //! Retrieve current effect id.
  EffectId_t effectId() const;

  //! Retrieve effect matte node tree.
  NodeTreeViewPtr_t matte() const;

private:

  class Impl;
  Impl *_i;
};

#endif /* _TV_EFFECT_DATA_VIEW_H_ */
