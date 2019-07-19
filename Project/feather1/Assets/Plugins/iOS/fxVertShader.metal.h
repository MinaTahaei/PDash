#ifndef __FXVERTSHADER_METAL_H__
#define __FXVERTSHADER_METAL_H__

const char *g_fxVertShader_metal =
"\n\
#include <metal_stdlib>\n\
using namespace metal;\n\
\n\
//  a_boneParams\n\
//  | boneIndex1      | boneWeight1     | boneIndex2      | boneWeight2     |\n\
//  |-----------------|-----------------|-----------------|-----------------|\n\
//  | (float) 32 bits | (float) 32 bits | (float) 32 bits | (float) 32 bits |\n\
\n\
struct InputVertex\n\
{\n\
  float3 a_position    [[attribute(0)]];\n\
  uchar4 a_color       [[attribute(1)]];\n\
  float2 a_texCoord0   [[attribute(2)]];\n\
\n\
  float4 a_fxParams0   [[attribute(3)]];\n\
  float4 a_fxViewport0 [[attribute(4)]];\n\
\n\
  float4 a_boneParams  [[attribute(5)]];\n\
};\n\
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
struct ConstantBuffer\n\
{\n\
  //  Model view projection matrix\n\
  float4x4 u_mvpMatrix;\n\
\n\
  //  Array of bone matrices.  Index 0 is identity matrix.\n\
  float4x4 u_boneMatrix[32];\n\
};\n\
\n\
vertex OutputVertex vertex_main( InputVertex i_vertex [[stage_in]],\n\
                                 constant ConstantBuffer& constants [[buffer(1)]] )\n\
{\n\
  float4x4 skinMatrix = i_vertex.a_boneParams.y * constants.u_boneMatrix[ static_cast<int>(i_vertex.a_boneParams.x) ] +\n\
                        i_vertex.a_boneParams.w * constants.u_boneMatrix[ static_cast<int>(i_vertex.a_boneParams.z) ];\n\
\n\
  OutputVertex o_vertex;\n\
\n\
  o_vertex.v_position = constants.u_mvpMatrix * skinMatrix * float4( i_vertex.a_position, 1 );\n\
  o_vertex.v_color = static_cast<half4>(i_vertex.a_color) / 255.0f;\n\
  o_vertex.v_texCoord0 = i_vertex.a_texCoord0;\n\
\n\
  o_vertex.v_fxParams0 = i_vertex.a_fxParams0;\n\
  o_vertex.v_fxViewport0 = i_vertex.a_fxViewport0;\n\
\n\
  return o_vertex;\n\
}\n\
\n\
";

#endif /* __FXVERTSHADER_METAL_H__ */
