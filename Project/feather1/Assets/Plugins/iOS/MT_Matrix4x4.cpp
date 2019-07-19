/*******************************************************************************
  Copyright 2003-2006 ToonBoom Technologies Inc.

  The source code contained in this file is intended for INTERNAL USE ONLY.
  The content is strictly confidential and should not be distributed without
  prior explicit authorization.
*******************************************************************************/

#include "MT_Matrix4x4.h"

#include <cmath>
#include <cfloat>
#include <cstdio>

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#endif // WIN32


namespace Math
{
  const double Matrix4x4::_identityPattern[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

  #define	TM_PI_180 0.01745329251994329577
  double Matrix4x4::convertDegreesToRadians(const double degs) const	
  { 
    return degs*TM_PI_180;
  }

  Matrix4x4::Matrix4x4(const double matrixTerms[16])
  {
    if (matrixTerms)
    {
      memcpy(_array, matrixTerms, sizeof(_array));
      _isIdentity = !memcmp(_array, _identityPattern, sizeof(_array));
    }
    else
    {
      memcpy(_array, _identityPattern, sizeof(_array));
      _isIdentity = true;
    }    
  }

  Matrix4x4::Matrix4x4(const float matrixTerms[16])
  {
    if (matrixTerms)
    {
      for (int i = 0; i < 16; i++)
        _array[i] = matrixTerms[i];
      _isIdentity = !memcmp(_array, _identityPattern, sizeof(_array));
    }
    else
    {
      memcpy(_array, _identityPattern, sizeof(_array));
      _isIdentity = true;
    }    
  }

  Matrix4x4::Matrix4x4(const Vector3d &xAxis, const Vector3d &yAxis, const Vector3d &zAxis, const Point3d &origin)
  {
    _array[0] = xAxis(Axis3d_X); _array[4] = yAxis(Axis3d_X); _array[8]  = zAxis(Axis3d_X); _array[12] = origin(Axis3d_X);
    _array[1] = xAxis(Axis3d_Y); _array[5] = yAxis(Axis3d_Y); _array[9]  = zAxis(Axis3d_Y); _array[13] = origin(Axis3d_Y);  
    _array[2] = xAxis(Axis3d_Z); _array[6] = yAxis(Axis3d_Z); _array[10] = zAxis(Axis3d_Z); _array[14] = origin(Axis3d_Z);
    _array[3] = 0;               _array[7] = 0;               _array[11] = 0;               _array[15] = 1;    

    _isIdentity = !memcmp(_array, _identityPattern, sizeof(_array));
  }




  Matrix4x4::Matrix4x4(const Vector3d &normal, const Point3d &origin)
  {
    Vector3d xAxis, yAxis, zAxis;
    
    // We need a vector that is at 90 degree with the _normal.
    // The following guarantees that the dot-product of xAxis and _normal will be 0.0.
    double x = fabs(normal.x());
    double y = fabs(normal.y());
    double z = fabs(normal.z());
        
    if (x > y && x > z)
      xAxis.setXYZ(-normal.y(), normal.x(), 0);
    else if (y > z)
      xAxis.setXYZ(0, -normal.z(), normal.y());
    else
      xAxis.setXYZ(normal.z(), 0, -normal.x());
    
    yAxis = normal.getCrossProduct(xAxis);
    zAxis = normal;

    _array[0] = xAxis(Axis3d_X); _array[4] = yAxis(Axis3d_X); _array[8]  = zAxis(Axis3d_X); _array[12] = origin(Axis3d_X);
    _array[1] = xAxis(Axis3d_Y); _array[5] = yAxis(Axis3d_Y); _array[9]  = zAxis(Axis3d_Y); _array[13] = origin(Axis3d_Y);  
    _array[2] = xAxis(Axis3d_Z); _array[6] = yAxis(Axis3d_Z); _array[10] = zAxis(Axis3d_Z); _array[14] = origin(Axis3d_Z);
    _array[3] = 0;               _array[7] = 0;               _array[11] = 0;               _array[15] = 1;    

    _isIdentity = !memcmp(_array, _identityPattern, sizeof(_array));
  }

  Matrix4x4::Matrix4x4(
    const double& a, const double& b, const double& e, 
    const double& c, const double& d, const double& f,
    const double& i, const double& j, const double& z,
    const double& g, const double& h, const double& w)
  {
    _array[0] = a; _array[4] = b; _array[8]  = 0; _array[12] = e;
    _array[1] = c; _array[5] = d; _array[9]  = 0; _array[13] = f;  
    _array[2] = i; _array[6] = j; _array[10] = 1; _array[14] = z;
    _array[3] = g; _array[7] = h; _array[11] = 0; _array[15] = w;

    /*
      Integration of the z value.
      
      1 0 0 0    a b 0 e    a    b    0 e
      0 1 0 0 *  c d 0 f == c    d    0 f
      0 0 1 z    i j 1 0    i+gz j+hz 1 wz
      0 0 0 1    g h 0 w    g    h    0 w
    */
    _array[2]  += g*z;
    _array[6]  += h*z;
    _array[14] *= w;

    _isIdentity = false;
  }

  void Matrix4x4::setMatrix(
    const double& a, const double& b, const double& e,
    const double& c, const double& d, const double& f,
    const double& i, const double& j, const double& z,
    const double& g, const double& h, const double& w)
  {
    _array[0] = a; _array[4] = b; _array[8]  = 0; _array[12] = e;
    _array[1] = c; _array[5] = d; _array[9]  = 0; _array[13] = f;  
    _array[2] = i; _array[6] = j; _array[10] = 1; _array[14] = z;
    _array[3] = g; _array[7] = h; _array[11] = 0; _array[15] = w;

    /*
      Integration of the z value.
      
      1 0 0 0    a b 0 e    a    b    0 e
      0 1 0 0 *  c d 0 f == c    d    0 f
      0 0 1 z    i j 1 0    i+gz j+hz 1 wz
      0 0 0 1    g h 0 w    g    h    0 w
    */
    _array[2]  += g*z;
    _array[6]  += h*z;
    _array[14] *= w;

    _isIdentity = false;
  }

  int Matrix4x4::compare( const Matrix4x4 &rhs ) const
  {
    for (int i = 0; i < 16; i++)
    {
      if (_array[i] < rhs._array[i])
        return -1;
      if (_array[i] > rhs._array[i])
        return 1;
    }
    return 0;
  }

  void Matrix4x4::multiplyPoint(double* x, double* y) const
  {
    if (x == NULL || y == NULL)
      return;

    if (_isIdentity)
      return;

    // Multiplication.
    double tmpX = _array[0] * *x + _array[4] * *y + _array[12];
    double tmpY = _array[1] * *x + _array[5] * *y + _array[13];
    double tmpW = _array[3] * *x + _array[7] * *y + _array[15];

    // 2d perspective division.
    *x = tmpX / tmpW;
    *y = tmpY / tmpW;
  }

  void Matrix4x4::multiplyPoint(float* x, float* y) const
  {
    if (x == NULL || y == NULL)
      return;

    if (_isIdentity)
      return;

    // Multiplication.
    float tmpX = (float)(_array[0] * *x + _array[4] * *y + _array[12]);
    float tmpY = (float)(_array[1] * *x + _array[5] * *y + _array[13]);
    float tmpW = (float)(_array[3] * *x + _array[7] * *y + _array[15]);

    // 2d perspective division.
    *x = tmpX / tmpW;
    *y = tmpY / tmpW;
  }

  void Matrix4x4::multiplyPoint(double* x, double* y, double* z) const
  {
    if (x == NULL || y == NULL || z == NULL)
      return;

    if (_isIdentity)
      return;

    // Multiplication.
    double tmpX = _array[0] * *x + _array[4] * *y + _array[8]  * *z + _array[12];
    double tmpY = _array[1] * *x + _array[5] * *y + _array[9]  * *z + _array[13];
    double tmpZ = _array[2] * *x + _array[6] * *y + _array[10] * *z + _array[14];
    double tmpW = _array[3] * *x + _array[7] * *y + _array[11] * *z + _array[15];

    // 2d perspective division.
    *x = tmpX / tmpW;
    *y = tmpY / tmpW;
    *z = tmpZ / tmpW;
  }

  void Matrix4x4::multiplyPoint(float* x, float* y, float* z) const
  {
    if (x == NULL || y == NULL || z == NULL)
      return;

    if (_isIdentity)
      return;

    // Multiplication.
    float tmpX = float(_array[0] * *x + _array[4] * *y + _array[8]  * *z + _array[12]);
    float tmpY = float(_array[1] * *x + _array[5] * *y + _array[9]  * *z + _array[13]);
    float tmpZ = float(_array[2] * *x + _array[6] * *y + _array[10] * *z + _array[14]);
    float tmpW = float(_array[3] * *x + _array[7] * *y + _array[11] * *z + _array[15]);

    // 2d perspective division.
    *x = tmpX / tmpW;
    *y = tmpY / tmpW;
    *z = tmpZ / tmpW;
  }

  void Matrix4x4::multiplyVector(double* x, double* y, double* z) const
  {
    if (x == NULL || y == NULL || z == NULL)
      return;

    if (_isIdentity)
      return;

    // Multiplication.
    double tmpX = _array[0] * *x + _array[4] * *y + _array[8]  * *z;
    double tmpY = _array[1] * *x + _array[5] * *y + _array[9]  * *z;

    *z = _array[2] * *x + _array[6] * *y + _array[10] * *z;

    *x = tmpX;
    *y = tmpY;
  }

  void Matrix4x4::multiplyVector(float* x, float* y, float* z) const
  {
    if (x == NULL || y == NULL || z == NULL)
      return;

    if (_isIdentity)
      return;

    // Multiplication.
    float tmpX = float(_array[0] * *x + _array[4] * *y + _array[8]  * *z);
    float tmpY = float(_array[1] * *x + _array[5] * *y + _array[9]  * *z);

    *z = float(_array[2] * *x + _array[6] * *y + _array[10] * *z);

    *x = tmpX;
    *y = tmpY;
  }

  Matrix4x4& Matrix4x4::normalize()
  {
    double w = _array[15];
    if (fabs(w) > 0.0000001)
    {
      for (int i = 0; i < 15; i++)
        _array[i] = _array[i] / w;
      _array[15] = 1.0;
    }        
    return *this;
  }


  Matrix4x4& Matrix4x4::translate(double deltaX, double deltaY, double deltaZ)
  {
    return translate(Vector3d(deltaX, deltaY, deltaZ));    
  }

  Matrix4x4& Matrix4x4::translate(const Vector3d &v)
  {
    if (!_isIdentity)
    {
      if (v.x() != 0.0)
      {      
        _array[12] += _array[0]*v.x();
        _array[13] += _array[1]*v.x();
        _array[14] += _array[2]*v.x();
        _array[15] += _array[3]*v.x();
      }
      if (v.y() != 0.0)
      {
        _array[12] += _array[4]*v.y();
        _array[13] += _array[5]*v.y();
        _array[14] += _array[6]*v.y();
        _array[15] += _array[7]*v.y();
      }
      if(v.z() != 0.0)
      {
        _array[12] += _array[8]*v.z();
        _array[13] += _array[9]*v.z();
        _array[14] += _array[10]*v.z();
        _array[15] += _array[11]*v.z();
      }
    }
    else if (v.x() != 0.0 || v.y() != 0.0 || v.z() != 0.0)
    {      
      _array[12] = v.x();
      _array[13] = v.y();
      _array[14] = v.z();
      _isIdentity = false;
    }
    return *this;
  }

  Matrix4x4& Matrix4x4::scale(double scaleX, double scaleY, double scaleZ)
  {
    if (!_isIdentity)
    {
      if( scaleX != 1.0 )
      {
        _array[0] *= scaleX;
        _array[1] *= scaleX;
        _array[2] *= scaleX;
        _array[3] *= scaleX;
      }
      if( scaleY != 1.0 )
      {
        _array[4] *= scaleY;
        _array[5] *= scaleY;
        _array[6] *= scaleY;
        _array[7] *= scaleY;
      }
      if( scaleZ != 1.0 )
      {
        _array[8] *= scaleZ;
        _array[9] *= scaleZ;
        _array[10] *= scaleZ;
        _array[11] *= scaleZ;
      }
    }
    else if (scaleX != 1.0 || scaleY != 1.0 || scaleZ != 1.0)
    {      
      _array[0] = scaleX;
      _array[5] = scaleY;
      _array[10] = scaleZ;
      _isIdentity = false;
    }
    return *this;
  }

  Matrix4x4& Matrix4x4::rotateRadians(double rads, Vector3d axis)
  {
    if (rads == 0)
      return *this;

    double cosrads = cos(rads);
    double sinrads = sin(rads);

    if (axis != Vector3d(0,0,1)) 
    {      
      // general case, rotate current matrix around a vector
      double n1,n2,n3;

      n1 = axis.x();
      n2 = axis.y();
      n3 = axis.z();

      Matrix4x4 matrix(false); // careful here, no initialization, we need to initialize every term ourself
      matrix._array[0]  = n1*n1*(1-cosrads) + cosrads;  
      matrix._array[1]  = n1*n2*(1-cosrads) + n3*sinrads;
      matrix._array[2]  = n1*n3*(1-cosrads) - n2*sinrads;
      matrix._array[3]  = 0.0;

      matrix._array[4]  = n1*n2*(1-cosrads) - n3*sinrads;
      matrix._array[5]  = n2*n2*(1-cosrads) + cosrads;  
      matrix._array[6]  = n2*n3*(1-cosrads) + n1*sinrads;
      matrix._array[7]  = 0.0;

      matrix._array[8]  = n1*n3*(1-cosrads) + n2*sinrads;           
      matrix._array[9]  = n2*n3*(1-cosrads) - n1*sinrads;
      matrix._array[10] = n3*n3*(1-cosrads) + cosrads;  
      matrix._array[11] = 0.0;

      matrix._array[12] = 0.0;
      matrix._array[13] = 0.0;
      matrix._array[14] = 0.0;
      matrix._array[15] = 1.0;

      multiply(matrix);
    }
    else if (!_isIdentity) 
    {
      // rotate current matrix in the xy plane
      double a = _array[0]*cosrads + _array[4]*sinrads;
      double b = _array[4]*cosrads - _array[0]*sinrads;
      double c = _array[1]*cosrads + _array[5]*sinrads;
      double d = _array[5]*cosrads - _array[1]*sinrads;
      _array[0] = a;
      _array[4] = b;
      _array[1] = c;
      _array[5] = d;

      if (_array[3] || _array[7])
      {
        double g = _array[3]*cosrads + _array[7]*sinrads;
        double h = _array[7]*cosrads - _array[3]*sinrads;
        _array[3] = g;
        _array[7] = h;
      }
      if (_array[2] || _array[6])
      {
        double i = _array[2]*cosrads + _array[6]*sinrads;
        double j = _array[6]*cosrads - _array[2]*sinrads;
        _array[2] = i;
        _array[6] = j;
      }
    }
    else
    {
      // current matrix is identity, all we need to do is set it to a rotation in the xy plane.
      _array[0] = cosrads;
      _array[4] = -sinrads;
      _array[1] = sinrads;
      _array[5] = cosrads;
      _isIdentity = false;
    }    
    return *this;
  }

  Matrix4x4& Matrix4x4::rotateDegrees(double degs, Vector3d axis)
  {
    return rotateRadians(convertDegreesToRadians(degs), axis);
  }

  Matrix4x4& Matrix4x4::skew(double skew)
  {
    if (skew == 0.0)
      return *this;

    // we can optimize this function a bit more by hardcoding the matrix multiplication.
    Matrix4x4 matrix;

    matrix.setB( sin(convertDegreesToRadians(skew)) );
    matrix.setD( cos(convertDegreesToRadians(skew)) );
    matrix._isIdentity = false;

    multiply(matrix);
    
    return *this;
  }


  Matrix4x4& Matrix4x4::orthogonalProject(double left, double right, double bottom, double top, double zNear, double zFar)
  {
	  double fFn = zFar - zNear;
	  double fRl = right - left;
	  double fTb = top - bottom;


    Matrix4x4 matrix;
	  matrix(Row_0,Column_0) = 2 / fRl;
	  matrix(Row_1,Column_1) = 2 / fTb;
	  matrix(Row_2,Column_2) = -2 / fFn;
	  matrix(Row_3,Column_3) = 1;

	  matrix(Row_0,Column_3) = - ( right + left ) / fRl;
	  matrix(Row_1,Column_3) = - ( top + bottom ) / fTb;
	  matrix(Row_2,Column_3) = - ( zNear + zFar ) / fFn;

    matrix._isIdentity = false;
    multiply(matrix);

    return *this;
  }

  Matrix4x4& Matrix4x4::perspectiveProject(double left, double right, double bottom, double top, double zNear, double zFar)
  {	 
	  double fFn = zFar - zNear;
	  double fRl = right - left;
	  double fTb = top - bottom;


	  Matrix4x4 matrix;
	  matrix(Row_0,Column_0) = 2 * zNear / fRl;
	  matrix(Row_1,Column_1) = 2 * zNear / fTb;
	  matrix(Row_0,Column_2) = (right + left) / fRl;
	  matrix(Row_1,Column_2) = (top + bottom ) / fTb;
	  matrix(Row_2,Column_2) = -(zFar + zNear ) / fFn;
	  matrix(Row_2,Column_3) = (-2 * zFar * zNear ) / fFn;
	  matrix(Row_3,Column_2) = -1;
    matrix(Row_3,Column_3) = 0;

    matrix._isIdentity = false;
    multiply(matrix);

    return *this;
  }

  Matrix4x4& Matrix4x4::perspectiveProject(double verticalFieldOfViewInDegrees, double widthOverHeightAspectRatio, double zNear, double zFar)
  {
    double fovy = convertDegreesToRadians(verticalFieldOfViewInDegrees);
    double f = tan(M_PI/2.0-(fovy/2.0));
    double znmzf = zNear - zFar;

    Matrix4x4 matrix;
    matrix(Row_0,Column_0) = f/widthOverHeightAspectRatio;
    matrix(Row_1,Column_1) = f;
    matrix(Row_2,Column_2) = (zFar + zNear)/znmzf;
    matrix(Row_2,Column_3) = (2*zFar*zNear)/znmzf;
    matrix(Row_3,Column_2) = -1;
    matrix(Row_3,Column_3) = 0;

    matrix._isIdentity = false;
    multiply(matrix);

    return *this;
  }


  bool Matrix4x4::extractParameters( bool sxPos, bool syPos, //to resolve sign ambiguity for scaling
                                        double px, double py,
                                        double &tx, double &ty, double &tz,
                                        double &sx, double &sy, 
                                        double &angle, double &skew ) const
  {
    //The Matrix is defined as:
    // M = P T R K S P-1
    // where P : Pivot
    //       T : Translation
    //       R : Rotation (angle a)
    //       K : sKew  (angle s)
    //       S : Scale
    //The Matrix contains:
    //_members._byName._a = M[0] = Sx cos(a)
    //_members._byName._c = M[1] = Sx sin(a)
    //_members._byName._b = M[4] = Sy sin(s) cos(a) - Sy cos(s) sin(a)
    //_members._byName._d = M[5] = Sy sin(s) sin(a) + Sy cos(s) cos(a)
    //_members._byName._e = M[12] = -Px Sx cos(a) - Py Sy sin(s) cos(a) + Py Sy cos(s) sin(a) + Tx + Px
    //_members._byName._f = M[13] = -Px Sx sin(a) - Py Sy sin(s) sin(a) - Py Sy cos(s) cos(a) + Ty + Py
    //_members._byName._z = M[14] = Tz


    //the trivial part:
    tz = _array[14];

    //the very easy part:
    sx = sqrt(_array[0]*_array[0] + _array[1]*_array[1]);
    if(!sxPos)
      sx = -sx;

    //the easy part:
    angle = atan2(_array[1]/sx, _array[0]/sx)*180.0/M_PI;

    //the hard part:
    double aRad = angle*M_PI/180.0;
    Matrix4x4 RKS(_array[0], _array[4], 0.0,
                      _array[1], _array[5], 0.0,
                      0.0, 0.0, _array[14],
                      0.0, 0.0, 1.0);
    Matrix4x4 invR(cos(-aRad), -sin(-aRad), 0.0,
                      sin(-aRad), cos(-aRad), 0.0,
                      0.0, 0.0, 0.0,
                      0.0, 0.0, 1.0);

    Matrix4x4 KS = invR * RKS; 

    sy = sqrt(KS.getB()*KS.getB() + KS.getD()*KS.getD());
    if(!syPos)
      sy = -sy;

    skew = atan2(KS.getB()/sy, KS.getD()/sy)*180.0/M_PI;

    //the really hard part:
    Matrix4x4 P(1.0, 0.0, px,
                    0.0, 1.0, py,
                    0.0, 0.0, 0.0,
                    0.0, 0.0, 1.0);

    Matrix4x4 PTRKSPinv(*this);

    PTRKSPinv.multiply(P);
    P.inverse();
    P.multiply(PTRKSPinv); //P is now TRKS

    if ( !KS.inverse() ) //KS is now (KS)-1
    {
      KS = invR * RKS; //just in case KS got corrupted
      if(!KS.getA())
        KS.setA(1.0 / 1024.0);
      if(!KS.getB() && !KS.getD())
        KS.setD(1.0 / 1024.0);

      return false;
    }

    P.multiply(KS); //P is now TR
    P.multiply(invR); //P is now T

    tx = P.getE();
    ty = P.getF();

    return true;
  }

  namespace
  {
    double norm1_3x3(const Matrix4x4 &m)
    {
      double max;
      double t;

      max = fabs(m(Row_0, Column_0)) +
            fabs(m(Row_1, Column_0)) +
            fabs(m(Row_2, Column_0));

      if((t = fabs(m(Row_0, Column_1)) +
              fabs(m(Row_1, Column_1)) +
              fabs(m(Row_2, Column_1)) ) > max)
      {
        max = t;
      }

      if((t = fabs(m(Row_0, Column_2)) +
              fabs(m(Row_1, Column_2)) +
              fabs(m(Row_2, Column_2)) ) > max)
      {
        max = t;
      }

      return max;
    }

    double normInf_3x3(const Matrix4x4 &m)
    {
      double max;
      double t;

      max = fabs(m(Row_0, Column_0)) +
            fabs(m(Row_0, Column_1)) +
            fabs(m(Row_0, Column_2));

      if((t = fabs(m(Row_1, Column_0)) +
              fabs(m(Row_1, Column_1)) +
              fabs(m(Row_1, Column_2)) ) > max)
      {
        max = t;
      }

      if((t = fabs(m(Row_2, Column_0)) +
              fabs(m(Row_2, Column_1)) +
              fabs(m(Row_2, Column_2)) ) > max)
      {
        max = t;
      }

      return max;
    }

    void adjointT_3x3(const Matrix4x4 &m, Matrix4x4 &r)
    {
      r(Row_0, Column_0) = m(Row_1, Column_1) * m(Row_2, Column_2) - m(Row_1, Column_2) * m(Row_2, Column_1);
      r(Row_0, Column_1) = m(Row_1, Column_2) * m(Row_2, Column_0) - m(Row_1, Column_0) * m(Row_2, Column_2);
      r(Row_0, Column_2) = m(Row_1, Column_0) * m(Row_2, Column_1) - m(Row_1, Column_1) * m(Row_2, Column_0);

      r(Row_1, Column_0) = m(Row_2, Column_1) * m(Row_0, Column_2) - m(Row_2, Column_2) * m(Row_0, Column_1);
      r(Row_1, Column_1) = m(Row_2, Column_2) * m(Row_0, Column_0) - m(Row_2, Column_0) * m(Row_0, Column_2);
      r(Row_1, Column_2) = m(Row_2, Column_0) * m(Row_0, Column_1) - m(Row_2, Column_1) * m(Row_0, Column_0);

      r(Row_2, Column_0) = m(Row_0, Column_1) * m(Row_1, Column_2) - m(Row_0, Column_2) * m(Row_1, Column_1);
      r(Row_2, Column_1) = m(Row_0, Column_2) * m(Row_1, Column_0) - m(Row_0, Column_0) * m(Row_1, Column_2);
      r(Row_2, Column_2) = m(Row_0, Column_0) * m(Row_1, Column_1) - m(Row_0, Column_1) * m(Row_1, Column_0);
    }

    /*
     *  Retrieved from OpenSG polarDecompose and spectralDecompose algorithms.
     */

    void polarDecompose( const Matrix4x4 &M, Matrix4x4 &Q, Matrix4x4 &S, double &det )
    {
      double const TOL = 1.0e-6;

      Matrix4x4 Mk;
      Matrix4x4 Ek;
      Matrix4x4 MkAdjT;

      M.getTranspose( Mk );

      double Mk_one = norm1_3x3(Mk);
      double Mk_inf = normInf_3x3(Mk);

      double MkAdjT_one;
      double MkAdjT_inf;

      double Ek_one;
      double Mk_det;

      do
      {
        // compute transpose of adjoint
        adjointT_3x3(Mk, MkAdjT);

        // Mk_det = det(Mk) -- computed from the adjoint        
        Mk_det = Mk(Row_0, Column_0) * MkAdjT(Row_0, Column_0) +
                 Mk(Row_0, Column_1) * MkAdjT(Row_0, Column_1) +
                 Mk(Row_0, Column_2) * MkAdjT(Row_0, Column_2);

        // should this be a close to zero test ?
        if(Mk_det == 0.0)
        {
          break;
        }

        MkAdjT_one = norm1_3x3(MkAdjT);
        MkAdjT_inf = normInf_3x3(MkAdjT);

        // compute update factors
        double gamma = sqrt( sqrt((MkAdjT_one * MkAdjT_inf) / (Mk_one * Mk_inf)) / fabs(Mk_det) );

        double g1 = 0.5 * gamma;
        double g2 = 0.5 / (gamma * Mk_det);

        Ek = Mk;

        for (Row r = Row_0 ; r <= Row_3 ; ++r ) for (Column c = Column_0 ; c <= Column_3 ; ++c)
        {
          Mk(r,c) = g1 * Mk(r,c) + g2 * MkAdjT(r,c);
          Ek(r,c) -= Mk(r,c);
        }

        Ek_one = norm1_3x3(Ek);
        Mk_one = norm1_3x3(Mk);
        Mk_inf = normInf_3x3(Mk);

      } while(Ek_one > (Mk_one * TOL));

      Q = Mk;
      Q.transpose();

      S = Mk;
      S.multiply(M);

      for (Column c = Column_0 ; c <= Column_2 ; ++c)
      {
        for (Row r = (Row)c ; r <= Row_2 ; ++r )
        {
          S(r,c) = S((Row)c, (Column)r) = 0.5 * (S(r,c) + S((Row)c, (Column)r));
        }
      }

      det = Mk_det;
    }

    void spectralDecompose(const Matrix4x4 &S, Matrix4x4 &SO, double &sx, double &sy, double &sz)
    {
      unsigned const next[3]       = {1, 2, 0};
      unsigned const maxIterations = 20;

      double diag[3];
      double offDiag[3];

      diag[0] = S(Row_0, Column_0);
      diag[1] = S(Row_1, Column_1);
      diag[2] = S(Row_2, Column_2);

      offDiag[0] = S(Row_1, Column_2);
      offDiag[1] = S(Row_2, Column_0);
      offDiag[2] = S(Row_0, Column_1);

      for(unsigned iter = 0; iter < maxIterations; ++iter)
      {
        double sm = fabs(offDiag[0]) + fabs(offDiag[1]) + fabs(offDiag[2]);

        if(sm == 0.0)
        {
          break;
        }

        for(int i = 2; i >= 0; --i)
        {
          unsigned p = next[i];
          unsigned q = next[p];

          double absOffDiag = fabs(offDiag[i]);
          double g          = 100.0 * absOffDiag; 

          if(absOffDiag > 0.0)
          {
            double t;
            double h    = diag[q] - diag[p];
            double absh = fabs(h);

            if(absh + g == absh)
            {
              t = offDiag[i] / h;
            }
            else
            {
              double theta = 0.5 * h / offDiag[i];
              t = 1.0 / (fabs(theta) + sqrt(theta * theta + 1.0));

              t = theta < 0.0 ? -t : t;
            }

            double c = 1.0 / sqrt(t * t + 1.0);
            double s = t * c;

            double tau = s / (c + 1.0);
            double ta  = t * offDiag[i];

            offDiag[i] = 0.0;

            diag[p] -= ta;
            diag[q] += ta;

            double offDiagq = offDiag[q];

            offDiag[q] -= s * (offDiag[p] + tau * offDiag[q]);
            offDiag[p] += s * (offDiagq   - tau * offDiag[p]);

            for(int j = 2; j >= 0; --j)
            {
              double a = SO(Row(j), Column(p));
              double b = SO(Row(j), Column(q));

              SO(Row(j), Column(p)) -= s * (b + tau * a);
              SO(Row(j), Column(q)) += s * (a - tau * b);
            }
          }
        }
      }

      sx = diag[0];
      sy = diag[1];
      sz = diag[2];
    }
  }


  bool Matrix4x4::extractParameters(
      double px, double py, double pz,
      double &tx, double &ty, double &tz,
      double &sx, double &sy, double &sz,
      double &ax, double &ay, double &az ) const
  {
    Matrix4x4 A;
    A.translate(-px,-py,-pz);
    A.multiply(*this);
    A.translate(px,py,pz);

    double flip;

    //  retrieve translation component
    tx = A._array[12];
    ty = A._array[13];
    tz = A._array[14];

    A._array[12] = 0.0;
    A._array[13] = 0.0;
    A._array[14] = 0.0;

    A._array[3] = 0.0;
    A._array[7] = 0.0;
    A._array[11] = 0.0;

    Matrix4x4 Q,S;
    double det;

    //  polar decomposition -- retrieve angles
    polarDecompose(A, Q, S, det);

    if (det < 0.0)
    {
      for (int i = 0; i < 16; i++)
        Q._array[i] *= -1.0;
      flip = -1.0;
    }
    else
    {
      flip = 1.0;
    }

    //  Retrieve angles from orientation matrix
    double cy = sqrt(Q(Row_0, Column_0)*Q(Row_0, Column_0) + Q(Row_1, Column_0)*Q(Row_1, Column_0));

    if (cy > 16*FLT_EPSILON)
    {
      ax = atan2(Q(Row_2, Column_1), Q(Row_2, Column_2)) * 180.0/M_PI;
      ay = atan2(-Q(Row_2, Column_0), cy) * 180.0/M_PI;
      az = atan2(Q(Row_1, Column_0), Q(Row_0, Column_0)) * 180.0/M_PI;
    }
    else
    {
      ax = atan2(-Q(Row_1, Column_2), Q(Row_1, Column_1)) * 180.0/M_PI;
      ay = atan2(-Q(Row_2, Column_0), cy) * 180.0/M_PI;
      az = 0;
    }

    //  spectral decomposition -- retrieve scaling values
    Matrix4x4 SO;
    spectralDecompose(S, SO, sx, sy, sz);

    sx *= flip;
    sy *= flip;
    sz *= flip;

    return true;
  }

  bool Matrix4x4::inverse()
  {
    if (_isIdentity)
      return true;

    double inv[4][4];
    double mat[4][4];
    memcpy(mat, _array, sizeof(_array));
    if ( inverse(inv, mat) )
    {
      memcpy(_array, inv, sizeof(_array));
      return true;
    }

    return false;
  }

  bool Matrix4x4::getInverse( Math::Matrix4x4 &matrix ) const
  {
    if (_isIdentity)
    {
      matrix.clear();
      return true;
    }

    double inv[4][4];
    double mat[4][4];
    memcpy(mat, _array, sizeof(_array));
    if ( inverse(inv, mat) )
    {
      memcpy(matrix._array, inv, sizeof(_array));
      return true;
    }

    return false;
  }

  bool Matrix4x4::isSingular() const
  {
    if (_isIdentity)
      return false;

    double mat[4][4];
    memcpy(mat, _array, sizeof(_array));

    bool even;
    int index[4];

    // LU Decomposition.
    return luDecompose(mat, index, &even);
  }

  bool Matrix4x4::transpose()
  {
    if (!_isIdentity)
    {
      Matrix4x4 m;
      getTranspose( m );
      *this = m;
    }

    return true;
  }

  bool Matrix4x4::getTranspose( Math::Matrix4x4 &matrix ) const
  {
    if (_isIdentity)
    {
      matrix.clear();
      return true;
    }

    for (Row i = Row_0; i <= Row_3; i++)
      for (Column j = Column_0; j <= Column_3; j++)
        matrix(i, j) = (*this)((Row)j, (Column)i);

    matrix._isIdentity = _isIdentity;

    return true;
  }

void Matrix4x4::rotatePlane( const Vector3d &normal, const Point3d &origin )
{
    Vector3d xAxis, yAxis, zAxis;
    
    // We need a vector that is at 90 degree with the _normal.
    // The following guarantees that the dot-product of xAxis and _normal will be 0.0.
    double x = fabs(normal.x());
    double y = fabs(normal.y());
    double z = fabs(normal.z());

    if (x > y && x > z)
      xAxis.setXYZ(-normal.y(), normal.x(), 0);
    else if (y > z)
      xAxis.setXYZ(0, -normal.z(), normal.y());
    else
      xAxis.setXYZ(normal.z(), 0, -normal.x());
    
    yAxis = normal.getCrossProduct(xAxis);
    zAxis = normal;

    _array[0] = xAxis(Axis3d_X); _array[4] = yAxis(Axis3d_X); _array[8]  = zAxis(Axis3d_X); _array[12] = origin(Axis3d_X);
    _array[1] = xAxis(Axis3d_Y); _array[5] = yAxis(Axis3d_Y); _array[9]  = zAxis(Axis3d_Y); _array[13] = origin(Axis3d_Y);  
    _array[2] = xAxis(Axis3d_Z); _array[6] = yAxis(Axis3d_Z); _array[10] = zAxis(Axis3d_Z); _array[14] = origin(Axis3d_Z);
    _array[3] = 0;               _array[7] = 0;               _array[11] = 0;               _array[15] = 1;    

    _isIdentity = !memcmp(_array, _identityPattern, sizeof(_array));
}
void Matrix4x4::print(const char *text) const
  {
    fprintf(stdout, "%s:\n%12.5f %12.5f %12.5f %12.5f\n%12.5f %12.5f %12.5f %12.5f\n%12.5f %12.5f %12.5f %12.5f\n%12.5f %12.5f %12.5f %12.5f\n",
      text && *text ? text : "Matrix",
      _array[0], _array[4], _array[8],  _array[12],
      _array[1], _array[5], _array[9],  _array[13],
      _array[2], _array[6], _array[10], _array[14],
      _array[3], _array[7], _array[11], _array[15]);
  }
  void Matrix4x4::getValues(float  m[16]) const
  {
    for (int i = 0; i < 16; i++)
      m[i]=(float)_array[i]; 
  }
  void Matrix4x4::getValues(double  m[16]) const
  {
    memcpy(m, _array, sizeof(_array));
  }

  Matrix4x4& Matrix4x4::operator=(const Matrix4x4& m)
  {
    if (&m != this)
    {
      memcpy(_array, m._array, sizeof(_array));
      _isIdentity = m._isIdentity;
    }
    return *this;
  }

  bool Matrix4x4::operator==(const Matrix4x4& m) const
  {
    if (_isIdentity && m._isIdentity)
      return true;
    return memcmp(_array, m._array, sizeof(_array)) == 0;    
  }

  bool Matrix4x4::operator!=(const Matrix4x4& m) const
  {
    if (_isIdentity && m._isIdentity)
      return false;
    return memcmp(_array, m._array, sizeof(_array)) != 0;    
  }

  Matrix4x4 Matrix4x4::operator*(const Matrix4x4& m) const
  {
    if (m._isIdentity)
      return Matrix4x4(*this);

    if (_isIdentity)
      return Matrix4x4(m);

    Matrix4x4 matrix(false); // careful here, no initialization, we need to initialize every term ourself
    matrix._array[0] = _array[0]*m._array[0] + _array[4]*m._array[1] + _array[8]*m._array[2]  + _array[12]*m._array[3];
    matrix._array[1] = _array[1]*m._array[0] + _array[5]*m._array[1] + _array[9]*m._array[2]  + _array[13]*m._array[3];
    matrix._array[2] = _array[2]*m._array[0] + _array[6]*m._array[1] + _array[10]*m._array[2] + _array[14]*m._array[3];
    matrix._array[3] = _array[3]*m._array[0] + _array[7]*m._array[1] + _array[11]*m._array[2] + _array[15]*m._array[3];

    matrix._array[4] = _array[0]*m._array[4] + _array[4]*m._array[5] + _array[8]*m._array[6]  + _array[12]*m._array[7];
    matrix._array[5] = _array[1]*m._array[4] + _array[5]*m._array[5] + _array[9]*m._array[6]  + _array[13]*m._array[7];
    matrix._array[6] = _array[2]*m._array[4] + _array[6]*m._array[5] + _array[10]*m._array[6] + _array[14]*m._array[7];
    matrix._array[7] = _array[3]*m._array[4] + _array[7]*m._array[5] + _array[11]*m._array[6] + _array[15]*m._array[7];

    matrix._array[8] = _array[0]*m._array[8]+ _array[4]*m._array[9]+ _array[8]*m._array[10] + _array[12]*m._array[11];
    matrix._array[9] = _array[1]*m._array[8] + _array[5]*m._array[9] + _array[9]*m._array[10]  + _array[13]*m._array[11];
    matrix._array[10] = _array[2]*m._array[8] + _array[6]*m._array[9] + _array[10]*m._array[10] + _array[14]*m._array[11];
    matrix._array[11] = _array[3]*m._array[8]+ _array[7]*m._array[9]+ _array[11]*m._array[10]+ _array[15]*m._array[11];

    matrix._array[12] = _array[0]*m._array[12]+ _array[4]*m._array[13]+ _array[8]*m._array[14] + _array[12]*m._array[15];
    matrix._array[13] = _array[1]*m._array[12]+ _array[5]*m._array[13]+ _array[9]*m._array[14] + _array[13]*m._array[15];
    matrix._array[14] = _array[2]*m._array[12]+ _array[6]*m._array[13]+ _array[10]*m._array[14]+ _array[14]*m._array[15];
    matrix._array[15] = _array[3]*m._array[12]+ _array[7]*m._array[13]+ _array[11]*m._array[14]+ _array[15]*m._array[15];

    return matrix;
  }

  Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& m)
  {
    if (!m._isIdentity)
    {
      if (_isIdentity)
        *this = m;
      else
      {
        //  copy the 12 first terms of the matrix to temp vector -- the 4 last terms of the matrix
        //  can be used as is as they won't get modified during calculations
        double e[12];
        memcpy(e, _array, sizeof(e));

        _array[0] = e[0]*m._array[0] + e[4]*m._array[1] + e[8]*m._array[2]  + _array[12]*m._array[3];
        _array[1] = e[1]*m._array[0] + e[5]*m._array[1] + e[9]*m._array[2]  + _array[13]*m._array[3];
        _array[2] = e[2]*m._array[0] + e[6]*m._array[1] + e[10]*m._array[2] + _array[14]*m._array[3];
        _array[3] = e[3]*m._array[0] + e[7]*m._array[1] + e[11]*m._array[2] + _array[15]*m._array[3];

        _array[4] = e[0]*m._array[4] + e[4]*m._array[5] + e[8]*m._array[6]  + _array[12]*m._array[7];
        _array[5] = e[1]*m._array[4] + e[5]*m._array[5] + e[9]*m._array[6]  + _array[13]*m._array[7];
        _array[6] = e[2]*m._array[4] + e[6]*m._array[5] + e[10]*m._array[6] + _array[14]*m._array[7];
        _array[7] = e[3]*m._array[4] + e[7]*m._array[5] + e[11]*m._array[6] + _array[15]*m._array[7];

        _array[8] = e[0]*m._array[8] + e[4]*m._array[9] + e[8]*m._array[10] + _array[12]*m._array[11];
        _array[9] = e[1]*m._array[8] + e[5]*m._array[9] + e[9]*m._array[10]  + _array[13]*m._array[11];
        _array[10] = e[2]*m._array[8] + e[6]*m._array[9] + e[10]*m._array[10] + _array[14]*m._array[11];
        _array[11] = e[3]*m._array[8] + e[7]*m._array[9] + e[11]*m._array[10] + _array[15]*m._array[11];

        _array[12] = e[0]*m._array[12] + e[4]*m._array[13] + e[8]*m._array[14] + _array[12]*m._array[15];
        _array[13] = e[1]*m._array[12] + e[5]*m._array[13] + e[9]*m._array[14] + _array[13]*m._array[15];
        _array[14] = e[2]*m._array[12] + e[6]*m._array[13] + e[10]*m._array[14] + _array[14]*m._array[15];
        _array[15] = e[3]*m._array[12] + e[7]*m._array[13] + e[11]*m._array[14] + _array[15]*m._array[15];
      }
    }
    return *this;
  }

