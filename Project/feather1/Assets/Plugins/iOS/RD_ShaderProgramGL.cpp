#include "RD_ShaderProgramGL.h"

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


#define  LOG_TAG    "LogGLES2"
#define  ERROR_TAG  "ErrorGLES2"

#ifdef TARGET_ANDROID
#include <android/log.h>

#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,ERROR_TAG,__VA_ARGS__)
#else
#define  LOGI(...)  fprintf(stderr, "[%s] ", LOG_TAG );   fprintf(stderr, __VA_ARGS__)
#define  LOGE(...)  fprintf(stderr, "[%s] ", ERROR_TAG ); fprintf(stderr, __VA_ARGS__)
#endif

#ifdef ENABLE_GL_DEBUG
  namespace
  {
    void CheckGLError(const char* file, int line)
    {
      GLenum e = glGetError();
      if( e )
        LOGE ( "OpenGL error 0x%04X in %s:%i\n", e, file, line);
    }
  }

  #define GLAssert()	do { CheckGLError (__FILE__, __LINE__); } while(0)
  #define GL_CHK(expr)	do { {expr;} GLAssert(); } while(0)
#else
  #define GLAssert()	do { } while(0)
  #define GL_CHK(expr)	do { expr; } while(0)
#endif

/*  Vertex/Fragment shaders */
#include "defaultVertShader.glsl.h"
#include "defaultFragShader.glsl.h"
#include "fxVertShader.glsl.h"
#include "fxFragShader.glsl.h"

#define MAX_BONES 32

namespace
{
  void logCompileError(GLuint shader)
  {
#ifdef ENABLE_GL_DEBUG
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if ( !status )
    {
      GLsizei length;
      glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &length);
      GLchar* src = (GLchar *)malloc(sizeof(GLchar) * length);

      glGetShaderSource(shader, length, NULL, src);
      LOGE( "Failed to compile shader:\n%s", src);

      free(src);

      GLint logLength = 0, charsWritten = 0;
      glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );
      if (logLength > 0)
      {
        GLchar *logBytes = (GLchar*)malloc(logLength);
        glGetShaderInfoLog( shader, logLength, &charsWritten, logBytes);

        LOGE( "Error:\n%s", logBytes);

        free(logBytes);
      }

      abort();
    }
#else /* ENABLE_GLES_DEBUG */
    (void)shader;
