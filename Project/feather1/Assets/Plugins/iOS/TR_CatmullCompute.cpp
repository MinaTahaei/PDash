#include "TR_CatmullCompute.h"
#include "TR_NodeTree.h"
#include "TR_DataObject.h"

#include <math.h>

#define NB_PTS_PER_SEG 50
#define FLOAT_EPSILON 0.0000001

namespace
{
  double linearInterpol(const double &u, const double &a, const double &b)
  {
    return (a + u * (b - a));
  }
}

void TR_CatmullCompute::getDiscreetValue( const TR_CatmullPointDataObject *p1,
                                          const TR_CatmullPointDataObject *p2,
                                          float wanted_distance,
                                          Discreet *point )
{
  int       i,  mmax(NB_PTS_PER_SEG);
  double    u,  du(1.0 / double(mmax)),
            dx, dy, dz;
  Discreet pa, pb; // start and end of each small segment during interpolation

  pa._x = p1->_x;  // prepare for first segment
  pa._y = p1->_y;
  pa._z = p1->_z;
  pa._distance = p1->_distance;

  for(i = 1; i <= mmax; ++i)
  {
    u = i * du;

    getValueU(u, p1, p2, pb._x, pb._y, pb._z);

    dx = pb._x - pa._x;
    dy = pb._y - pa._y;
    dz = pb._z - pa._z;

    dy = dy * 0.75;
    dz = dz * 2.0;

    if(dx == 0 && dy == 0 && dz == 0)
      pb._distance = pa._distance + FLOAT_EPSILON;
    else
      pb._distance = pa._distance + sqrt(dx*dx + dy*dy + dz*dz);

    if (point && (wanted_distance <= pb._distance))
    {
      break;
    }

    pa = pb;
  }

  if (point)
  {
    double percent;
    percent = (wanted_distance - pa._distance) / (pb._distance - pa._distance);
    point->_x = linearInterpol(percent, pa._x, pb._x);
    point->_y = linearInterpol(percent, pa._y, pb._y);
    point->_z = linearInterpol(percent, pa._z, pb._z);
  }
}

double TR_CatmullCompute::getDistance( const TR_CatmullPointDataObject *p1,
                                      const TR_CatmullPointDataObject *p2 )
{
  int       i,  mmax(NB_PTS_PER_SEG);
  double    u,  du(1.0 / double(mmax)),
            dx, dy, dz;
  Discreet pa, pb; // start and end of each small segment during interpolation

  pa._x = p1->_x;  // prepare for first segment
  pa._y = p1->_y;
  pa._z = p1->_z;
  pa._distance = 0.0f;

  for(i = 1; i <= mmax; ++i)
  {
    u    = i * du;

    getValueU(u, p1, p2, pb._x, pb._y, pb._z);

    dx = pb._x - pa._x;
    dy = pb._y - pa._y;
    dz = pb._z - pa._z;

    dy = dy * 0.75;
    dz = dz * 2.0;

    if(dx == 0 && dy == 0 && dz == 0)
      pb._distance = pa._distance + FLOAT_EPSILON;
    else
      pb._distance = pa._distance + sqrt(dx*dx + dy*dy + dz*dz);

    pa = pb;
  }

  return pb._distance;
}

void TR_CatmullCompute::getValueU( double u,
                                   const TR_CatmullPointDataObject *p1,
                                   const TR_CatmullPointDataObject *p2,
                                   double &x,
                                   double &y,
                                   double &z )
{
  x =
    (((p2->_d_in_x +       p1->_d_out_x - 2.0 * p2->_x + 2.0 * p1->_x) * u
      + -p2->_d_in_x - 2.0 * p1->_d_out_x + 3.0 * p2->_x - 3.0 * p1->_x) * u
      +  p1->_d_out_x) * u
      +  p1->_x;

  y =
    (((p2->_d_in_y +       p1->_d_out_y - 2.0 * p2->_y + 2.0 * p1->_y) * u
      + -p2->_d_in_y - 2.0 * p1->_d_out_y + 3.0 * p2->_y - 3.0 * p1->_y) * u
      +  p1->_d_out_y) * u
      +  p1->_y;

  z =
    (((p2->_d_in_z +       p1->_d_out_z - 2.0 * p2->_z + 2.0 * p1->_z) * u
      + -p2->_d_in_z - 2.0 * p1->_d_out_z + 3.0 * p2->_z - 3.0 * p1->_z) * u
      +  p1->_d_out_z) * u
      +  p1->_z;
}


