#ifndef __DEFAULTVERTSHADER_METAL_H__
#define __DEFAULTVERTSHADER_METAL_H__

const char *g_defaultVertShader_metal =
"\n\
#include <metal_stdlib>\n\
using namespace metal;\n\
\n\
struct InputVertex\n\
{\n\
  float3 a_position   [[attribute(0)]];\n\
  uchar4 a_color      [[attribute(1)]];\n\
  float2 a_texCoord0  [[attribute(2)]];\n\
};\n\
\n\
struct OutputVertex\n\
{\n\
  float4 v_position   [[position]];\n\
  half4 v_color;\n\
  float2 v_texCoord0;\n\
};\n\
\n\
vertex OutputVertex vertex_main( InputVertex i_vertex [[stage_in]],\n\
                                 constant float4x4& u_mvpMatrix [[buffer(1)]] )\n\
{\n\
  OutputVertex o_vertex;\n\
\n\
  o_vertex.v_position = u_mvpMatrix * float4( i_vertex.a_position, 1 );\n\
  o_vertex.v_color = static_cast<half4>(i_vertex.a_color) / 255.0f;\n\
  o_vertex.v_texCoord0 = i_vertex.a_texCoord0;\n\
\n\
  return o_vertex;\n\
}\n\
\n\
";

#endif /* __DEFAULTVERTSHADER_METAL_H__ */
