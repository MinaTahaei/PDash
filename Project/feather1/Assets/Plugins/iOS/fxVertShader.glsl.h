#ifndef __FXVERTSHADER_GLSL_H__
#define __FXVERTSHADER_GLSL_H__

const char *g_fxVertShader_glsl =
"\n\
#version 150\n\
\n\
#ifdef GL_ES\n\
precision highp float;\n\
#endif\n\
\n\
//  Model view projection matrix\n\
uniform mat4 u_mvpMatrix;\n\
\n\
//  Array of bone matrices.  Index 0 is identity matrix.\n\
uniform mat4 u_boneMatrix[32];\n\
\n\
//  Attributes\n\
in vec2  a_position;\n\
in vec4  a_color;\n\
\n\
in vec2  a_texCoord0;\n\
\n\
in vec4  a_fxParams0;\n\
in vec4  a_fxViewport0;\n\
\n\
//  a_boneParams\n\
//  | boneIndex1      | boneWeight1     | boneIndex2      | boneWeight2     |\n\
//  |-----------------|-----------------|-----------------|-----------------|\n\
//  | (float) 32 bits | (float) 32 bits | (float) 32 bits | (float) 32 bits |\n\
in vec4  a_boneParams;\n\
\n\
//  Varying parameters\n\
out vec4    v_color;\n\
\n\
out vec2    v_texCoord0;\n\
\n\
out vec4    v_fxParams0;\n\
out vec4    v_fxViewport0;\n\
\n\
void main()\n\
{\n\
  //  Compute skinning matrix\n\
  mat4 skinMatrix = a_boneParams.y * u_boneMatrix[ int(a_boneParams.x) ] +\n\
                    a_boneParams.w * u_boneMatrix[ int(a_boneParams.z) ];\n\
\n\
  gl_Position = u_mvpMatrix * skinMatrix * vec4(a_position, 0, 1);\n\
\n\
  //  Setup varying variables.\n\
  v_color         = a_color / 255.0;\n\
\n\
  v_texCoord0     = a_texCoord0;\n\
\n\
  v_fxParams0     = a_fxParams0;\n\
  v_fxViewport0   = a_fxViewport0;\n\
}\n\
";

#endif /* __FXVERTSHADER_GLSL_H__ */
