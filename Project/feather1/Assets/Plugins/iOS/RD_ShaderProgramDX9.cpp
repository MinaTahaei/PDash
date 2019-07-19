
#include "RD_ShaderProgramDX9.h"

#if defined(SUPPORT_RENDERER_DX9)

#include "RD_RenderScriptFx.h"

#include <d3d9.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <vector>

/*  Vertex/Fragment shaders */
//#include "Shaders/fxVertShaderDX9.hlsl.h"
//#include "Shaders/fxFragShaderDX9.hlsl.h"
#include "Shaders/fxVertShaderDX9.cso.h"
#include "Shaders/fxFragShaderDX9.cso.h"

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
#define D3DPREAMBLE if( !_i->_device ) return;
#endif

//#define LOGE(...) freopen( "log.txt", "w+", stderr ); fprintf( stderr, __VA_ARGS__); fflush(stderr)
#define LOGE(...) fprintf( stderr, __VA_ARGS__)

#define CONSTANT_MVP_MATRIX_REGISTER_INDEX        0
#define CONSTANT_BONE_MATRIX_REGISTER_INDEX       4

typedef HRESULT (WINAPI *D3DCompileFunc)(
	const void* pSrcData,
	unsigned long SrcDataSize,
	const char* pFileName,
	const D3D_SHADER_MACRO* pDefines,
	ID3DInclude* pInclude,
	const char* pEntrypoint,
	const char* pTarget,
	unsigned int Flags1,
	unsigned int Flags2,
	ID3DBlob** ppCode,
	ID3DBlob** ppErrorMsgs);

namespace
{
  void compileShader( D3DCompileFunc compileFunc, LPCSTR source, LPCSTR profile, ID3DBlob **shaderBuffer )
  {
    if (!source)
      return;

    HRESULT hr;

    ID3DBlob *errorBuffer = NULL;

    //  Compile to vertex shader.
    hr = compileFunc(
        source,
        (UINT)strlen(source),
        NULL,
        NULL,
        NULL,
        "main",
        profile,
#ifdef ENABLE_DX_DEBUG
        D3DCOMPILE_DEBUG,
#else
        0,
#endif
        0,
        shaderBuffer,
        &errorBuffer ); // error messages 

#ifdef ENABLE_DX_DEBUG
    if ( D3DFAIL(hr) )
    {
      LOGE( "Failed to compile shader.\n" );
      if ( errorBuffer != NULL )
      {
        LPVOID errorMsg = errorBuffer->GetBufferPointer();
        LOGE( "%s\n", (const char*)errorMsg );
      }

      abort();
    }
#endif // ENABLE_DX_DEBUG

    SAFE_RELEASE(errorBuffer);
  }

  void createVertexShader( IDirect3DDevice9 *device, const DWORD *pFunction, IDirect3DVertexShader9 **shader )
  {
    //  Create the vertex shader
    HRESULT hr = device->CreateVertexShader( pFunction, shader );

#ifdef ENABLE_DX_DEBUG
    if ( D3DFAIL(hr) )
    {
      LOGE( "Failed to create shader.\n" );
      abort();
    }
#endif // ENABLE_DX_DEBUG
  }

  void createFragmentShader( IDirect3DDevice9 *device, const DWORD *pFunction, IDirect3DPixelShader9 **shader )
  {
    //LPCSTR pixelProfile = D3DXGetPixelShaderProfile( device );

    //  Create the pixel shader
    HRESULT hr = device->CreatePixelShader( pFunction, shader );

#ifdef ENABLE_DX_DEBUG
    if ( D3DFAIL(hr) )
    {
      LOGE( "Failed to create shader.\n" );
      abort();
    }
#endif // ENABLE_DX_DEBUG
  }

  void compileAndCreateVertexShader( IDirect3DDevice9 *device, D3DCompileFunc compileFunc, LPCSTR source, IDirect3DVertexShader9 **shader )
  {
    //LPCSTR vertexProfile = D3DXGetVertexShaderProfile( device );

    ID3DBlob *shaderBuffer = NULL;
    compileShader( compileFunc, source, "vs_2_0", &shaderBuffer );
    createVertexShader( device, (DWORD*)shaderBuffer->GetBufferPointer(), shader );

    SAFE_RELEASE(shaderBuffer);
  }

  void compileAndCreateFragmentShader( IDirect3DDevice9 *device, D3DCompileFunc compileFunc, LPCSTR source, IDirect3DPixelShader9 **shader )
  {
    //LPCSTR pixelProfile = D3DXGetPixelShaderProfile( device );

    ID3DBlob *shaderBuffer = NULL;
    compileShader( compileFunc, source, "ps_2_0", &shaderBuffer );
    createFragmentShader( device, (DWORD*)shaderBuffer->GetBufferPointer(), shader );

    SAFE_RELEASE(shaderBuffer);
  }

  void compileAndCreateShaders( IDirect3DDevice9 *device,
                                LPCSTR vertSource,
                                LPCSTR fragSource,
                                IDirect3DVertexShader9 **vertShader,
                                IDirect3DPixelShader9 **fragShader )
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

