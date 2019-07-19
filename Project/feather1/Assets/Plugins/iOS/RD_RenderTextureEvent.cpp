#include "RD_RenderTextureEvent.h"
#include "RD_Renderer.h"

RD_RenderTextureEvent::RD_RenderTextureEvent() :
  _scriptId(-1),
  _scale(1.0f),
  _texture(0)
{
  _rect._offsetx = _rect._offsety = _rect._width = _rect._height = 0;
}

RD_RenderTextureEvent::RD_RenderTextureEvent( int scriptId, float offsetx, float offsety, float width, float height, float scale, TexturePtr_t texture ) :
  _scriptId(scriptId),
  _scale(scale),
  _texture(texture)
{
  _rect._offsetx = offsetx;
  _rect._offsety = offsety;
  _rect._width = width;
  _rect._height = height;
}

void RD_RenderTextureEvent::render( RD_Renderer *renderer )
{
  RD_RenderScriptPtr_t pScript = RD_RenderScriptManager::instance()->object( _scriptId );
  if ( pScript.isValid() )
  {
    renderer->renderScriptToTexture( pScript, _rect, _scale, _texture );
  }
}

