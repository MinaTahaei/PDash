#ifndef _TV_BEZIER_CURVE_VIEW_H_
#define _TV_BEZIER_CURVE_VIEW_H_

#include "TV_FloatDataView.h"
#include "TR_Types.h"

#include "MEM_Override.h"

class TR_NodeTree;
class TR_BezierPointDataObject;

/*!
 *  @class TV_BezierCurveView
 */
class TV_BezierCurveView : public TV_FloatDataView
{
  MEM_OVERRIDE

public:

  TV_BezierCurveView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef );
  ~TV_BezierCurveView();

  //! Retrieve value at specified frame.
  virtual bool getValue( float frame, float &value ) const;

  //! Retrieve number of key frames in current bezier curve.
  unsigned nPoints() const;

protected:

  const TR_BezierPointDataObject *pointAt( unsigned idx ) const;

  bool startIndex( float frame, unsigned &idx0, const TR_BezierPointDataObject *&pt0 ) const;

  bool findSegment( float frame, const TR_BezierPointDataObject *&leftPoint, const TR_BezierPointDataObject *&rightPoint ) const;

  float linearInterpol(const float &u, const float &a, const float &b) const;

  float findU( float x, const TR_BezierPointDataObject *leftPoint, const TR_BezierPointDataObject *rightPoint ) const;

  float getValueX( float u, const TR_BezierPointDataObject *leftPoint, const TR_BezierPointDataObject *rightPoint ) const;
  float getValueY( float u, const TR_BezierPointDataObject *leftPoint, const TR_BezierPointDataObject *rightPoint ) const;

private:

  //  not implemented
  TV_BezierCurveView( const TV_BezierCurveView & );
  TV_BezierCurveView &operator= ( const TV_BezierCurveView & );

private:

  class Impl;
  Impl *_i;
};

#endif /* _TV_BEZIER_CURVE_VIEW_H_ */
