
/*
 * Copyright ToonBoom Animation Inc. 
 * This file is part of the Toonboom Animation SDK
 */

#ifndef MT_POINT3D_H
#define MT_POINT3D_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <math.h>
#include "MT_Vector3d.h"

#include "MEM_Override.h"

namespace Math
{
  class Point3d
  {
    MEM_OVERRIDE

  public:
  	
	  //! Constructor for a point at the origin (0,0,0).    
	  Point3d();

    //! Constructor for a point (x,y,z).    
	  Point3d(const double x, const double y, const double z); 

    //! Constructs a point from a vector.    
    explicit Point3d(const Vector3d& v);

    //! Copy constructor.    
    Point3d(const Point3d& p);



    //! Assigns p to this point.    
    Point3d& operator=(const Point3d& p); 

    //! Returns true if p is equal to this point, false otherwise 
    bool operator==(const Point3d& p) const;  

    //! Returns true if p is not equal to this point, false otherwise 
    bool operator!=(const Point3d& p) const;  



    //! The x coordinate of this point.    
    double x() const;
    
    //! The y coordinate of this point.    
    double y() const;
    
    //! The z coordinate of this point.    
    double z() const;
    
    //! Sets the x coordinate for this point.    
    void setX(const double x);
    
    //! Sets the y coordinate for this point.    
    void setY(const double y);
    
    //! Sets the z coordinate for this point.    
    void setZ(const double z);

    //! Sets the all three coordinates for this point.    
	  void setXYZ(const double x, const double y, const double z);

    //! Returns the coordinate along the given axis for this point.
    double operator()(Axis3d axis) const;

    //! Returns the coordinate along the given axis for this point.
    double& operator()(Axis3d axis);

    //! Returns the coordinates as a const array of double.
    const double *asArray() const;

    //! Returns true if this point is at the origin, false otherwise. 
    bool isOrigin() const;

    //! The distance between this point and p.    
	  double distance(const Point3d &p) const; 

    //! The distance squared between this point and p. Useful for comparing distances without computing any square root.    
    double distanceSq(const Point3d &p) const; 



    //! Rounds each coordinate of this point on a boundary defined by quantum.    
    Point3d& round(const double quantum);



    //! Returns the result of adding v and this point, leaving this point unmodified.    
    Point3d operator+(const Vector3d& v) const;

    //! Returns the result of subtracting v from this point, leaving this point unmodified.    
    Point3d operator-(const Vector3d& v) const;  

    //! Returns the vector resulting from subtracting p from this point, leaving this point unmodified.    
    Vector3d operator-(const Point3d& p) const;  


    //! Adds v to this point.    
	  Point3d& operator+=(const Vector3d& v);	

    //! Subtracts v from this point.      
    Point3d& operator-=(const Vector3d& v);

    //! Returns the linear interpolation between two points, i.e. (1-t)*p1 + t*p2.      
    static Point3d interpolate(double t, const Point3d &p1, const Point3d &p2);	
  	
    friend class Vector3d;

  private:
    double _a[3];
  };


  // ---------------------- Inline functions


  inline Point3d::Point3d()					
  {
	  _a[0] = _a[1] = _a[2] = 0;
  }

  inline Point3d::Point3d(const double x, const double y, const double z) 
  {
    _a[0] = x; 
    _a[1] = y; 
    _a[2] = z;
  }

  inline Point3d::Point3d(const Vector3d& v)	
  {
    _a[0] = v._a[0];
    _a[1] = v._a[1];
    _a[2] = v._a[2];
  }

  inline Point3d::Point3d(const Point3d& p)	
  {
    _a[0] = p._a[0];
    _a[1] = p._a[1];
    _a[2] = p._a[2];
  }

  inline Point3d& Point3d::operator=(const Point3d& p)
  {
    _a[0] = p._a[0];
    _a[1] = p._a[1];
    _a[2] = p._a[2];
	  return *this ;
  }

  inline bool Point3d::operator==(const Point3d& p) const
  {
    return _a[0] == p._a[0] && _a[1] == p._a[1] && _a[2] == p._a[2];
  }

  inline bool Point3d::operator!=(const Point3d& p) const
  {
    return !(_a[0] == p._a[0] && _a[1] == p._a[1] && _a[2] == p._a[2]);
  }

  inline double Point3d::x() const
  {
    return _a[0];
  }

  inline double Point3d::y() const
  {
    return _a[1];
  }

  inline double Point3d::z() const
  {
    return _a[2];
  }

  inline void Point3d::setX(const double x)
  {
    _a[0] = x; 
  }

  inline void Point3d::setY(const double y)
  {
    _a[1] = y; 
  }

  inline void Point3d::setZ(const double z)
  {
    _a[2] = z;
  }

  inline void Point3d::setXYZ(const double x, const double y, const double z)
  {
    _a[0] = x; 
    _a[1] = y; 
    _a[2] = z;
  }

  inline double Point3d::operator()(Axis3d axis) const
  {
    return _a[axis];
  }

  inline double& Point3d::operator()(Axis3d axis)
  {
    return _a[axis];
  }

  inline const double *Point3d::asArray() const
  {
    return _a;
  }

  inline bool Point3d::isOrigin() const
  {
    return _a[0] == 0 && _a[1] == 0 && _a[2] == 0;
  }

  inline double Point3d::distance(const Point3d &p) const 
  {
    return sqrt( (_a[0] - p._a[0])*(_a[0] - p._a[0]) + (_a[1] - p._a[1])*(_a[1] - p._a[1]) + (_a[2] - p._a[2])*(_a[2] - p._a[2]) );
  }

  inline double Point3d::distanceSq(const Point3d &p) const
  {
    return (_a[0] - p._a[0])*(_a[0] - p._a[0]) + (_a[1] - p._a[1])*(_a[1] - p._a[1]) + (_a[2] - p._a[2])*(_a[2] - p._a[2]);
  }

  inline Point3d& Point3d::round(const double quantum)
  {
    _a[0] = ( floor(  (_a[0] / quantum ) + 0.5 ) * quantum );
    _a[1] = ( floor(  (_a[1] / quantum ) + 0.5 ) * quantum );
    _a[2] = ( floor(  (_a[2] / quantum ) + 0.5 ) * quantum );
    return *this;
  }

  inline Point3d Point3d::operator+(const Vector3d& v) const
  {
    return Point3d(_a[0] + v._a[0], _a[1] + v._a[1], _a[2] + v._a[2]);
  }

  inline Point3d Point3d::operator-(const Vector3d& v) const
  {
    return Point3d(_a[0] - v._a[0], _a[1] - v._a[1], _a[2] - v._a[2]);
  }

  inline Vector3d Point3d::operator-(const Point3d& p) const
  {
    return Vector3d(_a[0] - p._a[0], _a[1] - p._a[1], _a[2] - p._a[2]);
  }

  inline Point3d& Point3d::operator+=(const Vector3d& v)
  {
    _a[0] += v._a[0];
	  _a[1] += v._a[1];
	  _a[2] += v._a[2];
	  return *this;
  }

  inline Point3d& Point3d::operator-=(const Vector3d& v)
  {
    _a[0] -= v._a[0];
	  _a[1] -= v._a[1];
	  _a[2] -= v._a[2];
	  return *this;
  }

  inline Point3d Point3d::interpolate(double t, const Point3d &p1, const Point3d &p2)
  {
	  return Point3d(p1._a[0] + t*(p2._a[0]-p1._a[0]), p1._a[1] + t*(p2._a[1]-p1._a[1]), p1._a[2] + t*(p2._a[2]-p1._a[2]));
  }


}

#endif

