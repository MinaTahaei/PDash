
#include "RD_ShaderProgramDX11.h"

#if defined(SUPPORT_RENDERER_DX11)

#include "RD_RenderScriptPlain.h"
#include "RD_RenderScriptFx.h"

#include <d3d11.h>

#include <stdio.h>
#include <stdlib.h>

/*  Vertex/Fragment shaders */
//#include "Shaders/defaultVertShaderDX11.hlsl.h"
//#include "Shaders/defaultFragShaderDX11.hlsl.h"
//#include "Shaders/fxVertShaderDX11.hlsl.h"
//#include "Shaders/fxFragShaderDX11.hlsl.h"

#include "Shaders/defaultVertShaderDX11.cso.h"
#include "Shaders/defaultFragShaderDX11.cso.h"
#include "Shaders/fxVertShaderDX11.cso.h"
#include "Shaders/fxFragShaderDX11.cso.h"

#define ENABLE_DX_DEBUG

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
#define D3DPREAMBLE if( !_i->_device ) abort();
#endif

//#define LOGE(...) freopen( "log.txt", "w+", stderr ); fprintf( stderr, __VA_ARGS__); fflush(stderr)
#define LOGE(...) fprintf( stderr, __VA_ARGS__)


typedef HRESULT (WINAPI *D3DCompileFunc)(
	const void* pSrcData,
	unsigned long SrcDataSize,
	const char* pFileName,
	const D3D10_SHADER_MACRO* pDefines,
	ID3D10Include* pInclude,
	const char* pEntrypoint,
	const char* pTarget,
	unsigned int Flags1,
	unsigned int Flags2,
	ID3D10Blob** ppCode,
	ID3D10Blob** ppErrorMsgs);

namespace
{
  bool compileShader( D3DCompileFunc compileFunc, LPCSTR source, LPCSTR profile, ID3D10Blob **blob )
  {
    ID3D10Blob* errorBlob = NULL;

    HRESULT hr;
    hr = compileFunc(source, strlen(source), NULL, NULL, NULL, "main", profile, 0, 0, blob, &errorBlob);

#ifdef ENABLE_DX_DEBUG
    if ( D3DFAIL(hr) )
    {
      LOGE( "Failed to compile shader.\n" );
      if ( errorBlob != NULL )
      {
        LPVOID errorMsg = errorBlob->GetBufferPointer();
        LOGE( "%s\n", (const char*)errorMsg );
      }

      abort();
    }
#endif // ENABLE_DX_DEBUG

    SAFE_RELEASE(errorBlob);

    return D3DSUCCESS(hr);
  }

  void compileAndCreateShaders( ID3D11Device *device,
                                LPCSTR vertSource,
                                LPCSTR fragSource,
                                const D3D11_INPUT_ELEMENT_DESC *vertDesc,
                                UINT vertDescSize,
                                ID3D11VertexShader **vertShader,
                                ID3D11PixelShader **fragShader,
                                ID3D11InputLayout **vertLayout )
  {
    HMODULE compilerLib = LoadLibraryA("D3DCompiler_43.dll");

    if (compilerLib == NULL)
    {
      // Try compiler from Windows 8 SDK
      compilerLib = LoadLibraryA("D3DCompiler_46.dll");
    }

#ifdef ENABLE_DX_DEBUG
    if (compilerLib == NULL)
    {
      LOGE( "Cannot retrieve compiler library for shader.\n" );
      abort();
    }
#endif // ENABLE_DX_DEBUG

    if (compilerLib == NULL)
      return;

    ID3D10Blob* vertBlob = NULL;
		ID3D10Blob* fragBlob = NULL;

		D3DCompileFunc compileFunc = (D3DCompileFunc)GetProcAddress (compilerLib, "D3DCompile");

#ifdef ENABLE_DX_DEBUG
		if (compileFunc == NULL)
		{
      LOGE( "Cannot retrieve compiler function for shader.\n" );
      abort();
    }
#endif // ENABLE_DX_DEBUG

    if (compileFunc == NULL)
      return;

    if ( compileShader( compileFunc, vertSource, "vs_4_0", &vertBlob ) )
    {
      device->CreateVertexShader (vertBlob->GetBufferPointer(), vertBlob->GetBufferSize(), NULL, vertShader);
    }

    if ( compileShader( compileFunc, fragSource, "ps_4_0", &fragBlob ) )
    {
      device->CreatePixelShader (fragBlob->GetBufferPointer(), fragBlob->GetBufferSize(), NULL, fragShader);
    }

		// input layout
		if (vertShader && vertBlob)
		{
			device->CreateInputLayout (vertDesc, vertDescSize, vertBlob->GetBufferPointer(), vertBlob->GetBufferSize(), vertLayout);
		}

		SAFE_RELEASE(vertBlob);
		SAFE_RELEASE(fragBlob);

		FreeLibrary (compilerLib);
  }

