#ifndef __FXFRAGSHADER_ES2_GLSL_H__
#define __FXFRAGSHADER_ES2_GLSL_H__

const char *g_fxFragShader_es2_glsl =
"\n\
#ifdef GL_ES\n\
precision highp float;\n\
#endif\n\
\n\
varying vec4      v_color;\n\
varying vec2      v_texCoord0;\n\
\n\
//  v_fxParams ( cutter and inverse cutter )\n\
//  | u               | v               | add             | mult            |\n\
//  |-----------------|-----------------|-----------------|-----------------|\n\
//  | (float) 32 bits | (float) 32 bits | (float) 32 bits | (float) 32 bits |\n\
varying vec4      v_fxParams0;\n\
\n\
//  v_fxViewport\n\
//  | u0              | v0              | u1              | v1              |\n\
//  |-----------------|-----------------|-----------------|-----------------|\n\
//  | (float) 32 bits | (float) 32 bits | (float) 32 bits | (float) 32 bits |\n\
varying vec4      v_fxViewport0;\n\
\n\
uniform sampler2D u_mainTexture;\n\
uniform sampler2D u_maskTexture;\n\
\n\
void main()\n\
{\n\
  vec2 uv = clamp( v_fxParams0.xy, v_fxViewport0.xy, v_fxViewport0.zw );\n\
  float mult = (v_fxParams0.z + v_fxParams0.w * texture2D(u_maskTexture, uv ).a);\n\
\n\
  gl_FragColor = v_color * mult * texture2D(u_mainTexture, v_texCoord0);\n\
}\n\
";

#endif /* __FXFRAGSHADER_ES2_GLSL_H__ */
