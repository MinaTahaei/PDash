/*
 * Copyright ToonBoom Animation Inc. 
 * This file is part of the Toonboom Animation SDK
 */


#ifndef MT_VECTOR3D_H
#define MT_VECTOR3D_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <math.h>

#include "MT_TypeDefinitions.h"

#include "MEM_Override.h"

namespace Math
{
  class Point3d;

  class Vector3d
  {
    MEM_OVERRIDE

  public:
	  //! Constructor for a zero vector (0,0,0).    
	  Vector3d();

    //! Constructor for a vector (x,y,z).    
	  Vector3d(const double x, const double y, const double z); 

    //! Construct a vector from a point.    
    explicit Vector3d(const Point3d& p);

    //! Copy constructor.    
    Vector3d(const Vector3d& v);

    //! Construct a vector v from two points. Same as: v = b - a
    Vector3d(const Point3d& a, const Point3d& b);


    //! Assigns v to this vector.    
    Vector3d& operator=(const Vector3d& v); 

    //! Returns true if v is equal to this vector, false otherwise 
    bool operator==(const Vector3d& v) const;  

    //! Returns true if v is not equal to this vector, false otherwise 
    bool operator!=(const Vector3d& v) const;  

    //! The x coordinate of this vector.    
    double x() const;
    
    //! The y coordinate of this vector.    
    double y() const;
    
    //! The z coordinate of this vector.    
    double z() const; 

    //! Sets the x coordinate for this vector.    
    void setX(const double x);
    
    //! Sets the y coordinate for this vector.    
    void setY(const double y);
    
    //! Sets the x coordinate for this vector.    
    void setZ(const double z);

    //! Sets the all three coordinates for this vector.    
	  void setXYZ(const double x, const double y, const double z);

    //! Returns the coordinate along the given axis for this vector.
    double operator()(Axis3d axis) const;

    //! Returns the coordinate along the given axis for this vector.
    double& operator()(Axis3d axis);

    //! Returns the coordinates as a const array of double.
    const double *asArray() const;

    //! Returns true if this vector has zero length, false otherwise. 
    bool isZero() const;

    //! The length of this vector.    
	  double length() const; 

    //! The squared length of this vector. Useful for comparing the length between vectors without computing any square root.    
    double lengthSq() const; 

    //! Returns the cosinus of the angle formed between this vector and v (range: [-1, 1]). 
	  double getCos(const Vector3d &v) const;

	  //! Returns the sinus of the angle formed between this vector and v (range: [-1, 1]). 
	  double getSin(const Vector3d &v) const;

    //! Returns the dot product between this vector and v, leaving this vector unmodified (same as * operator).    
    double getDotProduct(const Vector3d& v) const;	

    //! Returns the cross product between this vector and v, leaving this vector unmodified (same as ^ operator).    
    Vector3d getCrossProduct(const Vector3d& v) const;		

    //! Returns the ratio of the length of the projection of this vector onto v over the length of v, leaving this vector unmodified.    
    double getProjectionRatio(const Vector3d &v) const;

    //! Returns the result of projecting this vector onto v, leaving this vector unmodified.    
    Vector3d getProjection(const Vector3d &v) const;

    //! Returns the result of normalizing this vector to unit length, leaving this vector unmodified.    
    Vector3d getNormalized() const;



    //! Rounds each coordinate of this vector on a boundary defined by quantum.    
    Vector3d& round(const double quantum);

    //! Normalizes this vector to unit length.    
    Vector3d& normalize();		
    
    //! Sets this vector to its opposite.    
    Vector3d& negate();		

    //! Projects this vector onto v.    
    Vector3d& project(const Vector3d& v);



    //! Returns the result of adding v and this vector, leaving this vector unmodified.    
    Vector3d operator+(const Vector3d& v) const;

    //! Returns the result of subtracting v from this vector, leaving this vector unmodified.    
    Vector3d operator-(const Vector3d& v) const;  

