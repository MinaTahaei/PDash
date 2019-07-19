#ifndef __DEFAULTFRAGSHADER_ES2_GLSL_H__
#define __DEFAULTFRAGSHADER_ES2_GLSL_H__

const char *g_defaultFragShader_es2_glsl =
"\n\
#ifdef GL_ES\n\
precision mediump float;\n\
#endif\n\
\n\
varying vec2 v_texCoord;\n\
varying vec4 v_color;\n\
\n\
uniform sampler2D u_texture0;\n\
\n\
void main()\n\
{\n\
  gl_FragColor = v_color * texture2D(u_texture0, v_texCoord);\n\
}\n\
";

#endif /* __DEFAULTFRAGSHADER_ES2_GLSL_H__ */
