
#include "RD_RendererMTL.h"

#ifdef SUPPORT_RENDERER_METAL

#include "RD_InterfaceMTL.h"
#include "RD_ShaderProgramMTL.h"

#include "STD_Containers.h"

#include <stdio.h>
#include <string.h>

#ifndef MTL_PREAMBLE
#define MTL_PREAMBLE if( !_i->_device ) return;
#endif

#ifndef MTL_CURRENT_ENCODER
#define MTL_CURRENT_ENCODER() (( _i->_encoders.empty() ) ? nil : _i->_encoders.back())
#endif

#ifndef MTL_PUSH_ENCODER
#define MTL_PUSH_ENCODER(encoder) (_i->_encoders.push_back(encoder))
#endif

#ifndef MTL_POP_ENCODER
#define MTL_POP_ENCODER() (_i->_encoders.pop_back())
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RendererMTL::Impl
#endif

class RD_RendererMTL::Impl
{
  friend class RD_RendererMTL;

public:
  Impl() :
    _device(nil),
    _depthState(nil),
    _activeIndexBuffer(nil),
    _activeProgram(NULL)
  {
  }

  ~Impl()
  {
  }

private:

  id <MTLDevice>                _device;

  id <MTLDepthStencilState>     _depthState;

  id <MTLBuffer>                _activeIndexBuffer;

  typedef STD_Vector< id<MTLRenderCommandEncoder> > EncoderStack_t;
  EncoderStack_t                _encoders;

  struct TextureData
  {
    TextureData() :
      _textureBuffer(nil),
      _texture(nil)
    {
    }

    id <MTLBuffer>              _textureBuffer;
    id <MTLTexture>             _texture;
  };

  typedef STD_Map< RD_SpriteSheetWeakPtr_t, TextureData > TextureCol_t;
  TextureCol_t _textures;

  struct BufferData
  {
    BufferData() :
      _vertexBuffer(nil),
      _indexBuffer(nil)
    {
    }

    id <MTLBuffer>              _vertexBuffer;
    id <MTLBuffer>              _indexBuffer;

    id <MTLBuffer>              _uniformBuffer;
  };

  typedef STD_Map< RD_RenderScriptWeakPtr_t, BufferData > BufferCol_t;
  BufferCol_t _buffers;

  enum ShaderMode_t
  {
    eNullShader,
    ePlainShader,
    eFxShader
  };

  typedef STD_Map< ShaderMode_t, RD_ShaderProgramMTL* > ShaderProgramCol_t;
  ShaderProgramCol_t _shaders;