    //! Returns the result of multiplying v by a scalar, leaving this vector unmodified.    
    Vector3d operator*(const double k) const;

    //! Returns the result of dividing v by a scalar, leaving this vector unmodified.    
    Vector3d operator/(const double k) const;

    //! Returns the opposite of this vector, leaving this vector unmodified.    
	  Vector3d operator-() const;	

    //! Returns the dot product between this vector and v, leaving this vector unmodified.    
    double operator*(const Vector3d& v) const;	

    //! Returns the cross product between this vector and v, leaving this vector unmodified.    
    Vector3d operator^(const Vector3d& v) const;		



    //! Adds v to this vector.    
	  Vector3d& operator+=(const Vector3d& v);	

    //! Subtracts v from this vector.      
    Vector3d& operator-=(const Vector3d& v);

    //! Multiplies this vector by a scalar.    
    Vector3d& operator*=(const double k);

    //! Divides this vector by a scalar.    
    Vector3d& operator/=(const double k);

	  friend Vector3d operator*(const double val, const Vector3d& v);
    friend class Point3d;

  private:
    double _a[3];
  };

  //! Returns the result of multiplying v by a scalar, leaving this vector unmodified.    
  Vector3d operator*(const double k, const Vector3d& v);


  // ---------------------- Inline functions

  inline Vector3d::Vector3d()					
  {
	  _a[0] = _a[1] = _a[2] = 0;
  }

  inline Vector3d::Vector3d(const double x, const double y, const double z) 
  {
    _a[0] = x; 
    _a[1] = y; 
    _a[2] = z;
  }

  inline Vector3d::Vector3d(const Vector3d& v)	
  {
    _a[0] = v._a[0];
    _a[1] = v._a[1];
    _a[2] = v._a[2];
  }

  inline Vector3d& Vector3d::operator=(const Vector3d& v)
  {
    _a[0] = v._a[0];
    _a[1] = v._a[1];
    _a[2] = v._a[2];
	  return *this ;
  }

  inline bool Vector3d::operator==(const Vector3d& v) const
  {
    return _a[0] == v._a[0] && _a[1] == v._a[1] && _a[2] == v._a[2];
  }

  inline bool Vector3d::operator!=(const Vector3d& v) const
  {
    return !(_a[0] == v._a[0] && _a[1] == v._a[1] && _a[2] == v._a[2]);
  }

  inline double Vector3d::x() const
  {
    return _a[0];
  }

  inline double Vector3d::y() const
  {
    return _a[1];
  }

  inline double Vector3d::z() const
  {
    return _a[2];
  }

  inline void Vector3d::setX(const double x)
  {
    _a[0] = x; 
  }

  inline void Vector3d::setY(const double y)
  {
    _a[1] = y; 
  }

  inline void Vector3d::setZ(const double z)
  {
    _a[2] = z;
  }

  inline void Vector3d::setXYZ(const double x, const double y, const double z)
  {
    _a[0] = x; 
    _a[1] = y; 
    _a[2] = z;
  }

  inline double Vector3d::operator()(Axis3d axis) const
  {    
    return _a[axis];
  }

  inline double& Vector3d::operator()(Axis3d axis) 
  {    
    return _a[axis];
  }

  inline const double *Vector3d::asArray() const
  {
    return _a;
  }

  inline bool Vector3d::isZero() const
  {
    return _a[0] == 0 && _a[1] == 0 && _a[2] == 0;
  }

  inline double Vector3d::length() const
  {
    return sqrt(_a[0]*_a[0] + _a[1]*_a[1] + _a[2]*_a[2]);
  }

  inline double Vector3d::lengthSq() const
  {
    return _a[0]*_a[0] + _a[1]*_a[1] + _a[2]*_a[2];
  }

  inline double Vector3d::getDotProduct(const Vector3d& v) const
  {
    return _a[0]*v._a[0] + _a[1]*v._a[1] + _a[2]*v._a[2];
  }

