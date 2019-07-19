#include "RD_CameraEvent.h"

RD_CameraEvent::RD_CameraEvent() :
  _cullingMask(0)
{
}

RD_CameraEvent::RD_CameraEvent( int cullingMask, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &viewMatrix ) :
  _cullingMask(cullingMask),
  _projectionMatrix(projectionMatrix),
  _viewMatrix(viewMatrix)
{

}

int RD_CameraEvent::cullingMask() const
{
  return _cullingMask;
}

const Math::Matrix4x4 &RD_CameraEvent::projectionMatrix() const
{
  return _projectionMatrix;
}

const Math::Matrix4x4 &RD_CameraEvent::viewMatrix() const
{
  return _viewMatrix;
}
