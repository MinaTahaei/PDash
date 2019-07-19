
#ifndef _RD_SHADER_PROGRAM_GL_H_
#define _RD_SHADER_PROGRAM_GL_H_

#include "PL_Configure.h"

#if defined(SUPPORT_RENDERER_GLES2) || defined(SUPPORT_RENDERER_GL)

/*!
 *  @class RD_ShaderProgramGL
 *  Shader interface.
 */
class RD_ShaderProgramGL
{
public:

  RD_ShaderProgramGL();
  virtual ~RD_ShaderProgramGL();

  /* common methods */
  virtual void load() =0;
  virtual void unload() =0;

  virtual const char* vertexShader() = 0;
  virtual const char* fragmentShader() = 0;

  virtual void activate() =0;
  virtual void deactivate() =0;

  /* common uniform setters */
  virtual void setMatrix( const float *matrix ) =0;
  virtual void setTextureSampler0( int samplerId ) =0;
};

/*!
*  @class RD_ShaderProgramPlainGL
*  Plain implementation.
*/
class RD_ShaderProgramPlainGL : public RD_ShaderProgramGL
{
public:

  /* Uniform attributes */
  enum
  {
    OGL_VERTEX_ATTRIB_POSITION,
    OGL_VERTEX_ATTRIB_COLOR,
    OGL_VERTEX_ATTRIB_TEXCOORDS_0,
    OGL_NUM_ATTRIBUTES
  };

  /* Uniform variables */
  enum
  {
    OGL_UNIFORM_MVP_MATRIX,
    OGL_UNIFORM_TEXTURE_0,
    OGL_NUM_UNIFORMS
  };

public:

  RD_ShaderProgramPlainGL();
  virtual ~RD_ShaderProgramPlainGL();

  /* common methods */
  virtual void load();
  virtual void unload();

  virtual const char* vertexShader();
  virtual const char* fragmentShader();

  virtual void activate();
  virtual void deactivate();

  /* common uniform setters */
  virtual void setMatrix( const float *matrix );
  virtual void setTextureSampler0( int activeTexture );

private:

  class Impl;
  Impl *_i;
};

/*!
*  @class RD_ShaderProgramFxGL
*  Fx shader implementation.
*/
class RD_ShaderProgramFxGL : public RD_ShaderProgramGL
{
public:

  /* Uniform attributes */
  enum
  {
    OGL_VERTEX_ATTRIB_POSITION,
    OGL_VERTEX_ATTRIB_COLOR,
    OGL_VERTEX_ATTRIB_TEXCOORDS_0,
    OGL_VERTEX_ATTRIB_FX_PARAMS_0,
    OGL_VERTEX_ATTRIB_FX_VIEWPORT_0,
    OGL_VERTEX_ATTRIB_BONE_INDEX,
    OGL_NUM_ATTRIBUTES
  };

  /* Uniform variables */
  enum
  {
    OGL_UNIFORM_MVP_MATRIX,
    OGL_UNIFORM_TEXTURE_0,
    OGL_UNIFORM_TEXTURE_1,

    OGL_UNIFORM_BONE_MATRIX_0,
    OGL_UNIFORM_BONE_MATRIX_1,
    OGL_UNIFORM_BONE_MATRIX_2,
    OGL_UNIFORM_BONE_MATRIX_3,
    OGL_UNIFORM_BONE_MATRIX_4,
    OGL_UNIFORM_BONE_MATRIX_5,
    OGL_UNIFORM_BONE_MATRIX_6,
    OGL_UNIFORM_BONE_MATRIX_7,
    OGL_UNIFORM_BONE_MATRIX_8,
    OGL_UNIFORM_BONE_MATRIX_9,
    OGL_UNIFORM_BONE_MATRIX_10,
    OGL_UNIFORM_BONE_MATRIX_11,
    OGL_UNIFORM_BONE_MATRIX_12,
    OGL_UNIFORM_BONE_MATRIX_13,
    OGL_UNIFORM_BONE_MATRIX_14,
    OGL_UNIFORM_BONE_MATRIX_15,
    OGL_UNIFORM_BONE_MATRIX_16,
    OGL_UNIFORM_BONE_MATRIX_17,
    OGL_UNIFORM_BONE_MATRIX_18,
    OGL_UNIFORM_BONE_MATRIX_19,
    OGL_UNIFORM_BONE_MATRIX_20,
    OGL_UNIFORM_BONE_MATRIX_21,
    OGL_UNIFORM_BONE_MATRIX_22,
    OGL_UNIFORM_BONE_MATRIX_23,
    OGL_UNIFORM_BONE_MATRIX_24,
    OGL_UNIFORM_BONE_MATRIX_25,
    OGL_UNIFORM_BONE_MATRIX_26,
    OGL_UNIFORM_BONE_MATRIX_27,
    OGL_UNIFORM_BONE_MATRIX_28,
    OGL_UNIFORM_BONE_MATRIX_29,
    OGL_UNIFORM_BONE_MATRIX_30,
    OGL_UNIFORM_BONE_MATRIX_31,

    OGL_NUM_UNIFORMS
  };

public:

  RD_ShaderProgramFxGL();
  virtual ~RD_ShaderProgramFxGL();

  /* common methods */
  virtual void load();
  virtual void unload();

  virtual const char* vertexShader();
  virtual const char* fragmentShader();

  virtual void activate();
  virtual void deactivate();

  /* common uniform setters */
  virtual void setMatrix( const float *matrix );
  virtual void setTextureSampler0( int texture );

  /* specific methods */
  void setTextureSampler1( int texture );

  void setBoneMatrices( const float *matrixArray, unsigned count );

private:

  class Impl;
  Impl *_i;
};

#endif /* SUPPORT_RENDERER_GLES2 || SUPPORT_RENDERER_GL */

#endif /* _RD_SHADER_PROGRAM_GL_H_ */