  void createShaders( ID3D11Device *device,
                      const void *vertObject,
                      UINT vertObjectSize,
                      const void *fragObject,
                      UINT fragObjectSize,
                      const D3D11_INPUT_ELEMENT_DESC *vertDesc,
                      UINT vertDescSize,
                      ID3D11VertexShader **vertShader,
                      ID3D11PixelShader **fragShader,
                      ID3D11InputLayout **vertLayout )
  {
    HRESULT hr = device->CreateVertexShader (vertObject, vertObjectSize, NULL, vertShader);

#ifdef ENABLE_DX_DEBUG
    if ( D3DFAIL(hr ) )
    {
      LOGE( "Failed to create vertex shader.\n" );
      abort();
    }
#endif

    hr = device->CreatePixelShader (fragObject, fragObjectSize, NULL, fragShader);

#ifdef ENABLE_DX_DEBUG
    if ( D3DFAIL(hr) )
    {
      LOGE( "Failed to create fragment shader.\n" );
      abort();
    }
#endif

		// input layout
		if (vertShader)
		{
			hr = device->CreateInputLayout (vertDesc, vertDescSize, vertObject, vertObjectSize, vertLayout);

#ifdef ENABLE_DX_DEBUG
      if ( D3DFAIL(hr) )
      {
        LOGE( "Failed to create input layout for vertex shader.\n" );
        abort();
      }
#endif
		}
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramDX11
#endif

RD_ShaderProgramDX11::RD_ShaderProgramDX11()
{
}

RD_ShaderProgramDX11::~RD_ShaderProgramDX11()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramPlainDX11::Impl
#endif

class RD_ShaderProgramPlainDX11::Impl
{
  friend class RD_ShaderProgramPlainDX11;

public:

  /*!
   *  @struct VertexConstantDesc
   *  Constant parameters in vertex shader.
   */
  struct VertexConstantDesc
  {
    float _mvpMatrix[16];
    float _boneMatrices[512]; // 32 * 16
  };

public:

  Impl() :
    _device(NULL),
    _vertLayout(NULL),
    _vertShader(NULL),
    _fragShader(NULL),
    _vertConstant(NULL),
    _vertConstantDirty(true)
  {
  }

private:


  ID3D11Device                 *_device;
  ID3D11DeviceContext          *_context;

  ID3D11InputLayout            *_vertLayout;

  ID3D11VertexShader           *_vertShader;
  ID3D11PixelShader            *_fragShader;

  ID3D11Buffer                 *_vertConstant;
  VertexConstantDesc            _vertConstantDesc;
  bool                          _vertConstantDirty;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramPlainDX11
#endif

RD_ShaderProgramPlainDX11::RD_ShaderProgramPlainDX11( Ptr_t device )
{
  _i = new Impl;
  _i->_device = (ID3D11Device*)device;
}

RD_ShaderProgramPlainDX11::~RD_ShaderProgramPlainDX11()
{
  delete _i;
}

void RD_ShaderProgramPlainDX11::load()
{
  D3DPREAMBLE;

  if ( (_i->_vertShader != 0) ||
       (_i->_fragShader != 0) )
  {
    return;
  }

	D3D11_BUFFER_DESC desc;
	memset (&desc, 0, sizeof(desc));

	// Assign constant buffer.
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(Impl::VertexConstantDesc);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	_i->_device->CreateBuffer (&desc, NULL, &_i->_vertConstant);

  // vertex layout.
  D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof( RD_RenderScriptPlain::VertexData, _x ),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, offsetof( RD_RenderScriptPlain::VertexData, _color ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof( RD_RenderScriptPlain::VertexData, _u ),     D3D11_INPUT_PER_VERTEX_DATA, 0 }
  };

  // compile shaders.
  //compileAndCreateShaders( _i->_device,
  //                         g_defaultVertShaderDX11_hlsl,
  //                         g_defaultFragShaderDX11_hlsl,
  //                         vertexDesc,
  //                         sizeof(vertexDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
  //                         &_i->_vertShader,
  //                         &_i->_fragShader,
  //                         &_i->_vertLayout );

  createShaders( _i->_device,
                 (const void*)g_defaultVertShaderDX11_cso,
                 sizeof(g_defaultVertShaderDX11_cso),
                 (const void*)g_defaultFragShaderDX11_cso,
                 sizeof(g_defaultFragShaderDX11_cso),
                 vertexDesc,
                 sizeof(vertexDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
                 &_i->_vertShader,
                 &_i->_fragShader,
                 &_i->_vertLayout );
}

void RD_ShaderProgramPlainDX11::unload()
{
  SAFE_RELEASE(_i->_vertLayout);

  SAFE_RELEASE(_i->_vertShader);
  SAFE_RELEASE(_i->_fragShader);

  SAFE_RELEASE(_i->_vertConstant);
  //SAFE_RELEASE(_i->_fragConstant);
}

void RD_ShaderProgramPlainDX11::activate( Ptr_t context )
{
  D3DPREAMBLE;

  load();

  _i->_context = (ID3D11DeviceContext*)context;

  _i->_context->VSSetConstantBuffers (0, 1, &_i->_vertConstant);

  _i->_context->VSSetShader (_i->_vertShader, NULL, 0);
  _i->_context->PSSetShader (_i->_fragShader, NULL, 0);

  _i->_context->IASetInputLayout (_i->_vertLayout);

  _i->_vertConstantDirty = true;
}

void RD_ShaderProgramPlainDX11::deactivate()
{
  D3DPREAMBLE;

  if ( _i->_context == NULL )
    return;

  _i->_context->VSSetShader (NULL, NULL, 0);
  _i->_context->PSSetShader (NULL, NULL, 0);

  _i->_context = NULL;
}

void RD_ShaderProgramPlainDX11::setDevice( Ptr_t device )
{
  _i->_device = (ID3D11Device*)device;
}

void RD_ShaderProgramPlainDX11::updateConstants()
{
  if ( _i->_context == NULL )
    return;

  if ( _i->_vertConstantDirty )
  {
    _i->_context->UpdateSubresource(_i->_vertConstant, 0, NULL, (LPCVOID)&_i->_vertConstantDesc, sizeof(Impl::VertexConstantDesc), 0);
    _i->_vertConstantDirty = false;
  }
}

void RD_ShaderProgramPlainDX11::setMatrix( const float *matrix )
{
  memcpy( _i->_vertConstantDesc._mvpMatrix, matrix, sizeof(float)*16 );
  _i->_vertConstantDirty = true;
}

void RD_ShaderProgramPlainDX11::setTextureSampler0( Ptr_t sampler )
{
  if ( _i->_context == NULL )
    return;

  ID3D11ShaderResourceView *textureSampler = (ID3D11ShaderResourceView*)sampler;
  _i->_context->PSSetShaderResources(0, 1, &textureSampler);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramFxDX11::Impl
#endif

class RD_ShaderProgramFxDX11::Impl
{
  friend class RD_ShaderProgramFxDX11;

public:

  /*!
   *  @struct VertexConstantDesc
   *  Constant parameters in vertex shader.
   */
  struct VertexConstantDesc
  {
    float _mvpMatrix[16];
    float _boneMatrices[512]; // 32 * 16
  };

public:

  Impl() :
    _device(NULL),
    _vertLayout(NULL),
    _vertShader(NULL),
    _fragShader(NULL),
    _vertConstant(NULL),
    _vertConstantDirty(true)
  {
  }

private:


  ID3D11Device                 *_device;
  ID3D11DeviceContext          *_context;

  ID3D11InputLayout            *_vertLayout;

  ID3D11VertexShader           *_vertShader;
  ID3D11PixelShader            *_fragShader;

  ID3D11Buffer                 *_vertConstant;
  VertexConstantDesc            _vertConstantDesc;
  bool                          _vertConstantDirty;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramFxDX11
#endif

RD_ShaderProgramFxDX11::RD_ShaderProgramFxDX11( Ptr_t device )
{
  _i = new Impl;
  _i->_device = (ID3D11Device*)device;
}

RD_ShaderProgramFxDX11::~RD_ShaderProgramFxDX11()
{
  delete _i;
}

void RD_ShaderProgramFxDX11::load()
{
  D3DPREAMBLE;

  if ( (_i->_vertShader != 0) ||
       (_i->_fragShader != 0) )
  {
    return;
  }

	D3D11_BUFFER_DESC desc;
	memset (&desc, 0, sizeof(desc));

	// Assign constant buffer.
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(Impl::VertexConstantDesc);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	_i->_device->CreateBuffer (&desc, NULL, &_i->_vertConstant);

  // vertex layout.
  D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof( RD_RenderScriptFx::VertexData, _x ),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, offsetof( RD_RenderScriptFx::VertexData, _color ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof( RD_RenderScriptFx::VertexData, _u0 ),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof( RD_RenderScriptFx::VertexData, _fxParams0[0] ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof( RD_RenderScriptFx::VertexData, _fxViewport0[0] ), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof( RD_RenderScriptFx::VertexData, _boneParams[0] ), D3D11_INPUT_PER_VERTEX_DATA, 0 }
  };

  // compile shaders.
  //compileAndCreateShaders( _i->_device,
  //                         g_fxVertShaderDX11_hlsl,
  //                         g_fxFragShaderDX11_hlsl,
  //                         vertexDesc,
  //                         sizeof(vertexDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
  //                         &_i->_vertShader,
  //                         &_i->_fragShader,
  //                         &_i->_vertLayout );

  createShaders( _i->_device,
                 (const void*)g_fxVertShaderDX11_cso,
                 sizeof(g_fxVertShaderDX11_cso),
                 (const void*)g_fxFragShaderDX11_cso,
                 sizeof(g_fxFragShaderDX11_cso),
                 vertexDesc,
                 sizeof(vertexDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
                 &_i->_vertShader,
                 &_i->_fragShader,
                 &_i->_vertLayout );


}

void RD_ShaderProgramFxDX11::unload()
{
  SAFE_RELEASE(_i->_vertLayout);

  SAFE_RELEASE(_i->_vertShader);
  SAFE_RELEASE(_i->_fragShader);

  SAFE_RELEASE(_i->_vertConstant);
  //SAFE_RELEASE(_i->_fragConstant);
}

void RD_ShaderProgramFxDX11::activate( Ptr_t context )
{
  D3DPREAMBLE;

  load();

  _i->_context = (ID3D11DeviceContext*)context;

  _i->_context->VSSetConstantBuffers (0, 1, &_i->_vertConstant);

  _i->_context->VSSetShader (_i->_vertShader, NULL, 0);
  _i->_context->PSSetShader (_i->_fragShader, NULL, 0);

  _i->_context->IASetInputLayout (_i->_vertLayout);

  _i->_vertConstantDirty = true;
}

void RD_ShaderProgramFxDX11::deactivate()
{
  D3DPREAMBLE;

  if ( _i->_context == NULL )
    return;

  _i->_context->VSSetShader (NULL, NULL, 0);
  _i->_context->PSSetShader (NULL, NULL, 0);

  _i->_context = NULL;
}

void RD_ShaderProgramFxDX11::setDevice( Ptr_t device )
{
  _i->_device = (ID3D11Device*)device;
}

void RD_ShaderProgramFxDX11::updateConstants()
{
  if ( _i->_context == NULL )
    return;

  if ( _i->_vertConstantDirty )
  {
    _i->_context->UpdateSubresource(_i->_vertConstant, 0, NULL, (LPCVOID)&_i->_vertConstantDesc, sizeof(Impl::VertexConstantDesc), 0);
    _i->_vertConstantDirty = false;
  }
}

void RD_ShaderProgramFxDX11::setMatrix( const float *matrix )
{
  memcpy( _i->_vertConstantDesc._mvpMatrix, matrix, sizeof(float)*16 );
  _i->_vertConstantDirty = true;
}

void RD_ShaderProgramFxDX11::setTextureSampler0( Ptr_t sampler )
{
  if ( _i->_context == NULL )
    return;

  ID3D11ShaderResourceView *textureSampler = (ID3D11ShaderResourceView*)sampler;
  _i->_context->PSSetShaderResources(0, 1, &textureSampler);
}

void RD_ShaderProgramFxDX11::setBoneMatrices( const float *matrixArray, unsigned count )
{
  if ( _i->_context == NULL )
    return;

  memcpy( _i->_vertConstantDesc._boneMatrices, matrixArray, sizeof(float)*16*count );
}


#endif /* SUPPORT_RENDERER_DX11 */

