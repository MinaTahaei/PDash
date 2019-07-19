
#include "RD_RenderEvent.h"

RD_RenderEvent::RD_RenderEvent() :
  _scriptId(-1),
  _layerMask(0)
{
}

RD_RenderEvent::RD_RenderEvent(int scriptId, int layerMask, const Math::Matrix4x4 &modelMatrix) :
  _scriptId(scriptId),
  _layerMask(layerMask),
  _modelMatrix(modelMatrix)
{

}

int RD_RenderEvent::scriptId() const
{
  return _scriptId;
}

int RD_RenderEvent::layerMask() const
{
  return _layerMask;
}

const Math::Matrix4x4 &RD_RenderEvent::modelMatrix() const
{
  return _modelMatrix;
}

