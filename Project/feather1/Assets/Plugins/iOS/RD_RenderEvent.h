
#ifndef _RD_RENDER_EVENT_H_
#define _RD_RENDER_EVENT_H_

#include "MT_Matrix4x4.h"

/*!
 *  @class RD_RenderEvent
 *  Rendering of a render script transformed by a model matrix.
 */
class RD_RenderEvent
{
public:

  RD_RenderEvent();
  RD_RenderEvent(int scriptId, int layerIdx, const Math::Matrix4x4 &modelMatrix);

  //! Retrieve unique render script id.
  int scriptId() const;
  //! Retrieve layer mask.
  int layerMask() const;
  //! Retrieve model matrix.
  const Math::Matrix4x4 &modelMatrix() const;

private:

  int             _scriptId;
  int             _layerMask;
  Math::Matrix4x4 _modelMatrix;
};

#endif /* _RD_RENDER_EVENT_H_ */
