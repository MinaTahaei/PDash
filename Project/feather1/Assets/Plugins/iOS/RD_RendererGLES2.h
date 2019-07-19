
#ifndef _RD_RENDERER_GL_ES2_H_
#define _RD_RENDERER_GL_ES2_H_

#include "RD_Renderer.h"
#include "PL_Configure.h"

#if defined(SUPPORT_RENDERER_GLES2) || defined(SUPPORT_RENDERER_GL)

#include "RD_RendererGLCore.h"

class RD_ShaderProgramGL;

/*!
 *  @class RD_RendererGLES2
 *  OpenGLES2 specific renderer.
 */
class RD_RendererGLES2 : public RD_RendererGLCore
{
public:

  RD_RendererGLES2();
  virtual ~RD_RendererGLES2();

  virtual bool useVertexArrayObject() const;

  virtual RD_ShaderProgramGL* createShaderProgramPlain() const;
  virtual RD_ShaderProgramGL* createShaderProgramFx() const;
};

#endif /* SUPPORT_RENDERER_GLES2 || SUPPORT_RENDERER_GL */

#endif /* _RD_RENDERER_GL_ES2_H_ */
