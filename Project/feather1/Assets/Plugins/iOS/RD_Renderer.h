
#ifndef _RD_RENDERER_H_
#define _RD_RENDERER_H_

#include "RD_SpriteSheet.h"
#include "RD_RenderScript.h"

#include "RD_RenderScriptPlain.h"
#include "RD_RenderScriptFx.h"

/*!
 *  @class RD_Renderer
 *  Multi-plaform rendering class.
 */
class RD_Renderer
{
public:

  typedef void* TexturePtr_t;

  struct Rect
  {
    float _offsetx, _offsety;
    float _width, _height;
  };

public:

  RD_Renderer();
  virtual ~RD_Renderer();

  //! Initialization.
  virtual void initialize() = 0;
  //! Final clenaup.
  virtual void shutdown() = 0;

  //! Synchronize renderer with current device.
  virtual void setDevice( void *device, int eventType ) = 0;

  //! Free unused resources.
  virtual void cleanup() = 0;

  //! Start render.
  virtual void beginRender() = 0;
  //! End render.
  virtual void endRender() = 0;

  //! Update current matrices.  Must be set inside a composition group. 
  virtual void setMatrices( const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix ) = 0;

  //! Bind specified sprite sheet.  Must be set inside a composition group.
  virtual void beginSpriteSheet( const RD_SpriteSheetPtr_t &spriteSheet ) = 0;
  //! Unbind sprite sheet.
  virtual void endSpriteSheet() = 0;

  //! Bind render script resources.  Must be set inside a render group.
  virtual void beginComposition( const RD_RenderScriptPlainPtr_t &renderScript ) = 0;
  //! Unbind render script resources.
  virtual void endComposition( const RD_RenderScriptPlainPtr_t &renderScript ) = 0;

  //! Bind render script resources.  Must be set inside a render group.
  virtual void beginComposition( const RD_RenderScriptFxPtr_t &renderScript ) = 0;
  //! Unbind render script resources.
  virtual void endComposition( const RD_RenderScriptFxPtr_t &renderScript ) = 0;

  //! Bind render batch uniform parameters.  Must be set inside a composition group.
  virtual void beginBatch( const RD_RenderScriptFx::RenderBatch &renderBatch ) = 0;
  //! Unbind render batch.
  virtual void endBatch( const RD_RenderScriptFx::RenderBatch &renderBatch ) = 0;

  //! Render script to offline texture.  Must be called inside a render group.
  virtual void renderScriptToTexture( const RD_RenderScriptPtr_t &pRenderScript, const Rect &rect, float scale, TexturePtr_t texture ) = 0;
  //! Render script.  Must be called inside a render group.
  virtual void renderScript( const RD_RenderScriptPtr_t &pRenderScript, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix ) = 0;

  //! Render vertices.  Must be called inside a composition group.
  virtual void renderVertices( unsigned short *indices, unsigned indexOffset, unsigned nIndices, unsigned vertexOffset, unsigned nVertices ) = 0;
  //! Render vertices.  Must be called inside a composition group.
  virtual void renderVertices( unsigned int   *indices, unsigned indexOffset, unsigned nIndices, unsigned vertexOffset, unsigned nVertices ) = 0;

private:
};



#endif /* _RD_RENDERER_H_ */
