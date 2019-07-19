
#include "RD_RendererDX11.h"

#if defined(SUPPORT_RENDERER_DX11)

#include "RD_ShaderProgramDX11.h"

#include "STD_Containers.h"

#include <stdio.h>
#include <d3d11.h>

// Event types for Direct X
enum GfxDeviceEventType {
	kGfxDeviceEventInitialize = 0,
	kGfxDeviceEventShutdown,
	kGfxDeviceEventBeforeReset,
	kGfxDeviceEventAfterReset,
};

// COM-like Release macro
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(a) if (a) { a->Release(); a = NULL; }
#endif

#ifndef D3DSUCCESS
#define D3DSUCCESS(X) ((X) == S_OK)
#endif

#ifndef D3DFAIL
#define D3DFAIL(X) ((X) != S_OK)
#endif

#ifndef D3DPREAMBLE
#define D3DPREAMBLE if( !_i->_device ) return;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RendererDX11::Impl
#endif

class RD_RendererDX11::Impl
{
  friend class RD_RendererDX11;

public:
  Impl() :
    _device(NULL),
    _context(NULL),
    _rasterState(NULL),
    _blendState(NULL),
    _depthState(NULL),
    _activeProgram(NULL)
  {
  }

  ~Impl()
  {
  }

private:

  ID3D11Device               *_device;
  ID3D11DeviceContext        *_context;

  ID3D11RasterizerState      *_rasterState;
  ID3D11BlendState           *_blendState;
  ID3D11DepthStencilState    *_depthState;

  struct TextureData
  {
    TextureData() :
      _texture(NULL),
      _resourceView(NULL)
    {
    }

    ID3D11Texture2D           *_texture;
    ID3D11ShaderResourceView  *_resourceView;
  };

  typedef STD_Map< RD_SpriteSheetWeakPtr_t, TextureData > TextureCol_t;
  TextureCol_t _textures;

  struct BufferData
  {
    BufferData() :
      _vertexBuffer(NULL),
      _vertexBufferSize(0),
      _indexBuffer(NULL),
      _indexBufferSize(0)
    {
    }

    ID3D11Buffer              *_vertexBuffer;
    UINT                       _vertexBufferSize;

    ID3D11Buffer              *_indexBuffer;
    UINT                       _indexBufferSize;
  };

  typedef STD_Map< RD_RenderScriptWeakPtr_t, BufferData > BufferCol_t;
  BufferCol_t _buffers;

  struct RenderTargetData
  {
    RenderTargetData() :
      _texture(NULL),
      _owned(false),
      _renderTargetView(NULL)
    {
    }

    ID3D11Texture2D          *_texture;
    D3D11_TEXTURE2D_DESC      _textureDesc;
    bool                      _owned;

    ID3D11RenderTargetView   *_renderTargetView;
  };

  typedef STD_Map< RD_RenderScriptWeakPtr_t, RenderTargetData > RenderTargetCol_t;
  RenderTargetCol_t _renderTargets;


  enum ShaderMode_t
  {
    eNullShader,
    ePlainShader,
    eFxShader
  };

  typedef STD_Map< ShaderMode_t, RD_ShaderProgramDX11* > ShaderProgramCol_t;
  ShaderProgramCol_t _shaders;

