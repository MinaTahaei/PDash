#ifndef _TR_CATMULL_COMPUTE_H_
#define _TR_CATMULL_COMPUTE_H_

#include "MEM_Override.h"

class TR_NodeTree;
class TR_CatmullCurveDataObject;
class TR_CatmullPointDataObject;

/*!
 *  @namespace TR_CatmullCompute
 *  Catmull data computation namespace
 */
namespace TR_CatmullCompute
{
  struct Discreet
  {
    MEM_OVERRIDE

    Discreet() : _x(0.0f), _y(0.0f), _z(0.0f), _distance(0.0f)
    {
    }

    double _x, _y, _z;
    double _distance;
  };

  void                             getDiscreetValue( const TR_CatmullPointDataObject *p1,
                                                     const TR_CatmullPointDataObject *p2,
                                                     float wanted_distance,
                                                     Discreet *point );

  double                           getDistance( const TR_CatmullPointDataObject *p1,
                                                const TR_CatmullPointDataObject *p2 );

  void                             getValueU( double u,
                                              const TR_CatmullPointDataObject *p1,
                                              const TR_CatmullPointDataObject *p2,
                                              double &x,
                                              double &y,
                                              double &z );

}

#endif /* _TR_CATMULL_COMPUTE_H_ */