  RD_ShaderProgramMTL *_activeProgram;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RendererMTL
#endif

RD_RendererMTL::RD_RendererMTL()
{
  _i = new Impl;
}

RD_RendererMTL::~RD_RendererMTL()
{
  delete _i;
}

void RD_RendererMTL::initialize()
{
  //  Nothing to do.
}

void RD_RendererMTL::shutdown()
{
#if !__has_feature(objc_arc)
  [_i->_depthState release];
#endif

  _i->_depthState = nil;
}

void RD_RendererMTL::setDevice( void *device, int /*eventType*/ )
{
  _i->_device = (__bridge id<MTLDevice> )(device);
  //NSLog( @"device name: %@", _i->_device.name );

  //  Update device in shaders as well.
  for ( Impl::ShaderProgramCol_t::iterator i = _i->_shaders.begin(), iEnd = _i->_shaders.end() ; i!=iEnd ; ++i )
  {
    i->second->setDevice( (__bridge void*)_i->_device );
  }

  //  Initialize States
  MTLDepthStencilDescriptor *depthStateDesc = [[[HarmonyGetMetalBundle() classNamed: @"MTLDepthStencilDescriptor"] alloc] init];
  depthStateDesc.depthCompareFunction = MTLCompareFunctionLessEqual;
  depthStateDesc.depthWriteEnabled = NO;

  _i->_depthState = [_i->_device newDepthStencilStateWithDescriptor:depthStateDesc];
}

void RD_RendererMTL::cleanup()
{
  //  Clear unused buffers.
  for ( Impl::BufferCol_t::iterator i = _i->_buffers.begin(), iEnd = _i->_buffers.end() ; i!=iEnd ; )
  {
    if ( !i->first.isValid() )
    {
#if !__has_feature(objc_arc)
      [i->second._vertexBuffer release];
      [i->second._indexBuffer release];
      [i->second._uniformBuffer release];
#endif

      _i->_buffers.erase( i++ );
    }
    else
    {
      ++i;
    }
  }

  bool deleteProgram = _i->_buffers.empty();

  //  Clear unused textures.
  for ( Impl::TextureCol_t::iterator i = _i->_textures.begin(), iEnd = _i->_textures.end() ; i!=iEnd ; )
  {
    if ( !i->first.isValid() )
    {
#if !__has_feature(objc_arc)
      [i->second._textureBuffer release];
      [i->second._texture release];
#endif

      _i->_textures.erase( i++ );
    }
    else
    {
      ++i;
    }
  }

  deleteProgram = deleteProgram && _i->_textures.empty();

  //  If there is no more data mapped in renderer, free shaders as well.
  if ( deleteProgram )
  {
    for ( Impl::ShaderProgramCol_t::iterator i = _i->_shaders.begin(), iEnd = _i->_shaders.end() ; i!=iEnd ; )
    {
      i->second->unload();
      delete i->second;

      _i->_shaders.erase(i++);
    }
  }

}

void RD_RendererMTL::beginRender()
{
  id <MTLRenderCommandEncoder> encoder = static_cast< id<MTLRenderCommandEncoder> >(HarmonyCurrentMTLCommandEncoder());
  MTL_PUSH_ENCODER(encoder);

  if (encoder != nil)
  {
    //  Render States.
    [encoder setCullMode: MTLCullModeNone];
    [encoder setDepthStencilState: _i->_depthState];
  }

  //  Clear unused data.
  cleanup();

  //  None of our shaders are active at this point.
  _i->_activeProgram = 0;
}

void RD_RendererMTL::endRender()
{
  MTL_POP_ENCODER();
}

void RD_RendererMTL::setMatrices( const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix )
{
  if ( _i->_activeProgram == 0 )
    return;

  Math::Matrix4x4 mvpMatrix = projectionMatrix;

  //  Shift near plane.  DirectX maps clip plane to [0, 1], while OpenGL maps it to [-1,1].
  mvpMatrix(Math::Row_2, Math::Column_0) =
    (mvpMatrix(Math::Row_2, Math::Column_0) + mvpMatrix(Math::Row_3, Math::Column_0)) * 0.5;
  mvpMatrix(Math::Row_2, Math::Column_1) =
    (mvpMatrix(Math::Row_2, Math::Column_1) + mvpMatrix(Math::Row_3, Math::Column_1)) * 0.5;
  mvpMatrix(Math::Row_2, Math::Column_2) =
    (mvpMatrix(Math::Row_2, Math::Column_2) + mvpMatrix(Math::Row_3, Math::Column_2)) * 0.5;
  mvpMatrix(Math::Row_2, Math::Column_3) =
    (mvpMatrix(Math::Row_2, Math::Column_3) + mvpMatrix(Math::Row_3, Math::Column_3)) * 0.5;

  mvpMatrix = mvpMatrix * modelViewMatrix;

  float array[16];
  mvpMatrix.getValues(array);

  _i->_activeProgram->setMatrix( array );
}

void RD_RendererMTL::renderScriptToTexture( const RD_RenderScriptPtr_t &pRenderScript, const Rect &rect, float scale, TexturePtr_t texture )
{
  if (texture == NULL)
    return;

  id <MTLTexture> mtlTexture = (__bridge id<MTLTexture> )( texture );
  id <MTLCommandQueue> commandQueue = HarmonyGetMetalCommandQueue();
  id <MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];

