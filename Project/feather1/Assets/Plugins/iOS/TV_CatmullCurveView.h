#ifndef _TV_CATMULL_CURVE_VIEW_H_
#define _TV_CATMULL_CURVE_VIEW_H_

#include "TV_Pos3dDataView.h"
#include "TV_FloatDataView.h"
#include "TR_Types.h"

#include "MEM_Override.h"

class TR_NodeTree;
class TR_CatmullPointDataObject;

/*!
 *  @class TV_CatmullDataView
 *  Catmull data view
 */
class TV_CatmullCurveView : public TV_Pos3dDataView
{
  MEM_OVERRIDE

public:
  TV_CatmullCurveView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef, const TV_FloatDataViewPtr_t &veloDataView = TV_FloatDataViewPtr_t(0) );
  virtual ~TV_CatmullCurveView();

  //! Retrieve value at specified frame.
  virtual bool getValue( float frame, float &x, float &y, float &z ) const;

  //! Attach a velocity function to current catmull curve.
  void setVelocity( const TV_FloatDataViewPtr_t &veloDataView );

  //! Retrieve number of key frames and control points in current catmull curve.
  unsigned nPoints() const;

private:

  //  not implemented
  TV_CatmullCurveView( const TV_CatmullCurveView & );
  TV_CatmullCurveView &operator= ( const TV_CatmullCurveView & );

protected:

  bool  startIndex( float distance, unsigned &idx0, const TR_CatmullPointDataObject *&pt0 ) const;
  bool  getValueU( float distance, float &x, float &y, float &z ) const;

  const TR_CatmullPointDataObject *pointAt(unsigned idx) const;

private:

  class Impl;
  Impl *_i;

};

#endif /* _TV_CATMULL_CURVE_VIEW_H_ */
