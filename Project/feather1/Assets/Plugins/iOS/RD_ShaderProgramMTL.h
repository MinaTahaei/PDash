
#ifndef _RD_SHADER_PROGRAM_MTL_H_
#define _RD_SHADER_PROGRAM_MTL_H_

#include "PL_Configure.h"

#if defined(SUPPORT_RENDERER_METAL)

/*!
 *  @class RD_ShaderProgramMTL
 *  Shader interface.
 */
class RD_ShaderProgramMTL
{
public:

  typedef void* Ptr_t;

public:

  RD_ShaderProgramMTL();
  virtual ~RD_ShaderProgramMTL();

  /* common methods */
  virtual void load() =0;
  virtual void unload() =0;

  virtual void activate( Ptr_t context, Ptr_t uniformBuffer ) =0;
  virtual void deactivate() =0;

  virtual void setDevice( Ptr_t device ) =0;

  /* common uniform setters */
  virtual void setMatrix( const float *matrix ) =0;
  virtual void setTextureSampler0( Ptr_t sampler ) =0;
};

/*!
 *  @class RD_ShaderProgramPlainMTL
 *  Plain shader implementation.
 */
class RD_ShaderProgramPlainMTL : public RD_ShaderProgramMTL
{
public:
  /*!
   *  @struct UniformBufferDesc
   *  Constant parameters in vertex shader.
   */
  struct UniformBufferDesc
  {
    float _mvpMatrix[16];
  };

public:

  RD_ShaderProgramPlainMTL( Ptr_t device );
  virtual ~RD_ShaderProgramPlainMTL();

  /* common methods */
  virtual void load();
  virtual void unload();

  virtual void activate( Ptr_t encoder, Ptr_t uniformBuffer );
  virtual void deactivate();

  virtual void setDevice( Ptr_t device );

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
 *  @class RD_ShaderProgramFxMTL
 *  Fx shader implementation.
 */
class RD_ShaderProgramFxMTL : public RD_ShaderProgramMTL
{
public:
  /*!
   *  @struct UniformBufferDesc
   *  Constant parameters in vertex shader.
   */
  struct UniformBufferDesc
  {
    float _mvpMatrix[16];
    float _boneMatrices[512]; // 32 * 16
  };

public:

  RD_ShaderProgramFxMTL( Ptr_t device );
  virtual ~RD_ShaderProgramFxMTL();

  /* common methods */
  virtual void load();
  virtual void unload();

  virtual void activate( Ptr_t encoder, Ptr_t uniformBuffer );
  virtual void deactivate();

  virtual void setDevice( Ptr_t device );

  /* common uniform setters */
  virtual void setMatrix( const float *matrix );
  virtual void setTextureSampler0( Ptr_t sampler );

  /* specific methods */
  void setBoneMatrices( const float *matrixArray, unsigned count );

private:

  class Impl;
  Impl *_i;
};

#endif /* SUPPORT_RENDERER_METAL */

#endif /* _RD_SHADER_PROGRAM_MTL_H_ */