  Point3d Matrix4x4::operator*(const Point3d& p) const
  {
    if (_isIdentity)
      return p;

    double w = _array[3]*p.x() + _array[7]*p.y() + _array[11]*p.z()  + _array[15];
    return Point3d(
      (_array[0]*p.x() + _array[4]*p.y() + _array[8]*p.z() + _array[12]) / w,
      (_array[1]*p.x() + _array[5]*p.y() + _array[9]*p.z() + _array[13]) / w,
      (_array[2]*p.x() + _array[6]*p.y() + _array[10]*p.z() + _array[14]) / w);
  }

  Vector3d Matrix4x4::operator*(const Vector3d& v) const
  {
    if (_isIdentity)
      return Vector3d(v);

    return Vector3d(
      _array[0]*v.x() + _array[4]*v.y() + _array[8]*v.z(),
      _array[1]*v.x() + _array[5]*v.y() + _array[9]*v.z(),
      _array[2]*v.x() + _array[6]*v.y() + _array[10]*v.z());
  }

  double Matrix4x4::truckFactor() const
  {
    double x0(0.0),
          y0(0.0),
          x1(1.0),
          y1(1.0),
          factor;
    multiplyPoint(&x0, &y0);
    multiplyPoint(&x1, &y1);

    factor = sqrt(((x1-x0) * (x1-x0) + (y1-y0) * (y1-y0)) / 2.0);
    
    return(factor);
  }