  MTLRenderPassDescriptor *renderPassDescriptor = [[HarmonyGetMetalBundle() classNamed: @"MTLRenderPassDescriptor"] renderPassDescriptor];
  MTLRenderPassColorAttachmentDescriptor *colorAttachment = renderPassDescriptor.colorAttachments[0];
  colorAttachment.texture = mtlTexture;
  colorAttachment.loadAction = MTLLoadActionClear;
  colorAttachment.clearColor = MTLClearColorMake(0.0f, 0.0f, 0.0f, 0.0f);
  colorAttachment.storeAction = MTLStoreActionStore;

  //  Replace current view encoder with offline texture.
  id <MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
  MTL_PUSH_ENCODER(encoder);

  MTLViewport viewport;
  viewport.originX = 0.0;
  viewport.originY = 0.0;
  viewport.width = [mtlTexture width];
  viewport.height = [mtlTexture height];
  viewport.znear = 0.0;
  viewport.zfar = 1.0;

  [encoder setViewport:viewport];

  Math::Matrix4x4 projMatrix;
  projMatrix.orthogonalProject( rect._offsetx, rect._offsetx + rect._width, rect._offsety, rect._offsety + rect._height, -1000, 1000 );

  Math::Matrix4x4 modelMatrix;
  modelMatrix.scale( scale, scale );

  //  Render script as usual.
  renderScript( pRenderScript, projMatrix, modelMatrix );

  [encoder endEncoding];
  [commandBuffer commit];

  //  Restore current view encoder.
  MTL_POP_ENCODER();

#if !__has_feature(objc_arc)
  //[encoder release];
#endif
}

void RD_RendererMTL::renderScript( const RD_RenderScriptPtr_t &pRenderScript, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix )
{
  pRenderScript->render(this, projectionMatrix, modelViewMatrix);
}

void RD_RendererMTL::beginSpriteSheet( const RD_SpriteSheetPtr_t &pSpriteSheet )
{
  MTL_PREAMBLE;

  RD_SpriteSheetWeakPtr_t pWeakSpriteSheet(pSpriteSheet); // for comparison.
  Impl::TextureCol_t::iterator iTexture = _i->_textures.find( pWeakSpriteSheet );

  //  Retrieve or create texture data.
  Impl::TextureData &data = (iTexture == _i->_textures.end()) ?
    _i->_textures.insert( STD_MakePair( pWeakSpriteSheet, Impl::TextureData() ) ).first->second :
    iTexture->second;

  //  No texture assigned to texture data.  Copy texture.
  if ( data._texture == nil )
  {
    IM_ImagePtr_t pImage = pSpriteSheet->image();
    if ( !pImage.isValid() )
      return;

    unsigned width = pImage->width();
    unsigned height = pImage->height();
    unsigned depth = pImage->depth();

    if ( depth == 4u )
    {
      data._textureBuffer = [_i->_device newBufferWithBytes: pImage->data() length: width*height*depth options: MTLResourceOptionCPUCacheModeDefault];
    }
    else //if ( depth == 3u )
    {
      const unsigned char* srcPixels = static_cast<const unsigned char*>(pImage->data());

      data._textureBuffer = [_i->_device newBufferWithLength: width*height*4 options: MTLResourceOptionCPUCacheModeDefault];
      unsigned char *dstPixels = static_cast<unsigned char*>([data._textureBuffer contents]);

      for ( unsigned y=0 ; y<height ; ++y )
      {
        for ( unsigned x=0 ; x<width ; ++x )
        {
          dstPixels[0] = srcPixels[0];
          dstPixels[1] = srcPixels[1];
          dstPixels[2] = srcPixels[2];
          dstPixels[3] = 255; // opaque

          dstPixels += 4;
          srcPixels += depth;
        }
      }
    }

    MTLTextureDescriptor* textureDesc =
      [[HarmonyGetMetalBundle() classNamed: @"MTLTextureDescriptor"]
       texture2DDescriptorWithPixelFormat: MTLPixelFormatRGBA8Unorm
       width: width
       height: height
       mipmapped: NO
      ];

    data._texture = [data._textureBuffer newTextureWithDescriptor: textureDesc offset:0 bytesPerRow:(width * 4)];

    //  Sprite sheet data has been loaded into graphics memory.  Free unecessary resources.
    pSpriteSheet->freeSpriteSheetData();
  }

  if ( _i->_activeProgram && data._texture != nil )
  {
    _i->_activeProgram->setTextureSampler0( (__bridge void*)data._texture );
  }

}

