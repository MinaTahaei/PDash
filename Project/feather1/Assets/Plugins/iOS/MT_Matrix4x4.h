
/*
 * Copyright ToonBoom Animation Inc. 
 * This file is part of the Toonboom Animation SDK
 */


#ifndef MT_MATRIX4X4_H
#define MT_MATRIX4X4_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MT_TypeDefinitions.h"
#include "MT_Vector3d.h"
#include "MT_Point3d.h"

#include "MEM_Override.h"

#include <string.h>

namespace Math
{
  class Matrix4x4
  {
    MEM_OVERRIDE

  public:
    class Singularity { };

    //! Construct an identity transformation matrix. 
    Matrix4x4();

    //! Construct a matrix from a column major array of 16 terms.
    Matrix4x4(const double matrixTerms[16]);

    //! Construct a matrix from a column major array of 16 terms.
    Matrix4x4(const float matrixTerms[16]);

    //! Construct a matrix from the given three axis and origin.
    Matrix4x4(const Vector3d &xAxis, const Vector3d &yAxis, const Vector3d &zAxis, const Point3d &origin);

    //! Construct a matrix from the given normal axis and origin. 
    Matrix4x4(const Vector3d &normal, const Point3d &origin);

    //! Copy constructor.
    Matrix4x4(const Matrix4x4& m);

    //! Assigns m to this matrix.    
    Matrix4x4& operator= (const Matrix4x4& m);

    //! Returns true if m is equal to this matrix, false otherwise 
    bool operator== (const Matrix4x4& m) const;

    //! Returns true if m is not equal to this matrix, false otherwise 
    bool operator!= (const Matrix4x4& m) const;

    //! Returns the matrix item located at the given row and column indices.
    double operator()(Row row, Column column) const;

    //! Returns the matrix item located at the given row and column indices.
    double& operator()(Row row, Column column);

    //! returns -1,0,1 if less, equal or greater. Used to define a total ordering.
    int compare( const Matrix4x4 &rhs ) const;

    //! Returns the terms of this matrix as a column-major const array of doubles.
    const double *asArray() const { return _array; }

    //! Returns one of the three axis components of this matrix (i.e. the first, second or third column).    
    //! More precicely, the result is the local axis transformed into world coordinates.
    //! Note that this methods ignores any projective component of this matrix.
    Vector3d axis(Axis3d axis) const;

    //! Returns the origin component of this matrix (i.e. the fourth column).
    //! More precicely, the result is the local origin (0, 0, 0) transformed into world coordinates.
    //! Note that this methods ignores any projective component of this matrix.
    Point3d origin() const;

    //! Returns rotation component of this matrix (3x3 block).
    Math::Matrix4x4 rotation() const;

    //! Returns true if this is the identity transform matrix, false otherwise. 
    bool isEmpty() const;

    //! Returns true if this is the identity transform matrix, false otherwise. 
    bool isIdentity() const;

    //! Returns true if the identity flag is set, false otherwise.
    bool isIdentityFlag() const;

    //! Reset this to the identity transform matrix. Returns this matrix.
    Matrix4x4& clear();

    //! Normalizes this matrix by dividing by the M(3,3) term. Returns this matrix.
    Matrix4x4& normalize();

    //! Compounds this matrix with m, same as (*this) = (*this) * m. Returns this matrix.
    Matrix4x4& multiply(const Matrix4x4& m);

    //! Translates the local coordinate system represented by this tranformation matrix by the given vector. Returns this matrix.
    Matrix4x4& translate(const Vector3d& v);

    //! Translates the local coordinate system represented by this tranformation matrix by the given vector. Returns this matrix.
    Matrix4x4& translate(double deltaX, double deltaY, double deltaZ = 0.0);

    //! Scales the local coordinate system represented by this tranformation matrix by the given factors. Returns this matrix.
    Matrix4x4& scale(double scaleX, double scaleY, double scaleZ = 1.0);

    //! Rotates the local coordinate system represented by this tranformation matrix by the given angle (expressed in radian) around the given vector. Returns this matrix.
    Matrix4x4& rotateRadians(double rads, Vector3d axis=Vector3d(0,0,1));

    //! Rotates the local coordinate system represented by this tranformation matrix by the given angle (expressed in degree) around the given vector. Returns this matrix.
    Matrix4x4& rotateDegrees(double degs, Vector3d axis=Vector3d(0,0,1));

    //! Construct a matrix from the given normal axis and origin. Same as similat constructor excepts easier to call from scripts
    void rotatePlane( const Vector3d &normal, const Point3d &origin );

    //! Skews the local coordinate system represented by this tranformation matrix by the given angle (expressed in degree). Returns this matrix.
    Matrix4x4& skew(double skew);

    /*! Applies an orthogonal projection to the local coordinate system represented by this tranformation matrix. Returns this matrix. */
    Matrix4x4& orthogonalProject(double left, double right, double bottom, double top, double zNear, double zFar);

    /*! Applies a perspective projection to the local coordinate system represented by this tranformation matrix. Returns this matrix. */
    Matrix4x4& perspectiveProject(double left, double right, double bottom, double top, double zNear, double zFar);

