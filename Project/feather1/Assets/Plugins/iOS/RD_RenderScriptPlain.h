
#ifndef _RD_RENDER_SCRIPT_PLAIN_H_
#define _RD_RENDER_SCRIPT_PLAIN_H_

#include "RD_RenderScript.h"
#include "RD_SpriteSheet.h"

#include "UT_SharedWeakPtr.h"
#include "STD_Types.h"
#include "STD_Containers.h"

class RD_Renderer;
class RD_RenderScriptPlain;

typedef UT_SharedPtr<RD_RenderScriptPlain> RD_RenderScriptPlainPtr_t;
typedef UT_SharedWeakPtr<RD_RenderScriptPlain> RD_RenderScriptPlainWeakPtr_t;

/*!
 *  @class RD_RenderScriptPlain
 *  Rendering data for a single clip.
 */
class RD_RenderScriptPlain : public RD_RenderScript
{
public:

  struct VertexData
  {
    VertexData(float x, float y, float u, float v, unsigned int color) :
      _x(x),
      _y(y),
      _z(0.0f),
      _color(color),
      _u(u),
      _v(v)
    {
    }

    float _x, _y, _z;
    unsigned int _color;
    float _u, _v;
  };

  typedef STD_Vector<VertexData> VertexDataCol_t;

  typedef unsigned short Index_t;
  typedef Index_t IndexPtr_t;

  typedef STD_Vector<Index_t> IndexDataCol_t;

  struct RenderBatch
  {
    RD_SpriteSheetPtr_t _spriteSheet;
    unsigned            _indexOffset;
    unsigned            _nIndices;
    unsigned            _vertexOffset;
    unsigned            _nVertices;
  };

  typedef STD_Vector< RenderBatch > RenderBatchCol_t;

public:

  RD_RenderScriptPlain();
  RD_RenderScriptPlain(const RD_RenderScriptMetaPtr_t &);
  virtual ~RD_RenderScriptPlain();

  virtual void update( const RD_ClipDataPtr_t &pClipData, const STD_String &projectFolder, const STD_String &sheetResolution, float frame, unsigned int color, int discretizationStep );
  virtual void updateWithBlending( const RD_ClipDataPtr_t &pClipDataFrom, const STD_String &projectFolder, const STD_String &sheetResolution, float frameFrom, float frameTo, unsigned int color, int discretizationStep , float fullBlendTime, float currentBlendTime,int blendID );
  virtual void render( RD_Renderer *renderer, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix );

  virtual void calculateBoundingBox( BoundingBox &box );
  virtual void calculateConvexHull( BoundingVertex* &convexHullArray, int &convexHullSize );
  virtual void calculatePolygons( BoundingVertex* &polygonArray, int &polygonSize, int* &subPolygonArray, int &subPolygonSize );

  virtual bool supportsFeature( Feature feature ) const;

  static Feature supportedFeatures();

public:

  bool isDirty( float frame ) const;
  bool isRenderDirty() const;

  const VertexData *vertices() const;
  size_t verticesCount() const;

  const Index_t *indices() const;
  size_t indicesCount() const;

protected:

  void addBatch( const RD_Composition *composition,
                 const RD_Composition::CompositionNodeCol_t::const_iterator iStart,
                 const RD_Composition::CompositionNodeCol_t::const_iterator iEnd,
                 unsigned int color );

private:

  class Impl;
  Impl *_i;

};

#endif /* _RD_RENDER_SCRIPT_PLAIN_H_ */
