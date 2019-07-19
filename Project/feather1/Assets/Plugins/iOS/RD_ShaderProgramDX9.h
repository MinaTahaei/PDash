
#ifndef _RD_SHADER_PROGRAM_DX9_H_
#define _RD_SHADER_PROGRAM_DX9_H_

#include "PL_Configure.h"

#if defined(SUPPORT_RENDERER_DX9)

/*!
 *  @class RD_ShaderProgramDX9
 *  Shader interface.
 */
class RD_ShaderProgramDX9
{
public:

  RD_ShaderProgramDX9();
  virtual ~RD_ShaderProgramDX9();

  /* common methods */
  virtual void load() =0;
  virtual void unload() =0;

  virtual void activate() =0;
  virtual void deactivate() =0;

  virtual void setDevice( void *device ) =0;

  /* common uniform setters */
  virtual void setMatrix( const float *matrix ) =0;
  //virtual void setTextureSampler0( int samplerId ) =0;

  /* static util methods */
  static void setNullShader( void *device );
};

/*!
 *  @class RD_ShaderProgramFxDX9
 *  Plain shader implementation.
 */
class RD_ShaderProgramFxDX9 : public RD_ShaderProgramDX9
{
public:

  RD_ShaderProgramFxDX9( void *device );
  virtual ~RD_ShaderProgramFxDX9();

  /* common methods */
  virtual void load();
  virtual void unload();

  virtual void activate();
  virtual void deactivate();

  virtual void setDevice( void *device );

  /* common uniform setters */
  virtual void setMatrix( const float *matrix );

  /* specific methods */
  void setBoneMatrices( const float *matrixArray, unsigned count );

private:

  class Impl;
  Impl *_i;
};

#endif /* SUPPORT_RENDERER_DX9 */

#endif /* _RD_SHADER_PROGRAM_DX9_H_ */