  RD_ShaderProgramDX11 *_activeProgram;

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RendererDX11
#endif

RD_RendererDX11::RD_RendererDX11()
{
  _i = new Impl;
}

RD_RendererDX11::~RD_RendererDX11()
{
  delete _i;
}

void RD_RendererDX11::initialize()
{
  //  Nothing to do.
}

void RD_RendererDX11::shutdown()
{
  //  Nothing to do.
}

void RD_RendererDX11::setDevice( void *device, int eventType )
{
  _i->_device = (ID3D11Device*)device;

  //  Update device in shaders as well.
  for ( Impl::ShaderProgramCol_t::iterator i = _i->_shaders.begin(), iEnd = _i->_shaders.end() ; i!=iEnd ; ++i )
  {
    i->second->setDevice( _i->_device );
  }

	if (eventType == kGfxDeviceEventInitialize)
  {
    //  Disable culling.  Geometry can be flipped.
    D3D11_RASTERIZER_DESC rsdesc;
    memset (&rsdesc, 0, sizeof(rsdesc));
    rsdesc.FillMode = D3D11_FILL_SOLID;
    rsdesc.CullMode = D3D11_CULL_NONE;
    rsdesc.DepthClipEnable = TRUE;
    _i->_device->CreateRasterizerState (&rsdesc, &_i->_rasterState);

    //  Enable blending.  Consider premultiplied textures.
    D3D11_BLEND_DESC bdesc;
    memset (&bdesc, 0, sizeof(bdesc));
    bdesc.RenderTarget[0].BlendEnable = TRUE;
    bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bdesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bdesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bdesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    bdesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bdesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    _i->_device->CreateBlendState (&bdesc, &_i->_blendState);

    //  Test depth buffer, but disable writing to avoid z-fighting issues.
    D3D11_DEPTH_STENCIL_DESC dsdesc;
    memset (&dsdesc, 0, sizeof(dsdesc));
    dsdesc.DepthEnable = TRUE;
    dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    _i->_device->CreateDepthStencilState (&dsdesc, &_i->_depthState);
  }
  else if (eventType == kGfxDeviceEventShutdown)
  {
    SAFE_RELEASE(_i->_rasterState);
    SAFE_RELEASE(_i->_blendState);
    SAFE_RELEASE(_i->_depthState);

    //  Make sure all buffers are deallocated.
    cleanupPriv(true /*forceCleanup*/);

    //  unassign device which will be invalid thereafter.
    _i->_device = 0;
  }
}

void RD_RendererDX11::cleanup()
{
  D3DPREAMBLE;

  cleanupPriv(false /*forceCleanup*/);
}

void RD_RendererDX11::beginRender()
{
  D3DPREAMBLE;

  //  Retrieve context for immediate rendering.
  _i->_device->GetImmediateContext(&_i->_context);

  //  Render states.
  _i->_context->OMSetDepthStencilState (_i->_depthState, 0);
  _i->_context->RSSetState (_i->_rasterState);
  _i->_context->OMSetBlendState (_i->_blendState, NULL, 0xFFFFFFFF);

  cleanup();

  //  None of our shaders are active at this point.
  _i->_activeProgram = 0;
}

void RD_RendererDX11::endRender()
{
  //  Release rendering context.
  SAFE_RELEASE(_i->_context);
}

void RD_RendererDX11::setMatrices( const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix )
{
  D3DPREAMBLE;

  if ( _i->_activeProgram != NULL )
  {
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

    _i->_activeProgram->setMatrix(array);
  }
}

void RD_RendererDX11::renderScriptToTexture( const RD_RenderScriptPtr_t &pRenderScript, const Rect &rect, float scale, TexturePtr_t texture )
{
  D3DPREAMBLE;

  RD_RenderScriptWeakPtr_t pWeakRenderScript(pRenderScript); // for comparison.
  Impl::RenderTargetCol_t::iterator iRenderTarget = _i->_renderTargets.find( pWeakRenderScript );

  //  Retrieve or create texture data.
  Impl::RenderTargetData &data = (iRenderTarget == _i->_renderTargets.end()) ?
    _i->_renderTargets.insert( STD_MakePair( pWeakRenderScript, Impl::RenderTargetData() ) ).first->second :
    iRenderTarget->second;

  ID3D11Texture2D *d3dTexture = (ID3D11Texture2D*)texture;
  if ( d3dTexture == NULL )
    return;

  bool createRenderTarget = (data._renderTargetView == NULL);
  if ( !createRenderTarget )
  {
    if ( data._texture != d3dTexture )
    {
      SAFE_RELEASE(data._renderTargetView);

      if ( data._owned )
      {
        SAFE_RELEASE(data._texture);
      }
      else
      {
        data._texture = NULL;
      }

      createRenderTarget = true;
    }
  }

  if ( createRenderTarget )
  {
    d3dTexture->GetDesc( &data._textureDesc );

    if ( data._textureDesc.BindFlags & D3D11_BIND_RENDER_TARGET )
    {
      D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
      renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
      renderTargetViewDesc.Texture2D.MipSlice = 0;

      // Create the render target view.
      if ( D3DSUCCESS(_i->_device->CreateRenderTargetView( d3dTexture, &renderTargetViewDesc, &data._renderTargetView)) )
      {
        data._texture = d3dTexture;
        data._owned = false; /* texture comes from external source */
      }
    }
  }

  if ( data._renderTargetView == NULL )
    return;

  //  Backup render target.
  ID3D11RenderTargetView *inRenderTargetView = NULL;
  ID3D11DepthStencilView *inDepthStencilView = NULL;
  _i->_context->OMGetRenderTargets(1, &inRenderTargetView, &inDepthStencilView );

  _i->_context->OMSetRenderTargets(1, &data._renderTargetView, NULL);

  D3D11_VIEWPORT viewport;
  viewport.Width = (FLOAT)data._textureDesc.Width;
  viewport.Height = (FLOAT)data._textureDesc.Height;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  viewport.TopLeftX = 0.0f;
  viewport.TopLeftY = 0.0f;

  _i->_context->RSSetViewports(1, &viewport);

  float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  _i->_context->ClearRenderTargetView(data._renderTargetView, clearColor);

  // Setup orthographic projection for rendering to texture coordinates.
  Math::Matrix4x4 projMatrix;
  projMatrix.orthogonalProject( rect._offsetx, rect._offsetx + rect._width, rect._offsety, rect._offsety + rect._height, -1000, 1000 );

  Math::Matrix4x4 modelMatrix;
  modelMatrix.scale( scale, scale );

  //  Render script as usual.
  renderScript( pRenderScript, projMatrix, modelMatrix );

  //  Restore render target.
	_i->_context->OMSetRenderTargets(1, &inRenderTargetView, inDepthStencilView);

  SAFE_RELEASE(inRenderTargetView);
  SAFE_RELEASE(inDepthStencilView);
}

void RD_RendererDX11::renderScript( const RD_RenderScriptPtr_t &pRenderScript, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix )
{
  pRenderScript->render(this, projectionMatrix, modelViewMatrix);
}

void RD_RendererDX11::beginSpriteSheet( const RD_SpriteSheetPtr_t &pSpriteSheet )
{
  D3DPREAMBLE;

  RD_SpriteSheetWeakPtr_t pWeakSpriteSheet(pSpriteSheet); // for comparison.
  Impl::TextureCol_t::iterator iTexture = _i->_textures.find( pWeakSpriteSheet );

  //  Retrieve or create texture data.
  Impl::TextureData &data = (iTexture == _i->_textures.end()) ?
    _i->_textures.insert( STD_MakePair( pWeakSpriteSheet, Impl::TextureData() ) ).first->second :
    iTexture->second;

  //  No texture assigned to texture data.  Copy texture.
  if ( data._texture == NULL )
  {
    IM_ImagePtr_t pImage = pSpriteSheet->image();
    if ( !pImage.isValid() )
      return;

    unsigned width = pImage->width();
    unsigned height = pImage->height();
    unsigned depth = pImage->depth();

    CD3D11_TEXTURE2D_DESC
      textureDesc( DXGI_FORMAT_R8G8B8A8_UNORM, // Format
                   width,                      // Width
                   height,                     // Height,
                   1,                          // ArraySize,
                   1,                          // MipLevels,
                   D3D11_BIND_SHADER_RESOURCE, // BindFlags,
                   D3D11_USAGE_IMMUTABLE       // Usage
                 );

    D3D11_SUBRESOURCE_DATA textureData;

    BYTE *tempData = NULL;

    if ( depth == 4u )
    {
      textureData.pSysMem = (const void*)pImage->data();
      textureData.SysMemPitch = width * depth;
      textureData.SysMemSlicePitch = width * height * depth;
    }
    else //if ( depth == 3u )
    {
      tempData = new BYTE[ width * height * 4 ];

      const BYTE* srcPixels = static_cast<const BYTE*>(pImage->data());
      BYTE *dstPixels = tempData;

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

      textureData.pSysMem = (const void*)tempData;
      textureData.SysMemPitch = width * 4;
      textureData.SysMemSlicePitch = width * height * 4;
    }

    if ( D3DSUCCESS(_i->_device->CreateTexture2D( &textureDesc, &textureData, &data._texture )) )
    {
      CD3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc( data._texture, D3D11_SRV_DIMENSION_TEXTURE2D );
      _i->_device->CreateShaderResourceView( data._texture, &resourceViewDesc, &data._resourceView );
    }

    //  free temp data.
    delete [] tempData;

    //  Sprite sheet data has been loaded into graphics memory.  Free unecessary resources.
    pSpriteSheet->freeSpriteSheetData();
  }

  if ( _i->_activeProgram && data._resourceView )
  {
    _i->_activeProgram->setTextureSampler0(data._resourceView);
  }
}

void RD_RendererDX11::endSpriteSheet()
{
  D3DPREAMBLE;

  if ( _i->_activeProgram )
  {
    _i->_activeProgram->setTextureSampler0(0);
  }
}

void RD_RendererDX11::beginComposition( const RD_RenderScriptPlainPtr_t &pRenderScript )
{
  D3DPREAMBLE;

  //  Create or retrieve shader program.
  Impl::ShaderProgramCol_t::const_iterator iProgram = _i->_shaders.find( Impl::ePlainShader );

  RD_ShaderProgramPlainDX11 *program = static_cast<RD_ShaderProgramPlainDX11*>(
      (iProgram == _i->_shaders.end()) ?
        _i->_shaders.insert( STD_MakePair( Impl::ePlainShader, new RD_ShaderProgramPlainDX11( (void*)_i->_device ) ) ).first->second :
        iProgram->second );

  //  Activate shader program if not already active.
  if ( _i->_activeProgram != program )
  {
    program->activate( (void*)_i->_context );
    _i->_activeProgram = program;
  }

  RD_RenderScriptWeakPtr_t pWeakRenderScript(pRenderScript.get()); // for comparison.

  const RD_RenderScriptPlain::VertexData *vertices = pRenderScript->vertices();
  size_t verticesCount = pRenderScript->verticesCount();

  const RD_RenderScriptPlain::Index_t *indices = pRenderScript->indices();
  size_t indicesCount = pRenderScript->indicesCount();

  bool mapBuffers = pRenderScript->isRenderDirty();

  ID3D11Buffer *vertexBuffer = NULL;
  ID3D11Buffer *indexBuffer = NULL;

  UINT vertexBufferSize = (UINT)(sizeof(RD_RenderScriptPlain::VertexData) * verticesCount);
  UINT indexBufferSize = (UINT)(sizeof(RD_RenderScriptPlain::Index_t) * indicesCount);

  Impl::BufferCol_t::iterator iBuffer = _i->_buffers.find( pWeakRenderScript );
  if ( iBuffer != _i->_buffers.end() )
  {
    vertexBuffer = iBuffer->second._vertexBuffer;

    //  Validate size of vertex buffer.
    if ( vertexBuffer != NULL )
    {
      if ( iBuffer->second._vertexBufferSize < vertexBufferSize )
      {
        //  Recreate vertex buffer.
        SAFE_RELEASE(vertexBuffer);
        vertexBuffer = NULL;
      }
    }

    //  vertex buffer has been reset, recreate.
    if ( vertexBuffer == NULL )
    {
      D3D11_BUFFER_DESC desc;
      desc.ByteWidth = vertexBufferSize;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      desc.CPUAccessFlags = 0;
      desc.MiscFlags = 0;
      desc.StructureByteStride = 0;

      _i->_device->CreateBuffer (&desc, NULL, &vertexBuffer);

      iBuffer->second._vertexBuffer = vertexBuffer;
      iBuffer->second._vertexBufferSize = vertexBufferSize;
    }

    indexBuffer = iBuffer->second._indexBuffer;

    //  Validate size of index buffer.
    if ( indexBuffer != NULL )
    {
      if ( iBuffer->second._indexBufferSize < indexBufferSize )
      {
        //  Recreate index buffer.
        SAFE_RELEASE(indexBuffer);
        indexBuffer = NULL;
      }
    }

    //  vertex buffer has been reset, recreate.
    if ( indexBuffer == NULL )
    {
      D3D11_BUFFER_DESC desc;
      desc.ByteWidth = indexBufferSize;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
      desc.CPUAccessFlags = 0;
      desc.MiscFlags = 0;
      desc.StructureByteStride = 0;

      _i->_device->CreateBuffer (&desc, NULL, &indexBuffer);

      iBuffer->second._indexBuffer = indexBuffer;
      iBuffer->second._indexBufferSize = indexBufferSize;
    }
  }
  else
  {
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = vertexBufferSize;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    Impl::BufferData data;

    _i->_device->CreateBuffer (&desc, NULL, &data._vertexBuffer);
    data._vertexBufferSize = vertexBufferSize;

    vertexBuffer = data._vertexBuffer;

    desc.ByteWidth = indexBufferSize;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    _i->_device->CreateBuffer (&desc, NULL, &data._indexBuffer);
    data._indexBufferSize = indexBufferSize;

    indexBuffer = data._indexBuffer;

    _i->_buffers.insert( STD_MakePair( pWeakRenderScript, data ) );
  }

  if ( mapBuffers )
  {
    _i->_context->UpdateSubresource ( vertexBuffer, 0, NULL, vertices, vertexBufferSize, 0);
    _i->_context->UpdateSubresource ( indexBuffer, 0, NULL, indices, indexBufferSize, 0);
  }

  _i->_context->IASetPrimitiveTopology ( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

  UINT stride = sizeof(RD_RenderScriptPlain::VertexData);
  UINT offset = 0;
  _i->_context->IASetVertexBuffers (0, 1, &vertexBuffer, &stride, &offset);

  _i->_context->IASetIndexBuffer ( indexBuffer, DXGI_FORMAT_R16_UINT, 0 );
}

void RD_RendererDX11::endComposition( const RD_RenderScriptPlainPtr_t &/*pRenderScript*/)
{
}

void RD_RendererDX11::beginComposition( const RD_RenderScriptFxPtr_t &pRenderScript)
{
  D3DPREAMBLE;

  //  Create or retrieve shader program.
  Impl::ShaderProgramCol_t::const_iterator iProgram = _i->_shaders.find( Impl::eFxShader );

  RD_ShaderProgramFxDX11 *program = static_cast<RD_ShaderProgramFxDX11*>(
      (iProgram == _i->_shaders.end()) ?
        _i->_shaders.insert( STD_MakePair( Impl::eFxShader, new RD_ShaderProgramFxDX11( (void*)_i->_device ) ) ).first->second :
        iProgram->second );

  //  Activate shader program if not already active.
  if ( _i->_activeProgram != program )
  {
    program->activate( (void*)_i->_context );
    _i->_activeProgram = program;
  }

  RD_RenderScriptWeakPtr_t pWeakRenderScript(pRenderScript.get()); // for comparison.

  const RD_RenderScriptFx::VertexData *vertices = pRenderScript->vertices();
  size_t verticesCount = pRenderScript->verticesCount();

  const RD_RenderScriptFx::Index_t *indices = pRenderScript->indices();
  size_t indicesCount = pRenderScript->indicesCount();

  bool mapBuffers = pRenderScript->isRenderDirty();

  ID3D11Buffer *vertexBuffer = NULL;
  ID3D11Buffer *indexBuffer = NULL;

  UINT vertexBufferSize = (UINT)(sizeof(RD_RenderScriptFx::VertexData) * verticesCount);
  UINT indexBufferSize = (UINT)(sizeof(RD_RenderScriptFx::Index_t) * indicesCount);

  Impl::BufferCol_t::iterator iBuffer = _i->_buffers.find( pWeakRenderScript );
  if ( iBuffer != _i->_buffers.end() )
  {
    vertexBuffer = iBuffer->second._vertexBuffer;

    //  Validate size of vertex buffer.
    if ( vertexBuffer != NULL )
    {
      if ( iBuffer->second._vertexBufferSize < vertexBufferSize )
      {
        //  Recreate vertex buffer.
        SAFE_RELEASE(vertexBuffer);
        vertexBuffer = NULL;
      }
    }

    //  vertex buffer has been reset, recreate.
    if ( vertexBuffer == NULL )
    {
      D3D11_BUFFER_DESC desc;
      desc.ByteWidth = vertexBufferSize;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      desc.CPUAccessFlags = 0;
      desc.MiscFlags = 0;
      desc.StructureByteStride = 0;

      _i->_device->CreateBuffer (&desc, NULL, &vertexBuffer);

      iBuffer->second._vertexBuffer = vertexBuffer;
      iBuffer->second._vertexBufferSize = vertexBufferSize;
    }

    indexBuffer = iBuffer->second._indexBuffer;

    //  Validate size of index buffer.
    if ( indexBuffer != NULL )
    {
      if ( iBuffer->second._indexBufferSize < indexBufferSize )
      {
        //  Recreate index buffer.
        SAFE_RELEASE(indexBuffer);
        indexBuffer = NULL;
      }
    }

    //  vertex buffer has been reset, recreate.
    if ( indexBuffer == NULL )
    {
      D3D11_BUFFER_DESC desc;
      desc.ByteWidth = indexBufferSize;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
      desc.CPUAccessFlags = 0;
      desc.MiscFlags = 0;
      desc.StructureByteStride = 0;

      _i->_device->CreateBuffer (&desc, NULL, &indexBuffer);

      iBuffer->second._indexBuffer = indexBuffer;
      iBuffer->second._indexBufferSize = indexBufferSize;
    }
  }
  else
  {
    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = vertexBufferSize;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    Impl::BufferData data;

    _i->_device->CreateBuffer (&desc, NULL, &data._vertexBuffer);
    data._vertexBufferSize = vertexBufferSize;

    vertexBuffer = data._vertexBuffer;

    desc.ByteWidth = indexBufferSize;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    _i->_device->CreateBuffer (&desc, NULL, &data._indexBuffer);
    data._indexBufferSize = indexBufferSize;

    indexBuffer = data._indexBuffer;

    _i->_buffers.insert( STD_MakePair( pWeakRenderScript, data ) );
  }

  if ( mapBuffers )
  {
    _i->_context->UpdateSubresource ( vertexBuffer, 0, NULL, vertices, vertexBufferSize, 0);
    _i->_context->UpdateSubresource ( indexBuffer, 0, NULL, indices, indexBufferSize, 0);
  }

  _i->_context->IASetPrimitiveTopology ( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

  UINT stride = sizeof(RD_RenderScriptFx::VertexData);
  UINT offset = 0;
  _i->_context->IASetVertexBuffers (0, 1, &vertexBuffer, &stride, &offset);

  _i->_context->IASetIndexBuffer ( indexBuffer, DXGI_FORMAT_R32_UINT, 0 );
}

void RD_RendererDX11::endComposition( const RD_RenderScriptFxPtr_t &/*pRenderScript*/)
{
}


void RD_RendererDX11::beginBatch(const RD_RenderScriptFx::RenderBatch &renderBatch)
{
  RD_ShaderProgramFxDX11 *program = static_cast<RD_ShaderProgramFxDX11*>(_i->_activeProgram);

  unsigned boneCount = renderBatch._uniformBoneMatrices.size();
  unsigned boneCountGPU = (boneCount > MAX_BONES_GPU) ? MAX_BONES_GPU : boneCount;
  program->setBoneMatrices( (const float*)&renderBatch._uniformBoneMatrices[0], boneCountGPU );
}

void RD_RendererDX11::endBatch(const RD_RenderScriptFx::RenderBatch &/*renderBatch*/)
{
}

void RD_RendererDX11::renderVertices( unsigned short * /*indices*/, unsigned indexOffset, unsigned nIndices, unsigned /*vertexOffset*/, unsigned /*nVertices*/ )
{
  D3DPREAMBLE;

  if ( nIndices > 0u )
  {
    if ( _i->_activeProgram != NULL )
    {
      _i->_activeProgram->updateConstants();
    }

    _i->_context->DrawIndexed( nIndices, indexOffset, 0 );
  }
}

void RD_RendererDX11::renderVertices( unsigned int * /*indices*/, unsigned indexOffset, unsigned nIndices, unsigned /*vertexOffset*/, unsigned /*nVertices*/ )
{
  D3DPREAMBLE;

  if ( nIndices > 0u )
  {
    if ( _i->_activeProgram != NULL )
    {
      _i->_activeProgram->updateConstants();
    }

    _i->_context->DrawIndexed( nIndices, indexOffset, 0 );
  }
}

void RD_RendererDX11::cleanupPriv( bool forceCleanup )
{
  //  Clear unused buffers.
  for ( Impl::BufferCol_t::iterator i = _i->_buffers.begin(), iEnd = _i->_buffers.end() ; i!=iEnd ; )
  {
    if ( !i->first.isValid() || forceCleanup )
    {
      //  Free vertex buffer and index buffer.
      SAFE_RELEASE(i->second._vertexBuffer);
      SAFE_RELEASE(i->second._indexBuffer);

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
    if ( !i->first.isValid() || forceCleanup )
    {
      SAFE_RELEASE(i->second._texture);
      SAFE_RELEASE(i->second._resourceView);

      _i->_textures.erase( i++ );
    }
    else
    {
      ++i;
    }
  }

  deleteProgram = deleteProgram && _i->_textures.empty();

  //  Clear unused render targets.
  for ( Impl::RenderTargetCol_t::iterator i = _i->_renderTargets.begin(), iEnd = _i->_renderTargets.end() ; i!=iEnd ; )
  {
    if ( !i->first.isValid() )
    {
      SAFE_RELEASE(i->second._renderTargetView);

      if (i->second._owned)
        SAFE_RELEASE(i->second._texture);

      _i->_renderTargets.erase( i++ );
    }
    else
    {
      ++i;
    }
  }

  deleteProgram = deleteProgram && _i->_renderTargets.empty();

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

#endif /* SUPPORT_RENDERER_DX11 */
