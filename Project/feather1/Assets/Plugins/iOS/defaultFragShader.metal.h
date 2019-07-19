#ifndef __DEFAULTFRAGSHADER_METAL_H__
#define __DEFAULTFRAGSHADER_METAL_H__

const char *g_defaultFragShader_metal =
"\n\
#include <metal_stdlib>\n\
using namespace metal;\n\
\n\
struct OutputVertex\n\
{\n\
  float4 v_position   [[position]];\n\
  half4 v_color;\n\
  float2 v_texCoord0;\n\
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
  OutputFragment o_fragment = { i_vertex.v_color * tex.sample(textureSampler, i_vertex.v_texCoord0) };\n\
  return o_fragment;\n\
};\n\
\n\
";

#endif /* __DEFAULTFRAGSHADER_METAL_H__ */
