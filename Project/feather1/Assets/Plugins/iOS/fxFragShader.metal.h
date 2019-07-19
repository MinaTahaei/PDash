#ifndef __FXFRAGSHADER_METAL_H__
#define __FXFRAGSHADER_METAL_H__

const char *g_fxFragShader_metal =
"\n\
#include <metal_stdlib>\n\
using namespace metal;\n\
\n\
//  v_fxParams ( cutter and inverse cutter )\n\
//  | u               | v               | add             | mult            |\n\
//  |-----------------|-----------------|-----------------|-----------------|\n\
//  | (float) 32 bits | (float) 32 bits | (float) 32 bits | (float) 32 bits |\n\
\n\
//  v_fxViewport\n\
//  | u0              | v0              | u1              | v1              |\n\
//  |-----------------|-----------------|-----------------|-----------------|\n\
//  | (float) 32 bits | (float) 32 bits | (float) 32 bits | (float) 32 bits |\n\
\n\
struct OutputVertex\n\
{\n\
  float4 v_position   [[position]];\n\
  half4  v_color;\n\
  float2 v_texCoord0;\n\
\n\
  float4 v_fxParams0;\n\
  float4 v_fxViewport0;\n\
};\n\
\n\
struct OutputFragment\n\
{\n\
  half4 f_color      [[color(0)]];\n\
};\n\
\n\
constexpr sampler textureSampler (address::clamp_to_edge, filter::linear);\n\
fragment OutputFragment fragment_main( OutputVertex i_vertex [[stage_in]],\n\
                                       texture2d<half> tex [[texture(0)]] )\n\
{\n\
  //  Clamp uv coordinates once they are outside of matte viewport.\n\
  //  This takes into account that last pixel bands are entirely\n\
  //  transparent.\n\
  float2 uv = clamp( i_vertex.v_fxParams0.xy, i_vertex.v_fxViewport0.xy, i_vertex.v_fxViewport0.zw );\n\
  float mult = (i_vertex.v_fxParams0.z + i_vertex.v_fxParams0.w * tex.sample(textureSampler, uv ).a);\n\
\n\
  OutputFragment o_fragment = { i_vertex.v_color * mult * tex.sample(textureSampler, i_vertex.v_texCoord0) };\n\
  return o_fragment;\n\
};\n\
\n\
";

#endif /* __FXFRAGSHADER_METAL_H__ */
