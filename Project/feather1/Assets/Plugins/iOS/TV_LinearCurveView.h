#ifndef _TV_LINEAR_CURVE_VIEW_H_
#define _TV_LINEAR_CURVE_VIEW_H_

#include "TV_FloatDataView.h"
#include "TR_Types.h"

#include "MEM_Override.h"

class TR_NodeTree;
class TR_LinearPointDataObject;

/*!
 *  @class TV_LinearCurveView
 */
class TV_LinearCurveView : public TV_FloatDataView
{
  MEM_OVERRIDE

public:

  TV_LinearCurveView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t dataRef );
  ~TV_LinearCurveView();

  //! Retrieve value at specified frame.
  virtual bool getValue( float frame, float &value ) const;

  //! Retrieve number of key frames in current curve.
  unsigned nPoints() const;

protected:

  const TR_LinearPointDataObject *pointAt( unsigned idx ) const;

  bool startIndex( float frame, unsigned &idx0, const TR_LinearPointDataObject *&pt0 ) const;

  bool findSegment( float frame, const TR_LinearPointDataObject *&leftPoint, const TR_LinearPointDataObject *&rightPoint ) const;

  float linearInterpol(const float &u, const float &a, const float &b) const;


private:

  //  not implemented
  TV_LinearCurveView( const TV_LinearCurveView & );
  TV_LinearCurveView &operator= ( const TV_LinearCurveView & );

private:

  class Impl;
  Impl *_i;
};

#endif /* _TV_LINEAR_CURVE_VIEW_H_ */