  inline Vector3d Vector3d::getCrossProduct(const Vector3d& v) const
  {
	  return Vector3d(
	    _a[1] * v._a[2] - _a[2] * v._a[1], 
	    _a[2] * v._a[0] - _a[0] * v._a[2],
	    _a[0] * v._a[1] - _a[1] * v._a[0]
      );	
  }

  inline Vector3d Vector3d::getNormalized() const
  {
    double f = 1 / sqrt(_a[0]*_a[0] + _a[1]*_a[1] + _a[2]*_a[2]);
    return Vector3d(_a[0] * f, _a[1] * f, _a[2] * f);
  }

  inline Vector3d& Vector3d::round(const double quantum)
  {
    _a[0] = ( floor(  (_a[0] / quantum ) + 0.5 ) * quantum );
    _a[1] = ( floor(  (_a[1] / quantum ) + 0.5 ) * quantum );
    _a[2] = ( floor(  (_a[2] / quantum ) + 0.5 ) * quantum );
    return *this;
  }

  inline Vector3d& Vector3d::normalize()
  {
    double f = 1 / sqrt(_a[0]*_a[0] + _a[1]*_a[1] + _a[2]*_a[2]);
    _a[0] *= f;
    _a[1] *= f;
    _a[2] *= f;
    return *this;
  }

  inline Vector3d& Vector3d::negate()
  {
    _a[0] -= _a[0];
	  _a[1] -= _a[1];
	  _a[2] -= _a[2];
	  return *this;
  }

  inline Vector3d Vector3d::operator+(const Vector3d& v) const
  {
    return Vector3d(_a[0] + v._a[0], _a[1] + v._a[1], _a[2] + v._a[2]);
  }

  inline Vector3d Vector3d::operator-(const Vector3d& v) const
  {
    return Vector3d(_a[0] - v._a[0], _a[1] - v._a[1], _a[2] - v._a[2]);
  }

  inline Vector3d Vector3d::operator*(const double k) const
  {
    return Vector3d(_a[0] * k, _a[1] * k, _a[2] * k);
  }

  inline Vector3d Vector3d::operator/(const double k) const
  {
    double kInv = 1/k;  
    return Vector3d(_a[0] * kInv, _a[1] * kInv, _a[2] * kInv);
  }

  inline Vector3d Vector3d::operator-() const
  {
    return Vector3d(-_a[0], -_a[1], -_a[2]);
  }

  inline double Vector3d::operator*(const Vector3d& v) const
  {
    return _a[0]*v._a[0] + _a[1]*v._a[1] + _a[2]*v._a[2];
  }

  inline Vector3d Vector3d::operator^(const Vector3d& v) const
  {
	  return Vector3d(
	    _a[1] * v._a[2] - _a[2] * v._a[1], 
	    _a[2] * v._a[0] - _a[0] * v._a[2],
	    _a[0] * v._a[1] - _a[1] * v._a[0]
      );	
  }

  inline Vector3d& Vector3d::operator+=(const Vector3d& v)
  {
    _a[0] += v._a[0];
	  _a[1] += v._a[1];
	  _a[2] += v._a[2];
	  return *this;
  }

  inline Vector3d& Vector3d::operator-=(const Vector3d& v)
  {
    _a[0] -= v._a[0];
	  _a[1] -= v._a[1];
	  _a[2] -= v._a[2];
	  return *this;
  }

  inline Vector3d& Vector3d::operator*=(const double k)
  {
    _a[0] *= k;
	  _a[1] *= k;
	  _a[2] *= k;
	  return *this;
  }

  inline Vector3d& Vector3d::operator/=(const double k)
  {
    double kInv = 1 / k;
    _a[0] *= kInv;
	  _a[1] *= kInv;
	  _a[2] *= kInv;
	  return *this;
  }

  inline Vector3d operator*(const double k, const Vector3d& v)
  {
    return Vector3d(v._a[0] * k, v._a[1] * k, v._a[2] * k);
  }
}

#endif