void RD_RendererMTL::endSpriteSheet()
{
}

void RD_RendererMTL::beginComposition( const RD_RenderScriptPlainPtr_t &pRenderScript )
{
  MTL_PREAMBLE;

  id <MTLRenderCommandEncoder> encoder = MTL_CURRENT_ENCODER();
  if (encoder == nil)
    return;

  RD_RenderScriptWeakPtr_t pWeakRenderScript(pRenderScript.get()); // for comparison.

  const RD_RenderScriptPlain::VertexData *vertices = pRenderScript->vertices();
  size_t verticesCount = pRenderScript->verticesCount();

  const RD_RenderScriptPlain::Index_t *indices = pRenderScript->indices();
  size_t indicesCount = pRenderScript->indicesCount();

  bool mapBuffers = pRenderScript->isRenderDirty();

  id <MTLBuffer> vertexBuffer = nil;
  id <MTLBuffer> indexBuffer = nil;
  id <MTLBuffer> uniformBuffer = nil;

  size_t vertexBufferSize = sizeof(RD_RenderScriptPlain::VertexData) * verticesCount;
  size_t indexBufferSize = sizeof(RD_RenderScriptPlain::Index_t) * indicesCount;
  size_t uniformBufferSize = sizeof(RD_ShaderProgramPlainMTL::UniformBufferDesc);

  Impl::BufferCol_t::iterator iBuffer = _i->_buffers.find( pWeakRenderScript );
  if ( iBuffer != _i->_buffers.end() )
  {
    vertexBuffer = iBuffer->second._vertexBuffer;

    //  Validate size of vertex buffer.
    if ( vertexBuffer != nil )
    {
      if ( [vertexBuffer length] < vertexBufferSize )
      {
        //  Recreate vertex buffer.
        vertexBuffer = nil;
      }
    }

    if ( vertexBuffer == nil )
    {
      vertexBuffer = [_i->_device newBufferWithBytes: vertices length: vertexBufferSize options: MTLResourceOptionCPUCacheModeDefault];
      iBuffer->second._vertexBuffer = vertexBuffer;
    }
    else if (mapBuffers)
    {
      memcpy( [vertexBuffer contents], vertices, vertexBufferSize );
    }

    indexBuffer = iBuffer->second._indexBuffer;

    //  Validate size of index buffer.
    if ( indexBuffer != nil )
    {
      if ( [indexBuffer length] < indexBufferSize )
      {
        //  Recreate index buffer.
        indexBuffer = nil;
      }
    }

    if ( indexBuffer == nil )
    {
      indexBuffer = [_i->_device newBufferWithBytes: indices length: indexBufferSize options: MTLResourceOptionCPUCacheModeDefault];
      iBuffer->second._indexBuffer = indexBuffer;
    }
    else if (mapBuffers)
    {
      memcpy( [indexBuffer contents], indices, indexBufferSize );
    }

    uniformBuffer = iBuffer->second._uniformBuffer;
  }
  else
  {
    Impl::BufferData data;

    data._vertexBuffer = [_i->_device newBufferWithBytes: vertices length: vertexBufferSize options: MTLResourceOptionCPUCacheModeDefault];
    data._indexBuffer = [_i->_device newBufferWithBytes: indices length: indexBufferSize options: MTLResourceOptionCPUCacheModeDefault];
    data._uniformBuffer = [_i->_device newBufferWithLength:uniformBufferSize options:0];

    _i->_buffers.insert( STD_MakePair( pWeakRenderScript, data ) );

    vertexBuffer = data._vertexBuffer;
    indexBuffer = data._indexBuffer;
    uniformBuffer = data._uniformBuffer;
  }

  [encoder setVertexBuffer: vertexBuffer offset: 0 atIndex: 0];
  [encoder setVertexBuffer: uniformBuffer offset: 0 atIndex: 1];

  _i->_activeIndexBuffer = indexBuffer;

  //  Create or retrieve shader program.
  Impl::ShaderProgramCol_t::const_iterator iProgram = _i->_shaders.find( Impl::ePlainShader );

  RD_ShaderProgramPlainMTL *program = static_cast<RD_ShaderProgramPlainMTL*>(
      (iProgram == _i->_shaders.end()) ?
        _i->_shaders.insert( STD_MakePair( Impl::ePlainShader, new RD_ShaderProgramPlainMTL( (__bridge void*)_i->_device ) ) ).first->second :
        iProgram->second );

  //  Activate shader program.
  program->activate( (__bridge void*)encoder, (__bridge void*)uniformBuffer );
  _i->_activeProgram = program;
}

