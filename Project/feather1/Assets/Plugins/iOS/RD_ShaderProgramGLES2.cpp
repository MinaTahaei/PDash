#include "RD_ShaderProgramGLES2.h"

#if defined(SUPPORT_RENDERER_GLES2) || defined(SUPPORT_RENDERER_GL)

#include <stdio.h>
#include <stdlib.h>

#define ENABLE_GL_DEBUG

#if defined(SUPPORT_RENDERER_GLES2)
  #if defined(TARGET_IOS)
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
  #else //TARGET_ANDROID
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
  #endif
#elif defined(SUPPORT_RENDERER_GL)

  #if defined(TARGET_MAC)
    #include <OpenGL/gl.h>
  #else
    #define ENABLE_GLEW
    #include <GL/glew.h>
  #endif

#endif

/*  Vertex/Fragment shaders */
#include "defaultVertShader_es2.glsl.h"
#include "defaultFragShader_es2.glsl.h"
#include "fxVertShader_es2.glsl.h"
#include "fxFragShader_es2.glsl.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramPlainGLES2
#endif

RD_ShaderProgramPlainGLES2::RD_ShaderProgramPlainGLES2()
{
}


RD_ShaderProgramPlainGLES2::~RD_ShaderProgramPlainGLES2()
{
}

const char* RD_ShaderProgramPlainGLES2::vertexShader()
{
  return g_defaultVertShader_es2_glsl;
}

const char* RD_ShaderProgramPlainGLES2::fragmentShader()
{
  return g_defaultFragShader_es2_glsl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramFxGLES2
#endif


RD_ShaderProgramFxGLES2::RD_ShaderProgramFxGLES2()
{
}


RD_ShaderProgramFxGLES2::~RD_ShaderProgramFxGLES2()
{
}

const char* RD_ShaderProgramFxGLES2::vertexShader()
{
  return g_fxVertShader_es2_glsl;
}

const char* RD_ShaderProgramFxGLES2::fragmentShader()
{
  return g_fxFragShader_es2_glsl;
}

#endif /* SUPPORT_RENDERER_GLES2 || SUPPORT_RENDERER_GL */
