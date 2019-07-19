#ifndef _RD_RENDER_BATCH_H_
#define _RD_RENDER_BATCH_H_

namespace Math
{
  class Matrix4x4;
}

class RD_RenderEvent;
class RD_Renderer;

/*!
 *  @class RD_RenderEventBatch
 *  Collection of render events scheduled for rendering.
 */
class RD_RenderEventBatch
{
public:

  RD_RenderEventBatch();
  ~RD_RenderEventBatch();

  //! Add a render event to scheduled list.
  void addEvent( int frameCount, int depth, RD_RenderEvent *renderEvent );
  //! Clear scheduled list.
  void clear();

  //! Render scheduled events with specified projection and view matrices.
  //! A single event layer mask must be a subset of the culling mask in order to be rendered. 
  void render( RD_Renderer *renderer, int cullingMask, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &viewMatrix ) const;

private:

  void flushEvents();

private:

  //  not implemented.
  RD_RenderEventBatch( const RD_RenderEventBatch& );
  RD_RenderEventBatch &operator= ( const RD_RenderEventBatch& );

private:

  class Impl;
  Impl *_i;
};

#endif /* _RD_RENDER_BATCH_H_ */
