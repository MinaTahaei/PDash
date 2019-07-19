
#include "RD_ShaderProgramMTL.h"

#if defined(SUPPORT_RENDERER_METAL)

#include "RD_RenderScriptPlain.h"
#include "RD_RenderScriptFx.h"
#include "RD_InterfaceMTL.h"

/*  Vertex/Fragment shaders */
#include "defaultVertShader.metal.h"
#include "defaultFragShader.metal.h"
#include "fxVertShader.metal.h"
#include "fxFragShader.metal.h"

#define ENABLE_MTL_DEBUG

#ifndef MTL_PREAMBLE
#define MTL_PREAMBLE if( !_i->_device ) abort();
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramMTL
#endif

RD_ShaderProgramMTL::RD_ShaderProgramMTL()
{
}

RD_ShaderProgramMTL::~RD_ShaderProgramMTL()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramPlainMTL::Impl
#endif

class RD_ShaderProgramPlainMTL::Impl
{
  friend class RD_ShaderProgramPlainMTL;

public:


public:

  Impl() :
    _device(nil),
    _encoder(nil),
    _uniformBuffer(nil),
    _pipelineState(nil),
    _loaded(false)
  {
  }

private:

  id <MTLDevice>               _device;
  id <MTLRenderCommandEncoder> _encoder;
  id <MTLBuffer>               _uniformBuffer;

  id <MTLFunction>             _vertexFunction;
  id <MTLFunction>             _fragmentFunction;
  id <MTLRenderPipelineState>  _pipelineState;

