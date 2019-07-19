
#ifndef _RD_SHADER_PROGRAM_GLES2_H_
#define _RD_SHADER_PROGRAM_GLES2_H_

#include "PL_Configure.h"

#if defined(SUPPORT_RENDERER_GLES2) || defined(SUPPORT_RENDERER_GL)

#include "RD_ShaderProgramGL.h"

/*!
 *  @class RD_ShaderProgramPlainGLES2
 *  Plain shader implementation.
 */
class RD_ShaderProgramPlainGLES2 : public RD_ShaderProgramPlainGL
{
public:

  RD_ShaderProgramPlainGLES2();
  virtual ~RD_ShaderProgramPlainGLES2();

  virtual const char* vertexShader();
  virtual const char* fragmentShader();
};

/*!
 *  @class RD_ShaderProgramFxGLES2
 *  Fx shader implementation.
 */
class RD_ShaderProgramFxGLES2 : public RD_ShaderProgramFxGL
{
public:

  RD_ShaderProgramFxGLES2();
  virtual ~RD_ShaderProgramFxGLES2();

  virtual const char* vertexShader();
  virtual const char* fragmentShader();
};

#endif /* SUPPORT_RENDERER_GLES2 || SUPPORT_RENDERER_GL */

#endif /* _RD_SHADER_PROGRAM_GLES2_H_ */
