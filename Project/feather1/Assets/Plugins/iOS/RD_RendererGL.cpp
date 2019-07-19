
#include "RD_RendererGL.h"

#ifdef SUPPORT_RENDERER_GL

#include "STD_Containers.h"

#if defined(TARGET_MAC)
  #include <OpenGL/gl.h>
#else
  #define ENABLE_GLEW
  #include <GL/glew.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RendererGL::Impl
#endif

class RD_RendererGLLegacy::Impl
{
  friend class RD_RendererGLLegacy;

public:
  Impl()
  {
  }

  ~Impl()
  {
  }

private:

  typedef STD_Map< RD_SpriteSheetWeakPtr_t, GLuint > TextureIdCol_t;
  TextureIdCol_t _textureIds;

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RendererGL
#endif

RD_RendererGLLegacy::RD_RendererGLLegacy()
{
  _i = new Impl;
}

RD_RendererGLLegacy::~RD_RendererGLLegacy()
{
  delete _i;
}

void RD_RendererGLLegacy::initialize()
{
#if defined(ENABLE_GLEW)
  glewInit();
#endif // ENABLE_GLEW
}

void RD_RendererGLLegacy::shutdown()
{
  //  Nothing to do!
}

void RD_RendererGLLegacy::setDevice( void * /*device*/, int /*eventType*/ )
{
  //  Nothing to do!
}

void RD_RendererGLLegacy::cleanup()
{
  //  Clear unused texture ids.
  for ( Impl::TextureIdCol_t::iterator i = _i->_textureIds.begin(), iEnd = _i->_textureIds.end() ; i!=iEnd ; )
  {
    if ( !i->first.isValid() )
    {
      glDeleteTextures( 1, &i->second );
      _i->_textureIds.erase( i++ );
    }
    else
    {
      ++i;
    }
  }
}

void RD_RendererGLLegacy::beginRender()
{
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  glDisable(GL_ALPHA_TEST);

  //  Make sure active texture is 1st one.
  glActiveTextureARB(GL_TEXTURE0_ARB);

  //  Using premultiplied textures.
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  //  Clear unused data.
  cleanup();
}

void RD_RendererGLLegacy::endRender()
{
}

void RD_RendererGLLegacy::setMatrices( const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix )
{
  glMatrixMode( GL_PROJECTION_MATRIX );
  glLoadMatrixd( projectionMatrix.asArray() );

  glMatrixMode( GL_MODELVIEW_MATRIX );
  glLoadMatrixd( modelViewMatrix.asArray() );
}

void RD_RendererGLLegacy::beginSpriteSheet( const RD_SpriteSheetPtr_t &pSpriteSheet )
{
  glEnable(GL_TEXTURE_2D);

  RD_SpriteSheetWeakPtr_t pWeakSpriteSheet(pSpriteSheet); // for comparison.
  Impl::TextureIdCol_t::const_iterator iTextureId = _i->_textureIds.find( pWeakSpriteSheet );

  if ( iTextureId == _i->_textureIds.end() )
  {
    //  Create texture id using sprite sheet image.
    IM_ImagePtr_t pImage = pSpriteSheet->image();
    if ( pImage.isValid() )
    {
      GLuint textureId;

      glGenTextures( 1, &textureId );
      glBindTexture( GL_TEXTURE_2D, textureId );

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

      unsigned depth = pImage->depth();

      if ( depth == 4u )
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, pImage->width(), pImage->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage->data() );
      else if ( depth == 3u )
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, pImage->width(), pImage->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, pImage->data() );

      //  Save in texture id collection
      _i->_textureIds.insert( STD_MakePair( pWeakSpriteSheet, textureId ) );

      //  Sprite sheet data has been loaded into GL.  Free unecessary resources.
      pSpriteSheet->freeSpriteSheetData();
    }
  }
  else
  {
    //  Reuse texture id.
    glBindTexture( GL_TEXTURE_2D, iTextureId->second );
  }
}

void RD_RendererGLLegacy::endSpriteSheet()
{
  glDisable( GL_TEXTURE_2D );
}

void RD_RendererGLLegacy::beginComposition( const RD_RenderScriptPlainPtr_t &pRenderScript )
{
  const RD_RenderScriptPlain::VertexData *vertices = pRenderScript->vertices();
  //size_t verticesCount = pRenderScript->verticesCount();

  glVertexPointer(2, GL_FLOAT, sizeof(RD_RenderScriptPlain::VertexData), &vertices->_x);
  glEnableClientState(GL_VERTEX_ARRAY);

  glTexCoordPointer(2, GL_FLOAT, sizeof(RD_RenderScriptPlain::VertexData), &vertices->_u);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(RD_RenderScriptPlain::VertexData), &vertices->_color);
  glEnableClientState(GL_COLOR_ARRAY);
}

void RD_RendererGLLegacy::endComposition( const RD_RenderScriptPlainPtr_t &/*pRenderScript*/ )
{
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
}

void RD_RendererGLLegacy::beginComposition( const RD_RenderScriptFxPtr_t &pRenderScript )
{
  //  This version of the renderer does not use shaders, so effects contained
  //  in this render script will be discarded.

  const RD_RenderScriptFx::VertexData *vertices = pRenderScript->vertices();
  //size_t verticesCount = pRenderScript->verticesCount();

  glVertexPointer(2, GL_FLOAT, sizeof(RD_RenderScriptFx::VertexData), &vertices->_x);
  glEnableClientState(GL_VERTEX_ARRAY);

  glTexCoordPointer(2, GL_FLOAT, sizeof(RD_RenderScriptFx::VertexData), &vertices->_u0);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(RD_RenderScriptFx::VertexData), &vertices->_color);
  glEnableClientState(GL_COLOR_ARRAY);
}

void RD_RendererGLLegacy::endComposition( const RD_RenderScriptFxPtr_t &/*pRenderScript*/ )
{
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
}

void RD_RendererGLLegacy::beginBatch( const RD_RenderScriptFx::RenderBatch &/*renderBatch*/ )
{
  //  Nothing to do in legacy opengl renderer!
}

void RD_RendererGLLegacy::endBatch( const RD_RenderScriptFx::RenderBatch &/*renderBatch*/ )
{
  //  Nothing to do in legacy opengl renderer!
}

void RD_RendererGLLegacy::renderScriptToTexture( const RD_RenderScriptPtr_t& /*pRenderScript*/, const Rect& /*rect*/, float /*scale*/, TexturePtr_t /*texture*/ )
{
  //  Not supported by legacy opengl renderer!
}

void RD_RendererGLLegacy::renderScript( const RD_RenderScriptPtr_t &pRenderScript, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix )
{
  pRenderScript->render( this, projectionMatrix, modelViewMatrix );
}

void RD_RendererGLLegacy::renderVertices( unsigned short *indices, unsigned indexOffset, unsigned nIndices, unsigned /*vertexOffset*/, unsigned /*nVertices*/ )
{
  if ( nIndices > 0u )
  {
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, (const GLvoid*)(indices + indexOffset * sizeof(unsigned short)) );
  }
}

void RD_RendererGLLegacy::renderVertices( unsigned int *indices, unsigned indexOffset, unsigned nIndices, unsigned /*vertexOffset*/, unsigned /*nVertices*/ )
{
  if ( nIndices > 0u )
  {
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, (const GLvoid*)(indices + indexOffset * sizeof(unsigned int)) );
  }
}

#endif /* SUPPORT_RENDERER_GL */