  bool                         _loaded;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramPlainMTL
#endif

RD_ShaderProgramPlainMTL::RD_ShaderProgramPlainMTL( Ptr_t device )
{
  _i = new Impl;
  _i->_device = (__bridge id<MTLDevice>)(device);
}

RD_ShaderProgramPlainMTL::~RD_ShaderProgramPlainMTL()
{
  delete _i;
}

void RD_ShaderProgramPlainMTL::load()
{
  MTL_PREAMBLE;

  if ( _i->_loaded )
    return;

  NSError *error = nil;
  id<MTLLibrary> vertexLib = [_i->_device newLibraryWithSource: [NSString stringWithFormat:@"%s", g_defaultVertShader_metal] options: nil error: &error];

#ifdef ENABLE_MTL_DEBUG
  if (error != nil)
  {
    NSLog( @"Failed to compile vertex shader.\n" );
    NSLog( @"%@", [error localizedDescription] );
  }
#endif

  error = nil;
  id<MTLLibrary> fragmentLib = [_i->_device newLibraryWithSource: [NSString stringWithFormat:@"%s", g_defaultFragShader_metal] options: nil error: &error];

#ifdef ENABLE_MTL_DEBUG
  if (error != nil)
  {
    NSLog( @"Failed to compile fragment shader.\n" );
    NSLog( @"%@", [error localizedDescription] );
  }
#endif

  _i->_vertexFunction = [vertexLib newFunctionWithName: @"vertex_main"];
  _i->_fragmentFunction = [fragmentLib newFunctionWithName: @"fragment_main"];

  MTLVertexAttributeDescriptor* positionAttrDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLVertexAttributeDescriptor"] alloc] init];
  positionAttrDesc.format         = MTLVertexFormatFloat3;
  positionAttrDesc.offset         = offsetof( RD_RenderScriptPlain::VertexData, _x );
  positionAttrDesc.bufferIndex    = 0;

  MTLVertexAttributeDescriptor* colorAttrDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLVertexAttributeDescriptor"] alloc] init];
  colorAttrDesc.format            = MTLVertexFormatUChar4;
  colorAttrDesc.offset            = offsetof(RD_RenderScriptPlain::VertexData, _color);
  colorAttrDesc.bufferIndex       = 0;

  MTLVertexAttributeDescriptor* texCoordAttrDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLVertexAttributeDescriptor"] alloc] init];
  texCoordAttrDesc.format         = MTLVertexFormatFloat2;
  texCoordAttrDesc.offset         = offsetof(RD_RenderScriptPlain::VertexData, _u);
  texCoordAttrDesc.bufferIndex    = 0;

  MTLVertexBufferLayoutDescriptor* streamDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLVertexBufferLayoutDescriptor"] alloc] init];
  streamDesc.stride = sizeof(RD_RenderScriptPlain::VertexData);
  streamDesc.stepFunction = MTLVertexStepFunctionPerVertex;
  streamDesc.stepRate = 1;

  MTLVertexDescriptor *vertexDesc = [[HarmonyGetMetalBundle() classNamed: @"MTLVertexDescriptor"] vertexDescriptor];
  vertexDesc.attributes[0] = positionAttrDesc;
  vertexDesc.attributes[1] = colorAttrDesc;
  vertexDesc.attributes[2] = texCoordAttrDesc;
  vertexDesc.layouts[0] = streamDesc;

  MTLRenderPipelineDescriptor *pipelineDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLRenderPipelineDescriptor"] alloc] init];
  MTLRenderPipelineColorAttachmentDescriptor *renderbufferAttachment = pipelineDesc.colorAttachments[0];

  pipelineDesc.label                                 = @"HarmonyPipeline";
  pipelineDesc.depthAttachmentPixelFormat            = MTLPixelFormatDepth32Float;
  pipelineDesc.stencilAttachmentPixelFormat          = MTLPixelFormatStencil8;
  pipelineDesc.sampleCount                           = 1;
  pipelineDesc.vertexFunction                        = _i->_vertexFunction;
  pipelineDesc.fragmentFunction                      = _i->_fragmentFunction;
  pipelineDesc.vertexDescriptor                      = vertexDesc;

  renderbufferAttachment.pixelFormat                 = MTLPixelFormatBGRA8Unorm;

  renderbufferAttachment.blendingEnabled             = YES;
  renderbufferAttachment.rgbBlendOperation           = MTLBlendOperationAdd;
  renderbufferAttachment.alphaBlendOperation         = MTLBlendOperationAdd;
  renderbufferAttachment.sourceRGBBlendFactor        = MTLBlendFactorOne;
  renderbufferAttachment.sourceAlphaBlendFactor      = MTLBlendFactorOne;
  renderbufferAttachment.destinationRGBBlendFactor   = MTLBlendFactorOneMinusSourceAlpha;
  renderbufferAttachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

  _i->_pipelineState = [_i->_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];

#ifdef ENABLE_MTL_DEBUG
  if (error != nil)
  {
    NSLog( @"Failed to create render pipeline state.\n" );
    NSLog( @"%@", [error localizedDescription] );
  }
#endif

  _i->_loaded = true;
}

void RD_ShaderProgramPlainMTL::unload()
{
#if !__has_feature(objc_arc)
  [_i->_vertexFunction release];
  [_i->_fragmentFunction release];
  [_i->_pipelineState release];
#endif

  _i->_vertexFunction = nil;
  _i->_fragmentFunction = nil;
  _i->_pipelineState = nil;

  _i->_loaded = false;
}

void RD_ShaderProgramPlainMTL::activate( Ptr_t encoder, Ptr_t uniformBuffer )
{
  MTL_PREAMBLE;

  load();

  _i->_encoder = (__bridge id<MTLRenderCommandEncoder>)(encoder);
  [_i->_encoder setRenderPipelineState: _i->_pipelineState];

  _i->_uniformBuffer = (__bridge id<MTLBuffer>)(uniformBuffer);
}

void RD_ShaderProgramPlainMTL::deactivate()
{
  MTL_PREAMBLE;

  if ( _i->_encoder == nil )
    return;

  [_i->_encoder setRenderPipelineState: nil];
  _i->_encoder = nil;

  _i->_uniformBuffer = nil;
}

void RD_ShaderProgramPlainMTL::setDevice( Ptr_t device )
{
  _i->_device = (__bridge id<MTLDevice> )(device);
}

void RD_ShaderProgramPlainMTL::setMatrix( const float *matrix )
{
  if ( _i->_uniformBuffer == nil )
    return;

  UniformBufferDesc *buffer = static_cast<UniformBufferDesc*>([_i->_uniformBuffer contents]);
  memcpy( &buffer->_mvpMatrix[0], matrix, sizeof(float)*16 );
}

void RD_ShaderProgramPlainMTL::setTextureSampler0( Ptr_t sampler )
{
  if ( _i->_encoder == nil )
    return;

  id <MTLTexture> texture = (__bridge id<MTLTexture> )( sampler );
  [_i->_encoder setFragmentTexture: texture atIndex: 0];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramFxMTL::Impl
#endif

class RD_ShaderProgramFxMTL::Impl
{
  friend class RD_ShaderProgramFxMTL;

public:

  Impl() :
    _device(nil),
    _encoder(nil),
    _uniformBuffer(nil),
    _pipelineState(nil),
    _loaded(false)
  {
  }

private:

  id <MTLDevice>               _device;
  id <MTLRenderCommandEncoder> _encoder;
  id <MTLBuffer>               _uniformBuffer;

  id <MTLFunction>             _vertexFunction;
  id <MTLFunction>             _fragmentFunction;
  id <MTLRenderPipelineState>  _pipelineState;

  bool                         _loaded;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramFxMTL
#endif

RD_ShaderProgramFxMTL::RD_ShaderProgramFxMTL( Ptr_t device )
{
  _i = new Impl;
  _i->_device = (__bridge id<MTLDevice> )(device);
}

RD_ShaderProgramFxMTL::~RD_ShaderProgramFxMTL()
{
  delete _i;
}

void RD_ShaderProgramFxMTL::load()
{
  MTL_PREAMBLE;

  if ( _i->_loaded )
    return;

  NSError *error = nil;
  id<MTLLibrary> vertexLib = [_i->_device newLibraryWithSource: [NSString stringWithFormat:@"%s", g_fxVertShader_metal] options: nil error: &error];

#ifdef ENABLE_MTL_DEBUG
  if (error != nil)
  {
    NSLog( @"Failed to compile vertex shader.\n" );
    NSLog( @"%@", [error localizedDescription] );
  }
#endif

  error = nil;
  id<MTLLibrary> fragmentLib = [_i->_device newLibraryWithSource: [NSString stringWithFormat:@"%s", g_fxFragShader_metal] options: nil error: &error];

#ifdef ENABLE_MTL_DEBUG
  if (error != nil)
  {
    NSLog( @"Failed to compile fragment shader.\n" );
    NSLog( @"%@", [error localizedDescription] );
  }
#endif

  _i->_vertexFunction = [vertexLib newFunctionWithName: @"vertex_main"];
  _i->_fragmentFunction = [fragmentLib newFunctionWithName: @"fragment_main"];

  MTLVertexAttributeDescriptor* positionAttrDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLVertexAttributeDescriptor"] alloc] init];
  positionAttrDesc.format         = MTLVertexFormatFloat3;
  positionAttrDesc.offset         = offsetof( RD_RenderScriptFx::VertexData, _x );
  positionAttrDesc.bufferIndex    = 0;

  MTLVertexAttributeDescriptor* colorAttrDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLVertexAttributeDescriptor"] alloc] init];
  colorAttrDesc.format            = MTLVertexFormatUChar4;
  colorAttrDesc.offset            = offsetof(RD_RenderScriptFx::VertexData, _color);
  colorAttrDesc.bufferIndex       = 0;

  MTLVertexAttributeDescriptor* texCoordAttrDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLVertexAttributeDescriptor"] alloc] init];
  texCoordAttrDesc.format         = MTLVertexFormatFloat2;
  texCoordAttrDesc.offset         = offsetof(RD_RenderScriptFx::VertexData, _u0);
  texCoordAttrDesc.bufferIndex    = 0;

  MTLVertexAttributeDescriptor* fxParamsAttrDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLVertexAttributeDescriptor"] alloc] init];
  fxParamsAttrDesc.format         = MTLVertexFormatFloat4;
  fxParamsAttrDesc.offset         = offsetof(RD_RenderScriptFx::VertexData, _fxParams0[0]);
  fxParamsAttrDesc.bufferIndex    = 0;

  MTLVertexAttributeDescriptor* fxViewportAttrDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLVertexAttributeDescriptor"] alloc] init];
  fxViewportAttrDesc.format       = MTLVertexFormatFloat4;
  fxViewportAttrDesc.offset       = offsetof(RD_RenderScriptFx::VertexData, _fxViewport0[0]);
  fxViewportAttrDesc.bufferIndex  = 0;

  MTLVertexAttributeDescriptor* boneParamsAttrDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLVertexAttributeDescriptor"] alloc] init];
  boneParamsAttrDesc.format       = MTLVertexFormatFloat4;
  boneParamsAttrDesc.offset       = offsetof(RD_RenderScriptFx::VertexData, _boneParams[0]);
  boneParamsAttrDesc.bufferIndex  = 0;

  MTLVertexBufferLayoutDescriptor* streamDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLVertexBufferLayoutDescriptor"] alloc] init];
  streamDesc.stride = sizeof(RD_RenderScriptFx::VertexData);
  streamDesc.stepFunction = MTLVertexStepFunctionPerVertex;
  streamDesc.stepRate = 1;

  MTLVertexDescriptor *vertexDesc = [[HarmonyGetMetalBundle() classNamed: @"MTLVertexDescriptor"] vertexDescriptor];
  vertexDesc.attributes[0] = positionAttrDesc;
  vertexDesc.attributes[1] = colorAttrDesc;
  vertexDesc.attributes[2] = texCoordAttrDesc;
  vertexDesc.attributes[3] = fxParamsAttrDesc;
  vertexDesc.attributes[4] = fxViewportAttrDesc;
  vertexDesc.attributes[5] = boneParamsAttrDesc;
  vertexDesc.layouts[0] = streamDesc;

  MTLRenderPipelineDescriptor *pipelineDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLRenderPipelineDescriptor"] alloc] init];
  MTLRenderPipelineColorAttachmentDescriptor *renderbufferAttachment = pipelineDesc.colorAttachments[0];

  pipelineDesc.label                                 = @"HarmonyPipeline";
  pipelineDesc.depthAttachmentPixelFormat            = MTLPixelFormatDepth32Float;
  pipelineDesc.stencilAttachmentPixelFormat          = MTLPixelFormatStencil8;
  pipelineDesc.sampleCount                           = 1;
  pipelineDesc.vertexFunction                        = _i->_vertexFunction;
  pipelineDesc.fragmentFunction                      = _i->_fragmentFunction;
  pipelineDesc.vertexDescriptor                      = vertexDesc;

  renderbufferAttachment.pixelFormat                 = MTLPixelFormatBGRA8Unorm;

  renderbufferAttachment.blendingEnabled             = YES;
  renderbufferAttachment.rgbBlendOperation           = MTLBlendOperationAdd;
  renderbufferAttachment.alphaBlendOperation         = MTLBlendOperationAdd;
  renderbufferAttachment.sourceRGBBlendFactor        = MTLBlendFactorOne;
  renderbufferAttachment.sourceAlphaBlendFactor      = MTLBlendFactorOne;
  renderbufferAttachment.destinationRGBBlendFactor   = MTLBlendFactorOneMinusSourceAlpha;
  renderbufferAttachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

  _i->_pipelineState = [_i->_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];

#ifdef ENABLE_MTL_DEBUG
  if (error != nil)
  {
    NSLog( @"Failed to create render pipeline state.\n" );
    NSLog( @"%@", [error localizedDescription] );
  }
#endif

  _i->_loaded = true;
}

void RD_ShaderProgramFxMTL::unload()
{
#if !__has_feature(objc_arc)
  [_i->_vertexFunction release];
  [_i->_fragmentFunction release];
  [_i->_pipelineState release];
#endif

  _i->_vertexFunction = nil;
  _i->_fragmentFunction = nil;
  _i->_pipelineState = nil;

  _i->_loaded = false;
}

void RD_ShaderProgramFxMTL::activate( Ptr_t encoder, Ptr_t uniformBuffer )
{
  MTL_PREAMBLE;

  load();

  _i->_encoder = (__bridge id<MTLRenderCommandEncoder> )(encoder);
  [_i->_encoder setRenderPipelineState: _i->_pipelineState];

  _i->_uniformBuffer = (__bridge id<MTLBuffer>)(uniformBuffer);
}

void RD_ShaderProgramFxMTL::deactivate()
{
  MTL_PREAMBLE;

  if ( _i->_encoder == nil )
    return;

  [_i->_encoder setRenderPipelineState: nil];
  _i->_encoder = nil;

  _i->_uniformBuffer = nil;
}

void RD_ShaderProgramFxMTL::setDevice( Ptr_t device )
{
  _i->_device = (__bridge id<MTLDevice> )(device);
}

void RD_ShaderProgramFxMTL::setMatrix( const float *matrix )
{
  if ( _i->_uniformBuffer == nil )
    return;

  UniformBufferDesc *buffer = static_cast<UniformBufferDesc*>([_i->_uniformBuffer contents]);
  memcpy( &buffer->_mvpMatrix[0], matrix, sizeof(float)*16 );
}

void RD_ShaderProgramFxMTL::setTextureSampler0( Ptr_t sampler )
{
  if ( _i->_encoder == nil )
    return;

  id <MTLTexture> texture = (__bridge id<MTLTexture> )( sampler );
  [_i->_encoder setFragmentTexture: texture atIndex: 0];
}

void RD_ShaderProgramFxMTL::setBoneMatrices( const float *matrixArray, unsigned count )
{
  if ( _i->_uniformBuffer == nil )
    return;

  UniformBufferDesc *buffer = static_cast<UniformBufferDesc*>([_i->_uniformBuffer contents]);
  memcpy( &buffer->_boneMatrices[0], matrixArray, sizeof(float)*16*count );
}

#endif /* SUPPORT_RENDERER_MTL */

