#ifndef _TV_FLOAT_DATA_VIEW_H_
#define _TV_FLOAT_DATA_VIEW_H_

#include "UT_ShareBase.h"
#include "UT_SharedPtr.h"
#include "MEM_Override.h"

class TV_FloatDataView;

typedef UT_SharedPtr<TV_FloatDataView> TV_FloatDataViewPtr_t;

/*!
 *  @class TV_FloatDataView
 *  animated float data view
 */
class TV_FloatDataView : public UT_ShareBase
{
  MEM_OVERRIDE

public:

  TV_FloatDataView();
  virtual ~TV_FloatDataView();

  //! Retrieve value at specified frame.
  virtual bool getValue( float frame, float &value ) const =0;
};

#endif /* _TV_CURVE_VIEW_H_ */
