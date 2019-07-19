
#include "RD_RendererDX9.h"

#if defined(SUPPORT_RENDERER_DX9)

#include "RD_ShaderProgramDX9.h"

#include "STD_Containers.h"

#include <stdio.h>
#include <d3d9.h>

// Event types for Direct X
enum GfxDeviceEventType {
	kGfxDeviceEventInitialize = 0,
	kGfxDeviceEventShutdown,
	kGfxDeviceEventBeforeReset,
	kGfxDeviceEventAfterReset,
};

//  Matches vertex data data buffer.
#define RENDER_SCRIPT_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

#define TEXTURE_STAGE 0

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
#pragma mark - RD_RendererDX9::Impl
#endif

class RD_RendererDX9::Impl
{
  friend class RD_RendererDX9;

public:
  Impl() :
    _device(NULL),
    _activeProgram(NULL)
  {
  }

  ~Impl()
  {
  }

private:

  IDirect3DDevice9           *_device;

  struct TextureData
  {
    TextureData() :
      _texture(NULL)
    {
    }

    IDirect3DTexture9         *_texture;
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

    IDirect3DVertexBuffer9    *_vertexBuffer;
    UINT                       _vertexBufferSize;

    IDirect3DIndexBuffer9     *_indexBuffer;
    UINT                       _indexBufferSize;
  };

  typedef STD_Map< RD_RenderScriptWeakPtr_t, BufferData > BufferCol_t;
  BufferCol_t _buffers;

  struct RenderTargetData
  {
    RenderTargetData() :
      _texture(NULL),
      _owned(false),
      _renderTarget(NULL),
      _depthStencilTarget(NULL)
    {
    }

    IDirect3DTexture9        *_texture;
    D3DSURFACE_DESC           _textureDesc;
    bool                      _owned;

    IDirect3DSurface9        *_renderTarget;
    IDirect3DSurface9        *_depthStencilTarget;
  };

  typedef STD_Map< RD_RenderScriptWeakPtr_t, RenderTargetData > RenderTargetCol_t;
  RenderTargetCol_t _renderTargets;

  enum ShaderMode_t
  {
    eNullShader,
    eFxShader
  };

  typedef STD_Map< ShaderMode_t, RD_ShaderProgramDX9* > ShaderProgramCol_t;
  ShaderProgramCol_t _shaders;

