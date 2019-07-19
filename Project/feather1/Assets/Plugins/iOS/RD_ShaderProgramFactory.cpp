#include "RD_ShaderProgramFactory.h"

#if defined(SUPPORT_RENDERER_GLES2) || defined(SUPPORT_RENDERER_GL)

#include "RD_ShaderProgramGL.h"
#include "RD_ShaderProgramGLES2.h"


RD_ShaderProgramGL* RD_ShaderProgramFactory::createShaderProgram(const RendererType rendererType, const ShaderType shaderType)
{
  if (rendererType == OPENGL_CORE)
  {
    if (shaderType == PLAIN)
      return new RD_ShaderProgramPlainGL();
    else if (shaderType == FX)
      return new RD_ShaderProgramFxGL();
  }
  else if (rendererType == OPENGL_ES2)
  {
    if (shaderType == PLAIN)
      return new RD_ShaderProgramPlainGLES2();
    else if (shaderType == FX)
      return new RD_ShaderProgramFxGLES2();
  }
}

#endif /* SUPPORT_RENDERER_GLES2 || SUPPORT_RENDERER_GL */