#endif
  }

  void logLinkError(GLuint program)
  {
#ifdef ENABLE_GL_DEBUG
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if ( !status )
    {
      GLint logLength;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
      if (logLength > 0)
      {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(program, logLength, &logLength, log);
        LOGE( "Program link log:\n%s", log);
        free(log);
      }

      abort();
    }
#else /* ENABLE_GLES_DEBUG */
    (void)program;
#endif
  }

  void compileShader(GLuint * shader, GLenum type, const char *source)
  {
    if (!source)
      return;

    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);

    logCompileError(*shader);
  }

  void linkProgram(GLuint program)
  {
    glLinkProgram(program);
    logLinkError(program);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramGL
#endif

RD_ShaderProgramGL::RD_ShaderProgramGL()
{
}

RD_ShaderProgramGL::~RD_ShaderProgramGL()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramPlainGLES2::Impl
#endif

class RD_ShaderProgramPlainGL::Impl
{
  friend class RD_ShaderProgramPlainGL;

public:

  Impl() :
    _program(0),
    _vertShader(0),
    _fragShader(0)
  {
  }

private:

  GLuint _program;
  GLuint _vertShader;
  GLuint _fragShader;

  GLint  _uniforms[OGL_NUM_UNIFORMS];

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramPlainGL
#endif

RD_ShaderProgramPlainGL::RD_ShaderProgramPlainGL()
{
  _i = new Impl;
}

RD_ShaderProgramPlainGL::~RD_ShaderProgramPlainGL()
{
  delete _i;
}

void RD_ShaderProgramPlainGL::load()
{
  //  Shaders were already loaded.
  if (_i->_program != 0)
    return;

  //  1. Create shader program.
  _i->_program = glCreateProgram();

  //  2. Compile vertex and fragment shader and attach to program.
  compileShader(&_i->_vertShader, GL_VERTEX_SHADER, vertexShader());
  compileShader(&_i->_fragShader, GL_FRAGMENT_SHADER, fragmentShader());

  if ( _i->_vertShader )
    glAttachShader( _i->_program, _i->_vertShader );

  if ( _i->_fragShader )
    glAttachShader( _i->_program, _i->_fragShader );

  //  3. Bind vertex attributes to program.
  glBindAttribLocation(_i->_program, OGL_VERTEX_ATTRIB_POSITION, "a_position");
  glBindAttribLocation(_i->_program, OGL_VERTEX_ATTRIB_COLOR, "a_color");
  glBindAttribLocation(_i->_program, OGL_VERTEX_ATTRIB_TEXCOORDS_0, "a_texCoord");

  //  4. Link program.
  linkProgram( _i->_program );

  if ( _i->_vertShader )
    glDeleteShader(_i->_vertShader);

  if ( _i->_fragShader )
    glDeleteShader(_i->_fragShader);

  _i->_vertShader = _i->_fragShader = 0;

  //  5. Bind uniform variables.
  _i->_uniforms[OGL_UNIFORM_MVP_MATRIX] = glGetUniformLocation(_i->_program, "u_mvpMatrix");
  _i->_uniforms[OGL_UNIFORM_TEXTURE_0] = glGetUniformLocation(_i->_program, "u_texture0");
}

void RD_ShaderProgramPlainGL::unload()
{
  glDeleteProgram(_i->_program);
  _i->_program = 0;
}


const char* RD_ShaderProgramPlainGL::vertexShader()
{
  return g_defaultVertShader_glsl;
}

const char* RD_ShaderProgramPlainGL::fragmentShader()
{
  return g_defaultFragShader_glsl;
}

void RD_ShaderProgramPlainGL::activate()
{
  load();
  glUseProgram(_i->_program);
}

void RD_ShaderProgramPlainGL::deactivate()
{
  glUseProgram(0);
}

void RD_ShaderProgramPlainGL::setMatrix(const float *matrix)
{
  glUniformMatrix4fv( _i->_uniforms[OGL_UNIFORM_MVP_MATRIX], 1, false, matrix );
}

void RD_ShaderProgramPlainGL::setTextureSampler0(int activeTexture)
{
  glUniform1i( _i->_uniforms[OGL_UNIFORM_TEXTURE_0], activeTexture );
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramFxGL::Impl
#endif

class RD_ShaderProgramFxGL::Impl
{
  friend class RD_ShaderProgramFxGL;

public:

  Impl() :
    _program(0),
    _vertShader(0),
    _fragShader(0)
  {
  }

private:

  GLuint _program;
  GLuint _vertShader;
  GLuint _fragShader;

  GLint  _uniforms[OGL_NUM_UNIFORMS];

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramFxGL
#endif

RD_ShaderProgramFxGL::RD_ShaderProgramFxGL()
{
  _i = new Impl;
}

RD_ShaderProgramFxGL::~RD_ShaderProgramFxGL()
{
  delete _i;
}

void RD_ShaderProgramFxGL::load()
{
  //  Shaders were already loaded.
  if (_i->_program != 0)
    return;

  //  1. Create shader program.
  _i->_program = glCreateProgram();

  //  2. Compile vertex and fragment shader and attach to program.
  compileShader(&_i->_vertShader, GL_VERTEX_SHADER, vertexShader());
  compileShader(&_i->_fragShader, GL_FRAGMENT_SHADER, fragmentShader());

  if ( _i->_vertShader )
    glAttachShader( _i->_program, _i->_vertShader );

  if ( _i->_fragShader )
    glAttachShader( _i->_program, _i->_fragShader );

  //  3. Bind vertex attributes to program.
  glBindAttribLocation(_i->_program, OGL_VERTEX_ATTRIB_POSITION, "a_position");
  glBindAttribLocation(_i->_program, OGL_VERTEX_ATTRIB_COLOR, "a_color");
  glBindAttribLocation(_i->_program, OGL_VERTEX_ATTRIB_TEXCOORDS_0, "a_texCoord0");
  glBindAttribLocation(_i->_program, OGL_VERTEX_ATTRIB_FX_PARAMS_0, "a_fxParams0");
  glBindAttribLocation(_i->_program, OGL_VERTEX_ATTRIB_FX_VIEWPORT_0, "a_fxViewport0");
  glBindAttribLocation(_i->_program, OGL_VERTEX_ATTRIB_BONE_INDEX, "a_boneIndex");

  //  4. Link program.
  linkProgram( _i->_program );

  if ( _i->_vertShader )
    glDeleteShader(_i->_vertShader);

  if ( _i->_fragShader )
    glDeleteShader(_i->_fragShader);

  _i->_vertShader = _i->_fragShader = 0;

  //  5. Bind uniform variables.
  _i->_uniforms[OGL_UNIFORM_MVP_MATRIX] = glGetUniformLocation(_i->_program, "u_mvpMatrix");
  _i->_uniforms[OGL_UNIFORM_TEXTURE_0] = glGetUniformLocation(_i->_program, "u_mainTexture");
  _i->_uniforms[OGL_UNIFORM_TEXTURE_1] = glGetUniformLocation(_i->_program, "u_maskTexture");

  char buffer[128];

  for ( unsigned i = 0 ; i < MAX_BONES ; ++i )
  {
    sprintf( buffer, "u_boneMatrix[%i]", i );
    _i->_uniforms[OGL_UNIFORM_BONE_MATRIX_0 + i] = glGetUniformLocation(_i->_program, buffer);
  }
}

void RD_ShaderProgramFxGL::unload()
{
  glDeleteProgram(_i->_program);
  _i->_program = 0;
}


const char* RD_ShaderProgramFxGL::vertexShader()
{
  return g_fxVertShader_glsl;
}


const char* RD_ShaderProgramFxGL::fragmentShader()
{
  return g_fxFragShader_glsl;
}

void RD_ShaderProgramFxGL::activate()
{
  load();
  glUseProgram(_i->_program);
}

void RD_ShaderProgramFxGL::deactivate()
{
  glUseProgram(0);
}

void RD_ShaderProgramFxGL::setMatrix(const float *matrix)
{
  glUniformMatrix4fv( _i->_uniforms[OGL_UNIFORM_MVP_MATRIX], 1, false, matrix );
}

void RD_ShaderProgramFxGL::setTextureSampler0(int texture)
{
  glUniform1i( _i->_uniforms[OGL_UNIFORM_TEXTURE_0], texture );
}

void RD_ShaderProgramFxGL::setTextureSampler1(int texture)
{
  glUniform1i( _i->_uniforms[OGL_UNIFORM_TEXTURE_1], texture );
}

void RD_ShaderProgramFxGL::setBoneMatrices(const float *matrixArray, unsigned count)
{
  glUniformMatrix4fv( _i->_uniforms[OGL_UNIFORM_BONE_MATRIX_0], count, false, matrixArray );
}

#endif /* SUPPORT_RENDERER_GLES2 || SUPPORT_RENDERER_GL */