    compileAndCreateVertexShader( device, compileFunc, vertSource, vertShader );
    compileAndCreateFragmentShader( device, compileFunc, fragSource, fragShader );

		FreeLibrary (compilerLib);
  }

  void createShaders( IDirect3DDevice9 *device,
                      const DWORD *vertObject,
                      const DWORD *fragObject,
                      IDirect3DVertexShader9 **vertShader,
                      IDirect3DPixelShader9 **fragShader )
  {
    createVertexShader( device, vertObject, vertShader );
    createFragmentShader( device, fragObject, fragShader );
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramDX9
#endif

RD_ShaderProgramDX9::RD_ShaderProgramDX9()
{
}

RD_ShaderProgramDX9::~RD_ShaderProgramDX9()
{
}

void RD_ShaderProgramDX9::setNullShader( void *device )
{
  IDirect3DDevice9 *d3dDevice = (IDirect3DDevice9*)device;

  d3dDevice->SetVertexDeclaration( NULL );

  d3dDevice->SetVertexShader( NULL );
  d3dDevice->SetPixelShader( NULL );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramFxDX9::Impl
#endif

class RD_ShaderProgramFxDX9::Impl
{
  friend class RD_ShaderProgramFxDX9;

public:

  Impl() :
    _device(NULL),
    _vertDeclaration(NULL),
    _vertShader(NULL),
    _fragShader(NULL)
  {
  }

private:

  IDirect3DDevice9             *_device;

  IDirect3DVertexDeclaration9  *_vertDeclaration;

  IDirect3DVertexShader9       *_vertShader;
  IDirect3DPixelShader9        *_fragShader;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_ShaderProgramFxDX9
#endif

RD_ShaderProgramFxDX9::RD_ShaderProgramFxDX9( void *device )
{
  _i = new Impl;
  _i->_device = (IDirect3DDevice9*)device;
}

RD_ShaderProgramFxDX9::~RD_ShaderProgramFxDX9()
{
  delete _i;
}

void RD_ShaderProgramFxDX9::load()
{
  D3DPREAMBLE;

  if ( (_i->_vertShader != 0) ||
       (_i->_fragShader != 0) )
  {
    return;
  }

  //compileAndCreateShaders( _i->_device,
  //                         g_fxVertShaderDX9_hlsl,
  //                         g_fxFragShaderDX9_hlsl,
  //                         &_i->_vertShader,
  //                         &_i->_fragShader );

  createShaders( _i->_device,
                 (const DWORD*)g_fxVertShaderDX9_cso,
                 (const DWORD*)g_fxFragShaderDX9_cso,
                 &_i->_vertShader,
                 &_i->_fragShader );

  D3DVERTEXELEMENT9 declaration[] =
  {
    { 0, offsetof( RD_RenderScriptFx::VertexData, _x ),              D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, offsetof( RD_RenderScriptFx::VertexData, _color ),          D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
    { 0, offsetof( RD_RenderScriptFx::VertexData, _u0 ),             D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    { 0, offsetof( RD_RenderScriptFx::VertexData, _fxParams0[0] ),   D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
    { 0, offsetof( RD_RenderScriptFx::VertexData, _fxViewport0[0] ), D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2 },
    { 0, offsetof( RD_RenderScriptFx::VertexData, _boneParams[0] ),  D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 },
    D3DDECL_END()
  };

  _i->_device->CreateVertexDeclaration( declaration, &_i->_vertDeclaration );

}

void RD_ShaderProgramFxDX9::unload()
{
  SAFE_RELEASE(_i->_vertDeclaration);

  SAFE_RELEASE(_i->_vertShader);
  SAFE_RELEASE(_i->_fragShader);
}

void RD_ShaderProgramFxDX9::activate()
{
  D3DPREAMBLE;

  load();

  _i->_device->SetVertexDeclaration( _i->_vertDeclaration );

  _i->_device->SetVertexShader( _i->_vertShader );
  _i->_device->SetPixelShader( _i->_fragShader );
}

void RD_ShaderProgramFxDX9::deactivate()
{
  D3DPREAMBLE;

  _i->_device->SetVertexDeclaration( NULL );

  _i->_device->SetVertexShader( NULL );
  _i->_device->SetPixelShader( NULL );
}

void RD_ShaderProgramFxDX9::setDevice( void *device )
{
  _i->_device = (IDirect3DDevice9*)device;
}

void RD_ShaderProgramFxDX9::setMatrix( const float *matrix )
{
  D3DPREAMBLE;
  _i->_device->SetVertexShaderConstantF( CONSTANT_MVP_MATRIX_REGISTER_INDEX, matrix, 4 );
}

void RD_ShaderProgramFxDX9::setBoneMatrices( const float *matrixArray, unsigned count )
{
  D3DPREAMBLE;
  _i->_device->SetVertexShaderConstantF( CONSTANT_BONE_MATRIX_REGISTER_INDEX, matrixArray, 4 * count );
}

#endif /* SUPPORT_RENDERER_DX9 */

