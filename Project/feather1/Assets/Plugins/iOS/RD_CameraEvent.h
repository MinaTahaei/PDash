
#ifndef _RD_CAMERA_EVENT_H_
#define _RD_CAMERA_EVENT_H_

#include "RD_RenderObjectManager.h"

#include "UT_ShareBaseSafe.h"
#include "UT_SharedPtr.h"
#include "MT_Matrix4x4.h"

class RD_CameraEvent;
typedef UT_SharedPtr< RD_CameraEvent > RD_CameraEventPtr_t;

//  Thread-safe collection.
typedef RD_RenderObjectManager<int, RD_CameraEvent> RD_CameraEventManager;

/*!
 *  @class RD_CameraEvent
 *  Camera parameters to render a batch of render events with.
 */
class RD_CameraEvent : public UT_ShareBaseSafe
{
public:

  RD_CameraEvent();
  RD_CameraEvent( int cullingMask, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &viewMatrix );

  int cullingMask() const;

  const Math::Matrix4x4 &projectionMatrix() const;
  const Math::Matrix4x4 &viewMatrix() const;

private:

  int             _cullingMask;

  Math::Matrix4x4 _projectionMatrix;
  Math::Matrix4x4 _viewMatrix;
};

#endif /* _RD_CAMERA_EVENT_H_ */