  bool Matrix4x4::luDecompose(double a[4][4],
                              int index[4],
                              bool* even) const
  {
    int i, imax=0, j, k;
    double big, dum, sum, temp;
    const int n = 4;
    double vv[4];

    *even = true;
    for (i = 0; i < n; i++) {
      big=0.0;
      for (j = 0; j < n; j++)
        if ((temp = fabs(a[i][j])) > big)
          big = temp;
      if (big == 0.0)
        return false;
      vv[i] = 1.0 / big;
    }

    for (j = 0; j < n; j++) {
      for (i = 0; i < j ;i++) {
        sum = a[i][j];
        for (k = 0; k < i; k++)
          sum -= a[i][k] * a[k][j];
        a[i][j] = sum;
      }
      big = 0.0;
      for (i = j; i < n; i++) {
        sum = a[i][j];
        for (k = 0; k < j; k++)
          sum -= a[i][k] * a[k][j];
        a[i][j] = sum;
        if ((dum = vv[i] * fabs(sum)) >= big) {
          big = dum;
          imax = i;
        }
      }
      if (j != imax) {
        for (k = 0; k < n; k++) {
          dum = a[imax][k];
          a[imax][k] = a[j][k];
          a[j][k] = dum;
        }
        *even = !(*even);
        vv[imax] = vv[j];
      }
      index[j] = imax;
      if (fabs(a[j][j]) < DBL_EPSILON)
        return false;;
      if (j != n - 1) {
        dum= 1.0 / (a[j][j]);
        for (i = j + 1; i < n; i++)
          a[i][j] *= dum;
      }
    }

    return true;
  }