  RD_ShaderProgramDX9 *_activeProgram;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RendererDX9
#endif

RD_RendererDX9::RD_RendererDX9()
{
  _i = new Impl;
}

RD_RendererDX9::~RD_RendererDX9()
{
  delete _i;
}

void RD_RendererDX9::initialize()
{
  //  Nothing to do.
}

void RD_RendererDX9::shutdown()
{
  //  Nothing to do.
}

void RD_RendererDX9::setDevice( void *device, int eventType )
{
  _i->_device = (IDirect3DDevice9*)device;

  //  Before device is reset or being shut down, release vertex buffers and index buffers.
  if ( (eventType == kGfxDeviceEventBeforeReset) ||
       (eventType == kGfxDeviceEventShutdown ) )
  {
    for ( Impl::BufferCol_t::iterator i = _i->_buffers.begin(), iEnd = _i->_buffers.end() ; i!=iEnd ; ++i )
    {
      //  Release buffers and set to NULL.  Will be re-instantiated.
      SAFE_RELEASE(i->second._vertexBuffer);
      SAFE_RELEASE(i->second._indexBuffer);
    }

    for ( Impl::TextureCol_t::iterator i = _i->_textures.begin(), iEnd = _i->_textures.end() ; i!=iEnd ; ++i )
    {
      //  Release textures and set to NULL.  Will be re-instantiated.
      SAFE_RELEASE(i->second._texture);
    }

    for ( Impl::RenderTargetCol_t::iterator i = _i->_renderTargets.begin(), iEnd = _i->_renderTargets.end() ; i!=iEnd ; ++i )
    {
      //  Release render targets.
      SAFE_RELEASE(i->second._renderTarget);
      SAFE_RELEASE(i->second._depthStencilTarget);

      //  Release textures and set to NULL.  Will be re-instantiated.
      if (i->second._owned)
      {
        SAFE_RELEASE(i->second._texture);
      }
      else
      {
        i->second._texture = 0;
      }
    }

    for ( Impl::ShaderProgramCol_t::iterator i = _i->_shaders.begin(), iEnd = _i->_shaders.end() ; i!=iEnd ; ++i )
    {
      //  Release shaders.
      i->second->unload();
    }
    _i->_activeProgram = NULL;

    //  Unassign device which will be invalid thereafter.
    _i->_device = 0;
  }

  //  Update device in shaders as well.
  for ( Impl::ShaderProgramCol_t::iterator i = _i->_shaders.begin(), iEnd = _i->_shaders.end() ; i!=iEnd ; ++i )
  {
    i->second->setDevice( _i->_device );
  }
}

void RD_RendererDX9::cleanup()
{
  //  Clear unused buffers.
  for ( Impl::BufferCol_t::iterator i = _i->_buffers.begin(), iEnd = _i->_buffers.end() ; i!=iEnd ; )
  {
    if ( !i->first.isValid() )
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
    if ( !i->first.isValid() )
    {
      SAFE_RELEASE(i->second._texture);

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
      SAFE_RELEASE(i->second._renderTarget);
      SAFE_RELEASE(i->second._depthStencilTarget);

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

void RD_RendererDX9::beginRender()
{
  D3DPREAMBLE;

  //  Disable culling.  Geometry can be flipped.
  _i->_device->SetRenderState (D3DRS_CULLMODE, D3DCULL_NONE);

  //_i->_device->SetRenderState (D3DRS_LIGHTING, FALSE);

  //  Enable blending.  Consider premultiplied textures.
  _i->_device->SetRenderState (D3DRS_ALPHABLENDENABLE, TRUE);
  _i->_device->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_ONE);
  _i->_device->SetRenderState (D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

  //  Enable alpha blending.
  //_i->_device->SetRenderState (D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
  //_i->_device->SetRenderState (D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
  //_i->_device->SetRenderState (D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);

  //  Test depth buffer, but disable writing to avoid z-fighting issues.
  _i->_device->SetRenderState (D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
  _i->_device->SetRenderState (D3DRS_ZWRITEENABLE, FALSE);

  _i->_device->SetTextureStageState (TEXTURE_STAGE, D3DTSS_COLOROP, D3DTOP_MODULATE);
  _i->_device->SetTextureStageState (TEXTURE_STAGE, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  _i->_device->SetTextureStageState (TEXTURE_STAGE, D3DTSS_COLORARG2, D3DTA_CURRENT);
  _i->_device->SetTextureStageState (TEXTURE_STAGE, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
  _i->_device->SetTextureStageState (TEXTURE_STAGE, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  _i->_device->SetTextureStageState (TEXTURE_STAGE, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
  _i->_device->SetTextureStageState (TEXTURE_STAGE, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);

  _i->_device->SetSamplerState(TEXTURE_STAGE, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  _i->_device->SetSamplerState(TEXTURE_STAGE, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  _i->_device->SetSamplerState(TEXTURE_STAGE, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
  _i->_device->SetSamplerState(TEXTURE_STAGE, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  _i->_device->SetSamplerState(TEXTURE_STAGE, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

  cleanup();

  //  None of our shaders are active at this point.
  _i->_activeProgram = 0;
}

void RD_RendererDX9::endRender()
{
}

void RD_RendererDX9::setMatrices( const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix )
{
  D3DPREAMBLE;

  if ( _i->_activeProgram != 0 )
  {
    Math::Matrix4x4 mvpMatrix = projectionMatrix;

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
  else
  {
    float array[16];

    projectionMatrix.getValues(array);

    //  Shift near plane.  DirectX maps clip plane to [0, 1], while OpenGL maps it to [-1,1].
    array[2] = (array[2] + array[3]) * 0.5f;
    array[6] = (array[6] + array[7]) * 0.5f;
    array[10] = (array[10] + array[11]) * 0.5f;
    array[14] = (array[14] + array[15]) * 0.5f;

    _i->_device->SetTransform (D3DTS_PROJECTION, (const D3DMATRIX*)array );

    modelViewMatrix.getValues(array);
    _i->_device->SetTransform (D3DTS_VIEW, (const D3DMATRIX*)array );

    static float g_identityArray[16] = {1.0f,0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f,0.0f, 0.0f,0.0f,1.0f,0.0f, 0.0f,0.0f,0.0f,1.0f};
    _i->_device->SetTransform (D3DTS_WORLD, (const D3DMATRIX*)g_identityArray );
  }
}

void RD_RendererDX9::renderScriptToTexture( const RD_RenderScriptPtr_t &pRenderScript, const Rect &rect, float scale, TexturePtr_t texture )
{
  D3DPREAMBLE;

  RD_RenderScriptWeakPtr_t pWeakRenderScript(pRenderScript); // for comparison.
  Impl::RenderTargetCol_t::iterator iRenderTarget = _i->_renderTargets.find( pWeakRenderScript );

  //  Retrieve or create texture data.
  Impl::RenderTargetData &data = (iRenderTarget == _i->_renderTargets.end()) ?
    _i->_renderTargets.insert( STD_MakePair( pWeakRenderScript, Impl::RenderTargetData() ) ).first->second :
    iRenderTarget->second;

  IDirect3DTexture9 *d3dTexture = (IDirect3DTexture9*)texture;
  if ( d3dTexture == NULL )
    return;

  bool createRenderTarget = (data._renderTarget == NULL);
  if ( !createRenderTarget )
  {
    if ( data._texture != d3dTexture )
    {
      SAFE_RELEASE(data._renderTarget);
      SAFE_RELEASE(data._depthStencilTarget);

      if ( data._owned )
      {
        SAFE_RELEASE(data._texture);
      }
      else
      {
        data._texture = 0;
      }

      createRenderTarget = true;
    }
  }

  DWORD renderIdx = 0;

  IDirect3DSurface9* inRenderTarget = NULL;
  if ( D3DFAIL( _i->_device->GetRenderTarget( renderIdx, &inRenderTarget ) ) )
    return;

  IDirect3DSurface9* inDepthStencilTarget = NULL;
  if ( D3DFAIL( _i->_device->GetDepthStencilSurface( &inDepthStencilTarget ) ) )
    return;

  if ( createRenderTarget )
  {
    D3DSURFACE_DESC surfaceDesc;
    inRenderTarget->GetDesc( &surfaceDesc );

    d3dTexture->GetLevelDesc( 0, &data._textureDesc );

    //  Only texture located on default memory pool can be used with a render target.
    if ( data._textureDesc.Pool == D3DPOOL_DEFAULT )
    {
      //  Retrieve surface from input texture.
      if ( D3DSUCCESS( d3dTexture->GetSurfaceLevel(0, &data._renderTarget) ) )
      {
        //  Create depth stencil surface that will be used alongside texture surface.
        if ( D3DSUCCESS( _i->_device->CreateDepthStencilSurface( rect._width, rect._height, D3DFMT_D16,
                                                                 data._textureDesc.MultiSampleType,
                                                                 data._textureDesc.MultiSampleQuality,
                                                                 true, &data._depthStencilTarget, NULL) ) )
        {
          data._texture = d3dTexture;
          data._owned = false; /* texture comes from external source */
        }
        else
        {
          SAFE_RELEASE(data._renderTarget);
        }
      }
    }
  }

  if ( data._renderTarget == NULL )
    return;

  if ( D3DSUCCESS(_i->_device->SetRenderTarget( renderIdx, data._renderTarget )) &&
       D3DSUCCESS(_i->_device->SetDepthStencilSurface( data._depthStencilTarget )) )
  {
    // Clear surface.
    _i->_device->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0,0,0,0), 1, 0 );

    // Setup orthographic projection for rendering to texture coordinates.
    Math::Matrix4x4 projMatrix;
    projMatrix.orthogonalProject( rect._offsetx, rect._offsetx + rect._width, rect._offsety, rect._offsety + rect._height, -1000, 1000 );

    Math::Matrix4x4 modelMatrix;
    modelMatrix.scale( scale, scale );

    //  Render script as usual.
    renderScript( pRenderScript, projMatrix, modelMatrix );
  }

  //  Set back initial values.
  _i->_device->SetRenderTarget( renderIdx, inRenderTarget );
  _i->_device->SetDepthStencilSurface( inDepthStencilTarget );

  //  Release unused surfaces.
  SAFE_RELEASE(inRenderTarget);
}

void RD_RendererDX9::renderScript( const RD_RenderScriptPtr_t &pRenderScript, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix )
{
  pRenderScript->render(this, projectionMatrix, modelViewMatrix);
}

void RD_RendererDX9::beginSpriteSheet( const RD_SpriteSheetPtr_t &pSpriteSheet )
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

    D3DFORMAT d3dfmt = (depth == 4u) ? D3DFMT_A8R8G8B8 : D3DFMT_R8G8B8;

    //  Using D3DPOOL_MANAGED or I won't be able to access my texture data.
    HRESULT hRes = _i->_device->CreateTexture(width, height, 1, 0, d3dfmt, D3DPOOL_MANAGED, &data._texture, NULL);

    if ( FAILED(hRes) || !data._texture )
      return;

    D3DSURFACE_DESC ddsd;
    if ( D3DFAIL(data._texture->GetLevelDesc( 0, &ddsd ) ) )
    {
      SAFE_RELEASE(data._texture);
      return;
    }

    if ( ddsd.Format != d3dfmt )
    {
      SAFE_RELEASE(data._texture);
      return;
    }

    D3DLOCKED_RECT d3dlr;
    data._texture->LockRect( 0, &d3dlr, 0, D3DLOCK_DISCARD );

    const BYTE* srcPixels = static_cast<const BYTE*>(pImage->data());

    BYTE *dstData = static_cast<BYTE *>(d3dlr.pBits);
    LONG dstDataPitch = d3dlr.Pitch;

    if ( depth == 4u )
    {
      for ( unsigned y=0 ; y<height ; ++y )
      {
        BYTE *dstPixels = dstData + (y * dstDataPitch);

        for ( unsigned x=0 ; x<width ; ++x )
        {
          dstPixels[0] = srcPixels[2];
          dstPixels[1] = srcPixels[1];
          dstPixels[2] = srcPixels[0];
          dstPixels[3] = srcPixels[3];

          dstPixels += depth;
          srcPixels += depth;
        }
      }
    }
    else /* if ( depth == 3u ) */
    {
      for ( unsigned y=0 ; y<height ; ++y )
      {
        BYTE *dstPixels = dstData + (y * dstDataPitch);

        for ( unsigned x=0 ; x<width ; ++x )
        {
          dstPixels[0] = srcPixels[2];
          dstPixels[1] = srcPixels[1];
          dstPixels[2] = srcPixels[0];

          dstPixels += depth;
          srcPixels += depth;
        }
      }
    }

    data._texture->UnlockRect(0);

    //  Sprite sheet data has been loaded into graphics memory.  Free unecessary resources.
    pSpriteSheet->freeSpriteSheetData();
  }

  if ( data._texture != NULL )
  {
    _i->_device->SetTexture(TEXTURE_STAGE, data._texture);
  }
}

void RD_RendererDX9::endSpriteSheet()
{
  D3DPREAMBLE;

  _i->_device->SetTexture(TEXTURE_STAGE, 0);
}

void RD_RendererDX9::beginComposition( const RD_RenderScriptPlainPtr_t &pRenderScript )
{
  D3DPREAMBLE;

  //  No shader used by plain render script.
  RD_ShaderProgramDX9::setNullShader( (void*)_i->_device );
  _i->_activeProgram = NULL;

  _i->_device->SetFVF(RENDER_SCRIPT_FVF);

  RD_RenderScriptWeakPtr_t pWeakRenderScript(pRenderScript.get()); // for comparison.

  const RD_RenderScriptPlain::VertexData *vertices = pRenderScript->vertices();
  size_t verticesCount = pRenderScript->verticesCount();

  const RD_RenderScriptPlain::Index_t *indices = pRenderScript->indices();
  size_t indicesCount = pRenderScript->indicesCount();

  bool mapBuffers = pRenderScript->isRenderDirty();

  IDirect3DVertexBuffer9 *vertexBuffer = NULL;
  IDirect3DIndexBuffer9  *indexBuffer = NULL;

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
      //  Note.  We do not create DirectX device, so we better leave pool to D3DPOOL_DEFAULT as D3DPOOL_MANAGED might
      //  not be valid with device created.
			_i->_device->CreateVertexBuffer ( vertexBufferSize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &vertexBuffer, NULL);
      iBuffer->second._vertexBuffer = vertexBuffer;
      iBuffer->second._vertexBufferSize = vertexBufferSize;
    }

    indexBuffer = iBuffer->second._indexBuffer;

    //  Validate size of index buffer.
    if ( indexBuffer != NULL )
    {
      if ( iBuffer->second._indexBufferSize < indexBufferSize )
      {
        //  Recreate vertex buffer.
        SAFE_RELEASE(indexBuffer);
        indexBuffer = NULL;
      }
    }

    //  vertex buffer has been reset, recreate.
    if ( indexBuffer == NULL )
    {
      //  Note.  We do not create DirectX device, so we better leave pool to D3DPOOL_DEFAULT as D3DPOOL_MANAGED might
      //  not be valid with device created.
			_i->_device->CreateIndexBuffer ( indexBufferSize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &indexBuffer, NULL);
      iBuffer->second._indexBuffer = indexBuffer;
      iBuffer->second._indexBufferSize = indexBufferSize;
    }
  }
  else
  {
    Impl::BufferData data;

    _i->_device->CreateVertexBuffer ( vertexBufferSize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &data._vertexBuffer, NULL);
    data._vertexBufferSize = vertexBufferSize;

    vertexBuffer = data._vertexBuffer;

    _i->_device->CreateIndexBuffer ( indexBufferSize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &data._indexBuffer, NULL);
    data._indexBufferSize = indexBufferSize;

    indexBuffer = data._indexBuffer;

    _i->_buffers.insert( STD_MakePair( pWeakRenderScript, data ) );
  }

  if ( mapBuffers )
  {
    void *vbPtr;

    vertexBuffer->Lock (0, 0, &vbPtr, D3DLOCK_DISCARD);
    memcpy( vbPtr, vertices, vertexBufferSize );
    vertexBuffer->Unlock();

    void *idxPtr;

    indexBuffer->Lock (0, 0, &idxPtr, D3DLOCK_DISCARD);
    memcpy( idxPtr, indices, indexBufferSize );
    indexBuffer->Unlock();
  }

  _i->_device->SetStreamSource( 0, vertexBuffer, 0, sizeof(RD_RenderScriptPlain::VertexData) );
  _i->_device->SetIndices(indexBuffer);
}

void RD_RendererDX9::endComposition(const RD_RenderScriptPlainPtr_t &/*pRenderScript*/)
{
}

void RD_RendererDX9::beginComposition( const RD_RenderScriptFxPtr_t &pRenderScript )
{
  D3DPREAMBLE;

  //  Create or retrieve shader program.
  Impl::ShaderProgramCol_t::const_iterator iProgram = _i->_shaders.find( Impl::eFxShader );

  RD_ShaderProgramFxDX9 *program = static_cast<RD_ShaderProgramFxDX9*>(
      (iProgram == _i->_shaders.end()) ?
        _i->_shaders.insert( STD_MakePair( Impl::eFxShader, new RD_ShaderProgramFxDX9( (void*)_i->_device ) ) ).first->second :
        iProgram->second );

  //  Activate shader program if not already active.
  if ( _i->_activeProgram != program )
  {
    program->activate();
    _i->_activeProgram = program;
  }

  RD_RenderScriptWeakPtr_t pWeakRenderScript(pRenderScript.get()); // for comparison.

  const RD_RenderScriptFx::VertexData *vertices = pRenderScript->vertices();
  size_t verticesCount = pRenderScript->verticesCount();

  const RD_RenderScriptFx::Index_t *indices = pRenderScript->indices();
  size_t indicesCount = pRenderScript->indicesCount();

  bool mapBuffers = pRenderScript->isRenderDirty();

  IDirect3DVertexBuffer9 *vertexBuffer = NULL;
  IDirect3DIndexBuffer9  *indexBuffer = NULL;

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
      //  Note.  We do not create DirectX device, so we better leave pool to D3DPOOL_DEFAULT as D3DPOOL_MANAGED might
      //  not be valid with device created.
			_i->_device->CreateVertexBuffer ( vertexBufferSize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &vertexBuffer, NULL);
      iBuffer->second._vertexBuffer = vertexBuffer;
      iBuffer->second._vertexBufferSize = vertexBufferSize;
    }

    indexBuffer = iBuffer->second._indexBuffer;

    //  Validate size of index buffer.
    if ( indexBuffer != NULL )
    {
      if ( iBuffer->second._indexBufferSize < indexBufferSize )
      {
        //  Recreate vertex buffer.
        SAFE_RELEASE(indexBuffer);
        indexBuffer = NULL;
      }
    }

    //  vertex buffer has been reset, recreate.
    if ( indexBuffer == NULL )
    {
      //  Note.  We do not create DirectX device, so we better leave pool to D3DPOOL_DEFAULT as D3DPOOL_MANAGED might
      //  not be valid with device created.
			_i->_device->CreateIndexBuffer ( indexBufferSize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &indexBuffer, NULL);
      iBuffer->second._indexBuffer = indexBuffer;
      iBuffer->second._indexBufferSize = indexBufferSize;
    }
  }
  else
  {
    Impl::BufferData data;

    _i->_device->CreateVertexBuffer ( vertexBufferSize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &data._vertexBuffer, NULL);
    data._vertexBufferSize = vertexBufferSize;

    vertexBuffer = data._vertexBuffer;

    _i->_device->CreateIndexBuffer ( indexBufferSize, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &data._indexBuffer, NULL);
    data._indexBufferSize = indexBufferSize;

    indexBuffer = data._indexBuffer;

    _i->_buffers.insert( STD_MakePair( pWeakRenderScript, data ) );
  }

  if ( mapBuffers )
  {
    void *vbPtr;

    vertexBuffer->Lock (0, 0, &vbPtr, D3DLOCK_DISCARD);
    memcpy( vbPtr, vertices, vertexBufferSize );
    vertexBuffer->Unlock();

    void *idxPtr;

    indexBuffer->Lock (0, 0, &idxPtr, D3DLOCK_DISCARD);
    memcpy( idxPtr, indices, indexBufferSize );
    indexBuffer->Unlock();
  }

  _i->_device->SetStreamSource( 0, vertexBuffer, 0, sizeof(RD_RenderScriptFx::VertexData) );
  _i->_device->SetIndices(indexBuffer);
}

void RD_RendererDX9::endComposition(const RD_RenderScriptFxPtr_t &/*pRenderScript*/)
{
}

void RD_RendererDX9::beginBatch(const RD_RenderScriptFx::RenderBatch &renderBatch )
{
  RD_ShaderProgramFxDX9 *program = static_cast<RD_ShaderProgramFxDX9*>(_i->_activeProgram);

  unsigned boneCount = renderBatch._uniformBoneMatrices.size();
  unsigned boneCountGPU = (boneCount > MAX_BONES_GPU) ? MAX_BONES_GPU : boneCount;
  program->setBoneMatrices( (const float*)&renderBatch._uniformBoneMatrices[0], boneCountGPU );
}

void RD_RendererDX9::endBatch(const RD_RenderScriptFx::RenderBatch &/*renderBatch*/ )
{
}

void RD_RendererDX9::renderVertices( unsigned short * /*indices*/, unsigned indexOffset, unsigned nIndices, unsigned vertexOffset, unsigned nVertices )
{
  D3DPREAMBLE;

  if ( nIndices > 0u )
  {
    _i->_device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, vertexOffset, nVertices, indexOffset, nIndices/3 );
  }
}

void RD_RendererDX9::renderVertices( unsigned int * /*indices*/, unsigned indexOffset, unsigned nIndices, unsigned vertexOffset, unsigned nVertices )
{
  D3DPREAMBLE;

  if ( nIndices > 0u )
  {
    _i->_device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, vertexOffset, nVertices, indexOffset, nIndices/3 );
  }
}

#endif /* SUPPORT_RENDERER_DX9 */