void RD_RendererMTL::endComposition( const RD_RenderScriptPlainPtr_t & /*pRenderScript*/ )
{
  _i->_activeIndexBuffer = nil;
}

void RD_RendererMTL::beginComposition( const RD_RenderScriptFxPtr_t &pRenderScript )
{
  MTL_PREAMBLE;

  id <MTLRenderCommandEncoder> encoder = MTL_CURRENT_ENCODER();
  if (encoder == nil)
    return;

  RD_RenderScriptWeakPtr_t pWeakRenderScript(pRenderScript.get()); // for comparison.

  const RD_RenderScriptFx::VertexData *vertices = pRenderScript->vertices();
  size_t verticesCount = pRenderScript->verticesCount();

  const RD_RenderScriptFx::Index_t *indices = pRenderScript->indices();
  size_t indicesCount = pRenderScript->indicesCount();

  bool mapBuffers = pRenderScript->isRenderDirty();

  id <MTLBuffer> vertexBuffer = nil;
  id <MTLBuffer> indexBuffer = nil;
  id <MTLBuffer> uniformBuffer = nil;

  size_t vertexBufferSize = sizeof(RD_RenderScriptFx::VertexData) * verticesCount;
  size_t indexBufferSize = sizeof(RD_RenderScriptFx::Index_t) * indicesCount;
  size_t uniformBufferSize = sizeof(RD_ShaderProgramFxMTL::UniformBufferDesc);

  Impl::BufferCol_t::iterator iBuffer = _i->_buffers.find( pWeakRenderScript );
  if ( iBuffer != _i->_buffers.end() )
  {
    vertexBuffer = iBuffer->second._vertexBuffer;

    //  Validate size of vertex buffer.
    if ( vertexBuffer != nil )
    {
      if ( [vertexBuffer length] < vertexBufferSize )
      {
        //  Recreate vertex buffer.
        vertexBuffer = nil;
      }
    }

    if ( vertexBuffer == nil )
    {
      vertexBuffer = [_i->_device newBufferWithBytes: vertices length: vertexBufferSize options: MTLResourceOptionCPUCacheModeDefault];
      iBuffer->second._vertexBuffer = vertexBuffer;
    }
    else if (mapBuffers)
    {
      memcpy( [vertexBuffer contents], vertices, vertexBufferSize );
    }

    indexBuffer = iBuffer->second._indexBuffer;

    //  Validate size of index buffer.
    if ( indexBuffer != nil )
    {
      if ( [indexBuffer length] < indexBufferSize )
      {
        //  Recreate index buffer.
        indexBuffer = nil;
      }
    }

    if ( indexBuffer == nil )
    {
      indexBuffer = [_i->_device newBufferWithBytes: indices length: indexBufferSize options: MTLResourceOptionCPUCacheModeDefault];
      iBuffer->second._indexBuffer = indexBuffer;
    }
    else if (mapBuffers)
    {
      memcpy( [indexBuffer contents], indices, indexBufferSize );
    }

    uniformBuffer = iBuffer->second._uniformBuffer;
  }
  else
  {
    Impl::BufferData data;

    data._vertexBuffer = [_i->_device newBufferWithBytes: vertices length: vertexBufferSize options: MTLResourceOptionCPUCacheModeDefault];
    data._indexBuffer = [_i->_device newBufferWithBytes: indices length: indexBufferSize options: MTLResourceOptionCPUCacheModeDefault];
    data._uniformBuffer = [_i->_device newBufferWithLength:uniformBufferSize options:0];

    _i->_buffers.insert( STD_MakePair( pWeakRenderScript, data ) );

    vertexBuffer = data._vertexBuffer;
    indexBuffer = data._indexBuffer;
    uniformBuffer = data._uniformBuffer;
  }

  [encoder setVertexBuffer: vertexBuffer offset: 0 atIndex: 0];
  [encoder setVertexBuffer: uniformBuffer offset: 0 atIndex: 1];

  _i->_activeIndexBuffer = indexBuffer;

  //  Create or retrieve shader program.
  Impl::ShaderProgramCol_t::const_iterator iProgram = _i->_shaders.find( Impl::eFxShader );

  RD_ShaderProgramFxMTL *program = static_cast<RD_ShaderProgramFxMTL*>(
      (iProgram == _i->_shaders.end()) ?
        _i->_shaders.insert( STD_MakePair( Impl::eFxShader, new RD_ShaderProgramFxMTL( (__bridge void*)_i->_device ) ) ).first->second :
        iProgram->second );

  //  Activate shader program.
  program->activate( (__bridge void*)encoder, (__bridge void*)uniformBuffer );
  _i->_activeProgram = program;
}

