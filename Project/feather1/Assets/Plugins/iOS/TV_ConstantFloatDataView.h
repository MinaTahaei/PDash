#ifndef _TV_CONSTANT_FLOAT_DATA_VIEW_H_
#define _TV_CONSTANT_FLOAT_DATA_VIEW_H_

#include "TV_FloatDataView.h"
#include "TR_Types.h"

#include "MEM_Override.h"

class TR_NodeTree;

/*!
 *  @class TV_ConstantFloatDataView
 *  constant float data view
 */
class TV_ConstantFloatDataView : public TV_FloatDataView
{
  MEM_OVERRIDE

public:

  TV_ConstantFloatDataView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef );
  virtual ~TV_ConstantFloatDataView();

  //! Retrieve value at specified frame.
  virtual bool getValue( float frame, float &value ) const;

private:

  class Impl;
  Impl *_i;
};

#endif /* _TV_CONSTANT_FLOAT_DATA_VIEW_H_ */