  void Matrix4x4::luSolve(const double a[4][4],
                              const int index[4],
                              double b[4]) const
  {
    int i, ii=-1, ip, j;
    double sum;
    const int n = 4;

    for (i = 0; i < n; i++) {
      ip = index[i];
      sum = b[ip];
      b[ip] = b[i];
      if (ii >= 0)
        for (j = ii; j < i; j++)
          sum -= a[i][j] * b[j];
      else if (sum) 
        ii=i;
      b[i]=sum;
    }
    for (i = n - 1; i >= 0; i--) {
      sum = b[i];
      for (j = i + 1; j < n; j++) 
        sum -= a[i][j] * b[j];
      b[i] = sum / a[i][i];
    }
  }

  bool Matrix4x4::inverse(double inv[4][4],
                          double mat[4][4]) const
  {
    bool even;
    const int n = 4;
    int i, j;

    double col[4];
    int index[4];

    // LU Decomposition.
    if ( luDecompose(mat, index, &even) )
    {
      // Find the inverse by columns.
      for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++)
          col[i] = 0.0;
        col[j] = 1.0;
        luSolve(mat, index, col);
        for (i = 0; i < n; i++)
          inv[i][j] = col[i];
      }

      return true;
    }

    return false;
  }

  bool Matrix4x4::hasQuadmap() const
  {
    return _array[3] != 0.0 || _array[7] != 0.0 || _array[11] != 0.0;
  }


  bool Matrix4x4::isConstantZ() const
  {
    return (*this)(Row_2, Column_0) == 0.0 && 
           (*this)(Row_3, Column_0) == 0.0 && 
           (*this)(Row_2, Column_1) == 0.0 &&
           (*this)(Row_3, Column_1) == 0.0;
  }


  bool Matrix4x4::isTransform2d() const
  {
    double wAbs = fabs((*this)(Row_3, Column_3));
    if (wAbs <= 0.00001)
      return false;

    double epsilonAbs = 0;
    epsilonAbs += fabs( (*this)(Row_2, Column_0) );
    epsilonAbs += fabs( (*this)(Row_3, Column_0) );
    epsilonAbs += fabs( (*this)(Row_2, Column_1) );
    epsilonAbs += fabs( (*this)(Row_3, Column_1) );

    return (epsilonAbs / wAbs) < 0.00001;
  }

  bool Matrix4x4::isPerspectiveProjection() const
  {    
    double epsilonAbs = 0;
    epsilonAbs += fabs( (*this)(Row_3, Column_0) );
    epsilonAbs += fabs( (*this)(Row_3, Column_1) );
    epsilonAbs += fabs( (*this)(Row_3, Column_2) );
    return epsilonAbs > 0.00001;
  }


  Matrix4x4& Matrix4x4::lookAt(const Point3d &eye, const Point3d &center, const Vector3d &upDirection)
  {    
    /* Z vector */
    Vector3d zAxis = eye - center;
    zAxis.normalize();

    /* Y vector */    
    Vector3d yAxis = upDirection;

    /* X vector = Y cross Z */
    Vector3d xAxis = yAxis ^ zAxis;

    /* Recompute Y = Z cross X */
    yAxis = zAxis ^ xAxis;

    yAxis.normalize();
    xAxis.normalize();

    Matrix4x4 matrix(xAxis, yAxis, zAxis, eye);
    multiply(matrix);
    
    return *this;
}

}
