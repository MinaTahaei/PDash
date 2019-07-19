#ifndef __DEFAULTVERTSHADER_ES2_GLSL_H__
#define __DEFAULTVERTSHADER_ES2_GLSL_H__

const char *g_defaultVertShader_es2_glsl =
"\n\
#ifdef GL_ES\n\
precision mediump float;\n\
#endif\n\
\n\
uniform mat4 u_mvpMatrix;\n\
\n\
attribute vec2 a_position;\n\
attribute vec2 a_texCoord;\n\
attribute vec4 a_color;\n\
\n\
varying vec2 v_texCoord;\n\
varying vec4 v_color;\n\
\n\
void main()\n\
{\n\
  gl_Position = u_mvpMatrix * vec4(a_position, 0, 1);\n\
  v_texCoord = a_texCoord;\n\
  v_color = a_color / 255.0;\n\
}\n\
";

#endif /* __DEFAULTVERTSHADER_ES2_GLSL_H__ */
