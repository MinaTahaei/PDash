
#ifndef _RD_SHADER_PROGRAM_DX11_H_
#define _RD_SHADER_PROGRAM_DX11_H_

#include "PL_Configure.h"

#if defined(SUPPORT_RENDERER_DX11)

/*!
 *  @class RD_ShaderProgramDX11
 *  Shader interface.
 */
class RD_ShaderProgramDX11
{
public:

  typedef void* Ptr_t;

public:

  RD_ShaderProgramDX11();
  virtual ~RD_ShaderProgramDX11();

  /* common methods */
  virtual void load() =0;
  virtual void unload() =0;

  virtual void activate( Ptr_t context ) =0;
  virtual void deactivate() =0;

  virtual void setDevice( Ptr_t device ) =0;

  virtual void updateConstants() =0;

  /* common uniform setters */
  virtual void setMatrix( const float *matrix ) =0;
  virtual void setTextureSampler0( Ptr_t sampler ) =0;
};

/*!
 *  @class RD_ShaderProgramPlainDX11
 *  Plain shader implementation.
 */
class RD_ShaderProgramPlainDX11 : public RD_ShaderProgramDX11
{
public:

  RD_ShaderProgramPlainDX11( Ptr_t device );
  virtual ~RD_ShaderProgramPlainDX11();

  /* common methods */
  virtual void load();
  virtual void unload();

  virtual void activate( Ptr_t context );
  virtual void deactivate();

  virtual void setDevice( Ptr_t device );

  virtual void updateConstants();

  /* common uniform setters */
  virtual void setMatrix( const float *matrix );
  virtual void setTextureSampler0( Ptr_t sampler );

  /* specific methods */
  // ...

private:

  class Impl;
  Impl *_i;
};

/*!
 *  @class RD_ShaderProgramPlainDX11
 *  Plain shader implementation.
 */
class RD_ShaderProgramFxDX11 : public RD_ShaderProgramDX11
{
public:

  RD_ShaderProgramFxDX11( Ptr_t device );
  virtual ~RD_ShaderProgramFxDX11();

  /* common methods */
  virtual void load();
  virtual void unload();

  virtual void activate( Ptr_t context );
  virtual void deactivate();

  virtual void setDevice( Ptr_t device );

  virtual void updateConstants();

  /* common uniform setters */
  virtual void setMatrix( const float *matrix );
  virtual void setTextureSampler0( Ptr_t sampler );

  /* specific methods */
  void setBoneMatrices( const float *matrixArray, unsigned count );

private:

  class Impl;
  Impl *_i;
};

#endif /* SUPPORT_RENDERER_DX11 */

#endif /* _RD_SHADER_PROGRAM_DX11_H_ */
