#ifndef _TV_POS_3D_DATA_VIEW_H_
#define _TV_POS_3D_DATA_VIEW_H_

#include "UT_ShareBase.h"
#include "UT_SharedPtr.h"

#include "MEM_Override.h"

class TV_Pos3dDataView;

typedef UT_SharedPtr<TV_Pos3dDataView> TV_Pos3dDataViewPtr_t;

/*!
 *  @class TV_Pos3dDataView
 *  animation 3d position data view
 */
class TV_Pos3dDataView : public UT_ShareBase
{
  MEM_OVERRIDE

public:

  TV_Pos3dDataView();
  virtual ~TV_Pos3dDataView();

  //! Retrieve value at specified frame.
  virtual bool getValue( float frame, float &x, float &y, float &z ) const =0;
};

#endif /* _TV_POS_3D_DATA_VIEW_H_ */