    /*! Applies an perspective projection to the local coordinate system represented by this tranformation matrix. Returns this matrix. */
    Matrix4x4& perspectiveProject(double verticalFieldOfViewInDegrees, double widthOverHeightAspectRatio, double zNear, double zFar);

    /*! Replaces this by a lookAt matrix */
    Matrix4x4& lookAt(const Point3d &eye, const Point3d &center, const Vector3d &upDirection);

    /*!
     *  Extract 2d matrix parameters using 2d pivot value.
     *  sxPos and syPos are provided to resolve sign ambiguity for scaling.
     */
    bool extractParameters( bool sxPos, bool syPos,
                            double px, double py,
                            double &tx, double &ty, double &tz,
                            double &sx, double &sy, double &angle,
                            double &skew) const;

    /*!
     *  Extract 3d matrix parameters using 3d pivot value.
     */
    bool extractParameters( double px, double py, double pz,
                            double &tx, double &ty, double &tz,
                            double &sx, double &sy, double &sz,
                            double &ax, double &ay, double &az ) const;

    //! Inverses the current matrix.
    bool inverse();

    //! Returns the inverse of this matrix, leaving this matrix unmodified.
    bool getInverse(Math::Matrix4x4 &matrix) const;

    bool isSingular() const;

    //! Transposes the current matrix.
    bool transpose();

    //! Returns the transpose of this matrix, leaving this matrix unmodified.
    bool getTranspose(Math::Matrix4x4 &matrix) const;

    //! Fills the array with a valid OpenGL matrix.
    void getValues(float  m[16]) const;

    //! Fills the array with a valid OpenGL matrix.
    void getValues(double m[16]) const;

    //! Prints this matrix. For debugging only.
    void print(const char* text) const;

    // the truck factor is a compensation factor due to the 
    // zooming of the camera and is inherant the the 
    // transformations of a matrix
    double truckFactor() const;

    //! Returns the result of multiplying this by m, leaving this matrix unmodified.    
    Matrix4x4 operator*(const Matrix4x4& m) const;

    //! Multiplies this matrix with m, i.e. (*this) = (*this) * m. 
    Matrix4x4& operator*=(const Matrix4x4& m);

    /* Returns the point resulting from transforming p with this matrix. 
       The point is first converted to homogeneous space (with an implicit 1 for w).
       The point is transformed by this matrix.
       The result is converted back to 3d space by dividing by w.
       Note that the rotation, scaling, shearing, projection and translation parts of the matrix all affect
       the point transformation.
      */
    Point3d operator*(const Point3d& p) const;

    /* Returns the vector resulting from transforming v with this matrix. 
       Note that the vector is only affected by the rotation, scaling and shearing parts of the matrix.
       The projection and translation components are ignored.
      */
    Vector3d operator*(const Vector3d& p) const;

    /* Returns true if transforming a constant Z plane yields another constant Z plane. 
       In other words, for two points P1 and P2 in a constant local plane (therefore having the same local z and w coordinates), 
       the transformed points are also in the same constant plane (therefore having the same transformed z' and w' coordinates):
         (x1',y1',z',w') = M * (x1, y1, z, w) 
         (x2',y2',z',w') = M * (x2, y2, z, w) 
       For instance, a 2d transform combined with the usual projection matrix will have this constantZ property.
     */
    bool isConstantZ() const;

    /* returns true if this matrix can be converted to a 3x2 matrix representing a 2d transformation, false otherwise. */
    bool isTransform2d() const;

    /* Returns true if any of the matrix's three projection terms are non-zero. */
    bool isPerspectiveProjection() const;

    ///////////////////// DEPRECATED METHODS

  private:
    /*! Deprecated as public. Eventually will be private only.
        If initialize is true, then this constructor has the same behavior as the default constuctor. 
        If initialize is false, then none of the matrix terms are initialized and all of them must be initialized one by one outside of the constructor. 
        Note that in the later case the _isIdentity private field is set to false. */
    Matrix4x4(bool initialize);
  public:

    //! Deprecated.
    Matrix4x4(const double& a, const double& b, const double& e, 
                  const double& c, const double& d, const double& f,
                  const double& i, const double& j, const double& z,
                  const double& g, const double& h, const double& w);

    //! Deprecated.
    double getA() const {return _array[0];}

    //! Deprecated.
    double getB() const {return _array[4];}

    //! Deprecated.
    double getE() const {return _array[12];}

    //! Deprecated.
    double getC() const {return _array[1];}

    //! Deprecated.
    double getD() const {return _array[5];}

    //! Deprecated.
    double getF() const {return _array[13];}

    //! Deprecated.
    double getZ() const {return _array[14];}

    //! Deprecated.
    double getG() const {return _array[3];}

    //! Deprecated.
    double getH() const {return _array[7];}

    //! Deprecated.
    double getW() const {return _array[15];}  


    //! Deprecated.
    void setA(const double &a) {_array[0] = a; _isIdentity = false; }

