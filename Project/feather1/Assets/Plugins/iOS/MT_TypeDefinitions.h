
/*
 * Copyright ToonBoom Animation Inc. 
 * This file is part of the Toonboom Animation SDK
 */


#ifndef MT_TypeDefinitions_H
#define MT_TypeDefinitions_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace Math
{
  class Matrix4x4;
  class Point3d;
  class Vector3d;

  enum Axis2d { Axis2d_X, Axis2d_Y };
  Axis2d operator++(Axis2d &rs);
  Axis2d operator++(Axis2d &rs, int);

  enum Axis3d { Axis3d_X, Axis3d_Y, Axis3d_Z };
  Axis3d operator++(Axis3d &rs);
  Axis3d operator++(Axis3d &rs, int);

  enum Row { Row_0, Row_1, Row_2, Row_3 };
  Row operator++(Row &rs);
  Row operator++(Row &rs, int);

  enum Column { Column_0, Column_1, Column_2, Column_3 };
  Column operator++(Column &rs);
  Column operator++(Column &rs, int);


  // -------------- inline functions

  inline Axis2d operator++(Axis2d &rs)
  {
    return rs = (Axis2d)(rs + 1);
  }

  inline Axis2d operator++(Axis2d &rs, int)
  {
    return rs = (Axis2d)(rs + 1);
  }

  inline Axis3d operator++(Axis3d &rs)
  {
    return rs = (Axis3d)(rs + 1);
  }

  inline Axis3d operator++(Axis3d &rs, int)
  {
    return rs = (Axis3d)(rs + 1);
  }

  inline Column operator++(Column &rs)
  {
    return rs = (Column)(rs + 1);
  }

  inline Column operator++(Column &rs, int)
  {
    return rs = (Column)(rs + 1);
  }

  inline Row operator++(Row &rs)
  {
    return rs = (Row)(rs + 1);
  }

  inline Row operator++(Row &rs, int)
  {
    return rs = (Row)(rs + 1);
  }

}

#endif
