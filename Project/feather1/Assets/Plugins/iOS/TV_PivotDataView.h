#ifndef _TV_PIVOT_DATA_VIEW_H_
#define _TV_PIVOT_DATA_VIEW_H_

#include "TV_Pos3dDataView.h"
#include "TR_Types.h"

#include "MEM_Override.h"

class TR_NodeTree;
class TR_PivotPointDataObject;

/*!
 *  @class TV_PivotDataView
 *  Animated pivot data view
 */
class TV_PivotDataView : public TV_Pos3dDataView
{
  MEM_OVERRIDE

public:
  TV_PivotDataView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef );
  virtual ~TV_PivotDataView();

  //! Retrieve pivot at specified frame.
  virtual bool getValue( float frame, float &x, float &y, float &z ) const;

  //! Retrieve number of key frames in current curve.
  unsigned nPoints() const;

private:

  //  not implemented
  TV_PivotDataView( const TV_PivotDataView & );
  TV_PivotDataView &operator= ( const TV_PivotDataView & );

protected:

  const TR_PivotPointDataObject *pointAt(int idx) const;

private:

  class Impl;
  Impl *_i;
};

#endif /* _TV_PIVOT_DATA_VIEW_H_ */
