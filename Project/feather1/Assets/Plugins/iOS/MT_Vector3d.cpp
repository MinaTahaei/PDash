/*******************************************************************************
  Copyright 2003-2006 ToonBoom Technologies Inc.

  The source code contained in this file is intended for INTERNAL USE ONLY.
  The content is strictly confidential and should not be distributed without
  prior explicit authorization.
*******************************************************************************/


#include "MT_Vector3d.h"
#include "MT_Point3d.h"

namespace Math
{
  Vector3d::Vector3d(const Point3d& p)	
  {
    _a[0] = p._a[0];
    _a[1] = p._a[1];
    _a[2] = p._a[2];
  }

  Vector3d::Vector3d(const Point3d& a, const Point3d& b)	
  {
    _a[0] = b._a[0] - a._a[0];
    _a[1] = b._a[1] - a._a[1];
    _a[2] = b._a[2] - a._a[2];
  }

  double Vector3d::getCos(const Vector3d &v) const
  {
    double denominatorSquared = this->lengthSq() * v.lengthSq();
	  if (denominatorSquared == 0)
		  return 0;

    double cos = this->getDotProduct(v) / sqrt(denominatorSquared);
    return cos < -1.0 ? -1.0 : cos > 1.0 ? 1.0 : cos;
  }

  double Vector3d::getSin(const Vector3d &v) const
  {
    double denominatorSquared = this->lengthSq() * v.lengthSq();
    if (denominatorSquared == 0)
      return 0;

    Vector3d cross = this->getCrossProduct(v);
    double sin = sqrt(cross.lengthSq() / denominatorSquared);
    return sin < -1.0 ? -1.0 : sin > 1.0 ? 1.0 : sin;
  }

  double Vector3d::getProjectionRatio(const Vector3d &v) const
  {
	  return this->getDotProduct(v) / v.lengthSq();	
  }

  Vector3d Vector3d::getProjection(const Vector3d &v) const
  { 	  
	  double ratio = getProjectionRatio(v);	
	  return Vector3d(
      v._a[0] * ratio,
	    v._a[1] * ratio,
      v._a[2] * ratio);	
  }

  Vector3d& Vector3d::project(const Vector3d& v)
  {
	  double ratio = getProjectionRatio(v);	
	  _a[0] = v._a[0] * ratio;
	  _a[1] = v._a[1] * ratio;
    _a[2] = v._a[2] * ratio;
	  return *this;
  }


}