    //! Deprecated.
    void setB(const double &b) {_array[4] = b; _isIdentity = false; }

    //! Deprecated.
    void setE(const double &e) {_array[12] = e; _isIdentity = false; }

    //! Deprecated.
    void setC(const double &c) {_array[1] = c; _isIdentity = false; }

    //! Deprecated.
    void setD(const double &d) {_array[5] = d; _isIdentity = false; }

    //! Deprecated.
    void setF(const double &f) {_array[13] = f; _isIdentity = false; }  

    //! Deprecated.
    void setZ(const double &z) {_array[14] = z; _isIdentity = false; }

    //! Deprecated.
    void setG(const double &g) {_array[3] = g; _isIdentity = false; }

    //! Deprecated.
    void setH(const double &h) {_array[7] = h; _isIdentity = false; }

    //! Deprecated.
    void setW(const double &w) {_array[15] = w; _isIdentity = false; }
   
    //! Deprecated. 
    void setMatrix(const double& a, const double& b, const double& e,
                  const double& c, const double& d, const double& f,
                  const double& i, const double& j, const double& z,
                  const double& g, const double& h, const double& w);

    //! Deprecated.
    void multiplyPoint(double* x, double* y) const;

    //! Deprecated.
    void multiplyPoint(float* x, float* y) const;

    //! Deprecated. 
    void multiplyPoint(double* x, double* y, double* z) const;

    //! Deprecated. 
    void multiplyPoint(float* x, float* y, float* z) const;

    //! Deprecated. Only used in one place.
    void multiplyVector(double* x, double* y, double* z) const;

    //! Deprecated. Only used in one place.
    void multiplyVector(float* x, float* y, float* z) const;

    /* deprecated */
    bool hasQuadmap() const;
    

  private:
    double _array[16];
    bool _isIdentity; 
    static const double _identityPattern[16];

    double convertDegreesToRadians(const double degs) const;

    /*!
    LU Decomposition for an 4 x 4 matrix.
    'a' will contain the decomposition.
    'even' indicate if they are even or odd number of row permutations.
    'index' contain the order of permuted rows.
    */
    bool luDecompose(double a[4][4], 
                    int index[4], bool* even) const;
    /*!
      Solve a linar system AX = B by backsubstitution of a lu decomposition.
      A must be given as a lu decomposition of an 4 x 4 matrix.
      B will contain X.
    */ 
    void luSolve(const double a[4][4], 
                const int index[4], double b[4]) const;
    /*!
      Inverse of an 4 x 4 matrix.
      mat will contain the lu decomposition.
    */
    bool inverse(double inv[4][4],
                double mat[4][4]) const;
  };



  // -------------- inline functions


  inline Matrix4x4::Matrix4x4()
  {
    memcpy(_array, _identityPattern, sizeof(_array));
    _isIdentity = true;
  }

  inline Matrix4x4::Matrix4x4(const Matrix4x4& m)
  {
    memcpy(_array, m._array, sizeof(_array));
    _isIdentity = m._isIdentity;
  }

  inline Matrix4x4::Matrix4x4(bool initialize)
  {    
    if (initialize)    
      memcpy(_array, _identityPattern, sizeof(_array));
    _isIdentity = initialize;
  }

  inline double Matrix4x4::operator()(Row row, Column column) const
  {
    return _array[column * 4 + row];
  }

  inline double& Matrix4x4::operator()(Row row, Column column)
  {
    _isIdentity = false;
    return _array[column * 4 + row];
  }

  inline Vector3d Matrix4x4::axis(Axis3d axis) const
  {
    int index = axis * 4;
    return Vector3d(_array[index], _array[index + 1], _array[index + 2]);
  }

  inline Point3d Matrix4x4::origin() const
  {
    double w = _array[15];
    if (fabs(w) > 0.0000001)
      return Point3d(_array[12] / w, _array[13] / w, _array[14] / w);
    else
      return Point3d(_array[12], _array[13], _array[14]);
  }

  inline Math::Matrix4x4 Math::Matrix4x4::rotation() const
  {
    return Math::Matrix4x4( axis(Axis3d_X), axis(Axis3d_Y), axis(Axis3d_Z), Math::Point3d() );
  }

  inline bool Matrix4x4::isEmpty() const
  {
    if (_isIdentity)
      return true;
    return !memcmp(_array, _identityPattern, sizeof(_array));
  }

  inline bool Matrix4x4::isIdentity() const
  {
    if (_isIdentity)
      return true;
    return !memcmp(_array, _identityPattern, sizeof(_array));
  }

  inline bool Matrix4x4::isIdentityFlag() const
  {
    return _isIdentity;
  }

  inline Matrix4x4& Matrix4x4::clear()
  {
    if (_isIdentity)
      return *this;

    memcpy(_array, _identityPattern, sizeof(_array));
    _isIdentity = true;

    return *this;
  }

  inline Matrix4x4& Matrix4x4::multiply(const Matrix4x4& m)
  {
    *this *= m;    
    return *this;
  }


}

#endif
