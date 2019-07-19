#include "RD_RendererGLES2.h"

#if defined(SUPPORT_RENDERER_GLES2) || defined(SUPPORT_RENDERER_GL)

#include "RD_ShaderProgramGL.h"
#include "RD_ShaderProgramFactory.h"


RD_RendererGLES2::RD_RendererGLES2()
{
}


RD_RendererGLES2::~RD_RendererGLES2()
{
}


bool RD_RendererGLES2::useVertexArrayObject() const
{
  return false;
}

RD_ShaderProgramGL* RD_RendererGLES2::createShaderProgramPlain() const
{
  return RD_ShaderProgramFactory::createShaderProgram(
    RD_ShaderProgramFactory::OPENGL_ES2,
    RD_ShaderProgramFactory::PLAIN);
}

RD_ShaderProgramGL* RD_RendererGLES2::createShaderProgramFx() const
{
  return RD_ShaderProgramFactory::createShaderProgram(
    RD_ShaderProgramFactory::OPENGL_ES2,
    RD_ShaderProgramFactory::FX);
}


#endif /* SUPPORT_RENDERER_GLES2 || SUPPORT_RENDERER_GL */