void RD_RendererMTL::endComposition( const RD_RenderScriptFxPtr_t &/*pRenderScript*/ )
{
  _i->_activeIndexBuffer = nil;
}

void RD_RendererMTL::beginBatch( const RD_RenderScriptFx::RenderBatch &renderBatch )
{
  if (_i->_activeProgram)
  {
    RD_ShaderProgramFxMTL *program = static_cast<RD_ShaderProgramFxMTL*>(_i->_activeProgram);

    unsigned boneCount = renderBatch._uniformBoneMatrices.size();
    unsigned boneCountGPU = (boneCount > MAX_BONES_GPU) ? MAX_BONES_GPU : boneCount;
    program->setBoneMatrices( (const float*)&renderBatch._uniformBoneMatrices[0], boneCountGPU );
  }
}

void RD_RendererMTL::endBatch( const RD_RenderScriptFx::RenderBatch &/*renderBatch*/ )
{
}

void RD_RendererMTL::renderVertices( unsigned short * /*indices*/, unsigned indexOffset, unsigned nIndices, unsigned /*vertexOffset*/, unsigned /*nVertices*/ )
{
  MTL_PREAMBLE;

  id <MTLRenderCommandEncoder> encoder = MTL_CURRENT_ENCODER();
  if (encoder == nil)
    return;

  //NSLog( @"drawIndexedPrimitives: nIndices: %i indexOffset: %i indexBuffer: %li\n", nIndices, indexOffset, [_i->_indexBuffer length] );
  [encoder drawIndexedPrimitives: MTLPrimitiveTypeTriangle indexCount: nIndices indexType: MTLIndexTypeUInt16 indexBuffer: _i->_activeIndexBuffer indexBufferOffset: indexOffset];
}

void RD_RendererMTL::renderVertices( unsigned int * /*indices*/, unsigned indexOffset, unsigned nIndices, unsigned /*vertexOffset*/, unsigned /*nVertices*/ )
{
  MTL_PREAMBLE;

  id <MTLRenderCommandEncoder> encoder = MTL_CURRENT_ENCODER();
  if (encoder == nil)
    return;

  //NSLog( @"drawIndexedPrimitives: nIndices: %i indexOffset: %i indexBuffer: %li\n", nIndices, indexOffset, [_i->_indexBuffer length] );
  [encoder drawIndexedPrimitives: MTLPrimitiveTypeTriangle indexCount: nIndices indexType: MTLIndexTypeUInt32 indexBuffer: _i->_activeIndexBuffer indexBufferOffset: indexOffset];
}

#endif /* SUPPORT_RENDERER_METAL */
