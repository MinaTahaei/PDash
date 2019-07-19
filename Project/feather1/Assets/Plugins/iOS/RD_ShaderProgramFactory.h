#ifndef _RD_SHADER_PROGRAM_FACTORY_H_
#define _RD_SHADER_PROGRAM_FACTORY_H_

#include "PL_Configure.h"

#if defined(SUPPORT_RENDERER_GLES2) || defined(SUPPORT_RENDERER_GL)

class RD_ShaderProgramGL;

class RD_ShaderProgramFactory
{
public:
  enum RendererType {
    OPENGL_CORE,
    OPENGL_ES2
  };

  enum ShaderType {
    PLAIN,
    FX
  };

  static RD_ShaderProgramGL* createShaderProgram(const RendererType, const ShaderType);
};

#endif /* SUPPORT_RENDERER_GLES2 || SUPPORT_RENDERER_GL */

#endif /* _RD_SHADER_PROGRAM_FACTORY_H_ */
