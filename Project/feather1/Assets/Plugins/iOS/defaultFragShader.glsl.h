#ifndef __DEFAULTFRAGSHADER_GLSL_H__
#define __DEFAULTFRAGSHADER_GLSL_H__

const char *g_defaultFragShader_glsl =
"\n\
#version 150\n\
\n\
#ifdef GL_ES\n\
precision mediump float;\n\
#endif\n\
\n\
in vec2 v_texCoord;\n\
in vec4 v_color;\n\
\n\
uniform sampler2D u_texture0;\n\
\n\
out vec4 fragColor;\n\
\n\
void main()\n\
{\n\
 fragColor = v_color * texture(u_texture0, v_texCoord);\n\
}\n\
";

#endif /* __DEFAULTFRAGSHADER_GLSL_H__ */
