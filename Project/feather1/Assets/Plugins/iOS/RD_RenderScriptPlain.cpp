
#include "RD_RenderScriptPlain.h"
#include "RD_Renderer.h"

#include "STD_Types.h"
#include "STD_Containers.h"

#include "clipper.h"

#define INIT_BOUND( val, min_bound, max_bound ) \
  min_bound = val; \
  max_bound = val;

#define SET_BOUND( val, min_bound, max_bound ) \
  if ( val < min_bound ) \
    min_bound = val; \
  else if ( val > max_bound ) \
    max_bound = val;

#define VEC_DET( v1x, v1y, v2x, v2y ) \
  v1x * v2y - v1y * v2x

#define ROUND_VALUE( val ) \
  floorf( (val * 64.0f) + 0.5f ) / 64.0f;

#define WORLD_TO_CLIP( val ) \
  (int)(val * 1024.0f)

#define CLIP_TO_WORLD( val ) \
  (val / 1024.0f)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RenderScriptPlain::Impl
#endif

class RD_RenderScriptPlain::Impl
{
  friend class RD_RenderScriptPlain;

public:

  Impl() :
    _frame(0.0f),
    _renderDirty(true)
  {
  }

  ~Impl()
  {
  }

private:

  VertexDataCol_t         _vertices;
  IndexDataCol_t          _indices;

  RD_ClipDataPtr_t        _clipData;

  STD_String              _sheetResolution;

  RenderBatchCol_t        _batches;

  float                   _frame;
  bool                    _renderDirty;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RenderScriptPlain
#endif

RD_RenderScriptPlain::RD_RenderScriptPlain()
{
  _i = new Impl;
}

RD_RenderScriptPlain::RD_RenderScriptPlain(const RD_RenderScriptMetaPtr_t &meta) :
  RD_RenderScript(meta)
{
  _i = new Impl;
}

RD_RenderScriptPlain::~RD_RenderScriptPlain()
{
  cleanup();
  delete _i;
}

void RD_RenderScriptPlain::update( const RD_ClipDataPtr_t &pClipData, const STD_String &projectFolder, const STD_String &sheetResolution, float frame, unsigned int color, int /*discretizationStep*/ )
{
  lock();

  _i->_vertices.clear();
  _i->_indices.clear();
  _i->_batches.clear();

  //  Render now set to frame.
  _i->_frame = frame;

  //  Clip data has changed between two renders.  Replace hierarchy with new clip data.
  if ( pClipData != _i->_clipData )
  {
    //  Reset composition and replace with new hiearchy.
    _i->_clipData = pClipData;
    cleanup();
  }

  //  sheet resolution has changed, a new sprite sheet must be loaded.  Rebuild hierarchy.
  if ( sheetResolution.compare( _i->_sheetResolution ) != 0 )
  {
    //  Reset composition and replace with new hiearchy.
    _i->_sheetResolution = sheetResolution;
    cleanup();
  }

  //  If there is no composition data, build from clip data.
  if ( _compositions.empty() )
  {
    unsigned nPrimitives = buildComposition( pClipData, frame, projectFolder, sheetResolution );

    //  4 vertices and 6 indices by primitive.
    _i->_vertices.reserve( nPrimitives * 4 );
    _i->_indices.reserve( nPrimitives * 6 );
  }
  else
  {
    updateComposition( frame );
  }

  updateRequests();

  //  Create batches from requests.  This will generate vertices and indices.
  for ( RenderBatchRequestCol_t::const_iterator i = _requests.begin(), iEnd = _requests.end() ; i!=iEnd ; ++i )
  {
    addBatch( i->_composition, i->_range.first, i->_range.second, color );
  }

  _i->_renderDirty = true;

  unlock();
}

void RD_RenderScriptPlain::updateWithBlending( const RD_ClipDataPtr_t &pClipDataFrom, const STD_String &projectFolder, const STD_String &sheetResolution, float frameFrom, float frameTo, unsigned int color, int discretizationStep , float fullBlendTime, float currentBlendTime,int blendID ){
	lock();

	_i->_vertices.clear();
	_i->_indices.clear();
	_i->_batches.clear();

	//  Render now set to frame.
	_i->_frame = frameFrom;

	//  Clip data has changed between two renders.  Replace hierarchy with new clip data.
	if ( pClipDataFrom != _i->_clipData)
	{
		//  Reset composition and replace with new hiearchy.
		_i->_clipData = pClipDataFrom;
		cleanup();
	}

	//  sheet resolution has changed, a new sprite sheet must be loaded.  Rebuild hierarchy.
	if ( sheetResolution.compare( _i->_sheetResolution ) != 0 )
	{
		//  Reset composition and replace with new hiearchy.
		_i->_sheetResolution = sheetResolution;
		cleanup();
	}

	//  If there is no composition data, build from clip data.
	if ( _compositions.empty() )
	{
		unsigned nPrimitives = buildCompositionWithBlending( pClipDataFrom, frameFrom,frameTo, projectFolder, sheetResolution,fullBlendTime, currentBlendTime,blendID );

		//  4 vertices and 6 indices by primitive.
		_i->_vertices.reserve( nPrimitives * 4 );
		_i->_indices.reserve( nPrimitives * 6 );
	}
	else
	{
		updateCompositionWithBlending( frameFrom,frameTo, fullBlendTime, currentBlendTime,blendID );
	}

	updateRequests();

	//  Create batches from requests.  This will generate vertices and indices.
	for ( RenderBatchRequestCol_t::const_iterator i = _requests.begin(), iEnd = _requests.end() ; i!=iEnd ; ++i )
	{
		addBatch( i->_composition, i->_range.first, i->_range.second, color );
	}

	_i->_renderDirty = true;

	unlock();
}

void RD_RenderScriptPlain::render( RD_Renderer *renderer, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix )
{
  lock();

  renderer->beginComposition( this );

  renderer->setMatrices(projectionMatrix, modelViewMatrix);

  for ( RenderBatchCol_t::const_iterator i = _i->_batches.begin(), iEnd = _i->_batches.end() ; i!=iEnd ; ++i )
  {
    renderer->beginSpriteSheet( i->_spriteSheet );
    renderer->renderVertices( &_i->_indices[0], i->_indexOffset, i->_nIndices, i->_vertexOffset, i->_nVertices );
    renderer->endSpriteSheet();
  }

  renderer->endComposition( this );

  _i->_renderDirty = false;

  unlock();
}

void RD_RenderScriptPlain::calculateBoundingBox( BoundingBox &box )
{
  lock();

  if ( _i->_vertices.empty() )
  {
    box._x1 = box._x2 = box._y1 = box._y2 = 0.0f;
  }
  else
  {
    VertexDataCol_t::const_iterator i = _i->_vertices.begin();
    VertexDataCol_t::const_iterator iEnd = _i->_vertices.end();

    INIT_BOUND( i->_x, box._x1, box._x2 );
    INIT_BOUND( i->_y, box._y1, box._y2 );
    ++i;

    for ( ; i!=iEnd ; ++i )
    {
      SET_BOUND( i->_x, box._x1, box._x2 );
      SET_BOUND( i->_y, box._y1, box._y2 );
    }
  }

  unlock();
}

namespace
{
  typedef RD_RenderScript::BoundingVertex    BoundingVertex;
  typedef STD_Vector< BoundingVertex >       BoundingVertexCol_t;

  typedef BoundingVertexCol_t                Polygon_t;
  typedef STD_Vector< BoundingVertexCol_t >  PolygonCol_t;

  class PolarAnglePred
  {
  public:
    PolarAnglePred(const BoundingVertex &p) : _p(p)
    {}

    bool operator()(const BoundingVertex &p1, const BoundingVertex &p2) const
    {
      float v1x = p1._x - _p._x;
      float v1y = p1._y - _p._y;

      float v2x = p2._x - _p._x;
      float v2y = p2._y - _p._y;

      if ( v1y == 0.0f && v2y == 0.0f )
        return (v1x > v2x);

      if ( v1y == 0.0f )
        return true;
      if ( v2y == 0.0f )
        return false;

      float ctg1 = v1x / v1y;
      float ctg2 = v2x / v2y;

      return (ctg1 > ctg2);
    }

    BoundingVertex _p;
  };
}

void RD_RenderScriptPlain::calculateConvexHull( BoundingVertex* &convexHullArray, int &convexHullSize )
{
  lock();

  convexHullArray = NULL;
  convexHullSize = 0;

  //  Need at least 3 vertices to make a convex hull.
  if ( _i->_vertices.size() > 2 )
  {
    const float kInf = 9999999.0f;
    //const float kThres = 0.001f;

    double min_x, min_y;
    min_x = min_y = kInf;

    size_t min_idx = 0;
    size_t idx = 0;

    //  Copy current script vertices to local array.
    BoundingVertexCol_t points;
    points.reserve( _i->_vertices.size() );
    for ( VertexDataCol_t::const_iterator i = _i->_vertices.begin(), iEnd = _i->_vertices.end() ; i!=iEnd ; ++i, ++idx )
    {
      float x = ROUND_VALUE(i->_x);
      float y = ROUND_VALUE(i->_y);

      //  Isolate lower left vertex as basis for convex hull.
      if ( (y < min_y) || ((y == min_y) && (x < min_x)) )
      {
        min_idx = idx;
        min_y = y;
        min_x = x;
      }

      points.push_back( BoundingVertex( i->_x, i->_y ) );
    }

    BoundingVertexCol_t::iterator iBegin = points.begin();
    BoundingVertexCol_t::iterator iPt = iBegin;
    STD_Advance( iPt, min_idx );
    STD_IterSwap( iBegin, iPt );

    //  Sort points agaings polar angle.
    STD_Sort( ++points.begin(), points.end(), PolarAnglePred( *points.begin() ) );

    BoundingVertexCol_t convexHull;

    //  Build convex hull using sorted points.
    iPt = points.begin();
    convexHull.push_back(*iPt); ++iPt;
    convexHull.push_back(*iPt); ++iPt;
    while ( iPt != points.end() )
    {
      if (convexHull.size() > 1)
      {
        BoundingVertexCol_t::const_reverse_iterator iHead = convexHull.rbegin();
        BoundingVertexCol_t::const_reverse_iterator iPreHead = iHead; ++iPreHead;

        BoundingVertex v1( iHead->_x - iPreHead->_x, iHead->_y - iPreHead->_y );
        BoundingVertex v2( iPt->_x - iPreHead->_x, iPt->_y - iPreHead->_y );

        if ( VEC_DET( v1._x, v1._y, v2._x, v2._y ) > 0.0f )
        {
          //  Add the point to convex hull
          convexHull.push_back(*iPt); ++iPt;
        }
        else
        {
          //  Remove point from convex hull, try again
          convexHull.pop_back();
        }
      }
      else
      {
        convexHull.push_back(*iPt); ++iPt;
      }
    }

    convexHullSize = convexHull.size();

    convexHullArray = new BoundingVertex[ convexHullSize ];
    memcpy( convexHullArray, &convexHull[0], sizeof(BoundingVertex) * convexHullSize );
  }

  unlock();
}

void RD_RenderScriptPlain::calculatePolygons( BoundingVertex* &polygonArray, int &polygonSize, int* &subPolygonArray, int &subPolygonSize )
{
  polygonArray = NULL;
  polygonSize = 0;

  subPolygonArray = NULL;
  subPolygonSize = 0;

  IndexDataCol_t::const_iterator iIndex = _i->_indices.begin();
  IndexDataCol_t::const_iterator iIndexEnd = _i->_indices.end();

  ClipperLib::Clipper c;

  //  Pre-allocate triangle memory.
  ClipperLib::Path polygon;
  polygon.resize(3);
  while ( iIndex != iIndexEnd )
  {
    VertexData &v1 = _i->_vertices[ *iIndex ];
    polygon[ 0 ].X = WORLD_TO_CLIP(v1._x);
    polygon[ 0 ].Y = WORLD_TO_CLIP(v1._y);
    ++iIndex;

    VertexData &v2 = _i->_vertices[ *iIndex ];
    polygon[ 1 ].X = WORLD_TO_CLIP(v2._x);
    polygon[ 1 ].Y = WORLD_TO_CLIP(v2._y);
    ++iIndex;

    VertexData &v3 = _i->_vertices[ *iIndex ];
    polygon[ 2 ].X = WORLD_TO_CLIP(v3._x);
    polygon[ 2 ].Y = WORLD_TO_CLIP(v3._y);
    ++iIndex;

    c.AddPath( polygon, ClipperLib::ptSubject, true );
  }

  //  Let clipperlib do its business.
  ClipperLib::Paths mergedPolygons;
  if ( c.Execute( ClipperLib::ctUnion, mergedPolygons, ClipperLib::pftNonZero, ClipperLib::pftNonZero ) )
  {
    size_t polygonArraySize = 0;
    ClipperLib::Paths::const_iterator iPolygon = mergedPolygons.begin();
    ClipperLib::Paths::const_iterator iPolygonEnd = mergedPolygons.end();

    for ( ; iPolygon!=iPolygonEnd ; ++iPolygon )
    {
      polygonArraySize += iPolygon->size();
    }

    polygonArray = new BoundingVertex[ polygonArraySize ];
    subPolygonArray = new int[ mergedPolygons.size() ];

    polygonSize = polygonArraySize;
    subPolygonSize = mergedPolygons.size();

    unsigned polygonIndex = 0;
    unsigned ptIndex = 0;

    iPolygon = mergedPolygons.begin();
    for ( ; iPolygon!=iPolygonEnd ; ++iPolygon, ++polygonIndex )
    {
      ClipperLib::Path::const_iterator iPt = iPolygon->begin();
      ClipperLib::Path::const_iterator iPtEnd = iPolygon->end();

      for ( ; iPt!=iPtEnd ; ++iPt, ++ptIndex )
      {
        polygonArray[ ptIndex ]._x = CLIP_TO_WORLD(iPt->X);
        polygonArray[ ptIndex ]._y = CLIP_TO_WORLD(iPt->Y);
      }

      subPolygonArray[ polygonIndex ] = iPolygon->size();
    }
  }
}

bool RD_RenderScriptPlain::supportsFeature( Feature feature ) const
{
  static Feature allFeatures = RD_RenderScriptPlain::supportedFeatures();
  return (allFeatures & feature) == feature;
}

RD_RenderScript::Feature RD_RenderScriptPlain::supportedFeatures()
{
  static Feature allFeatures = ePlainFeature;
  return allFeatures;
}

bool RD_RenderScriptPlain::isDirty( float frame ) const
{
  return ( frame != _i->_frame );
}

bool RD_RenderScriptPlain::isRenderDirty() const
{
  return _i->_renderDirty;
}

const RD_RenderScriptPlain::VertexData *RD_RenderScriptPlain::vertices() const
{
  return &_i->_vertices[0];
}

size_t RD_RenderScriptPlain::verticesCount() const
{
  return _i->_vertices.size();
}

const RD_RenderScriptPlain::Index_t *RD_RenderScriptPlain::indices() const
{
  return &_i->_indices[0];
}

size_t RD_RenderScriptPlain::indicesCount() const
{
  return _i->_indices.size();
}

void RD_RenderScriptPlain::addBatch( const RD_Composition *composition,
                                     const RD_Composition::CompositionNodeCol_t::const_iterator iStart,
                                     const RD_Composition::CompositionNodeCol_t::const_iterator iEnd,
                                     unsigned int color )
{
  const RD_SpriteSheetPtr_t spriteSheet = composition->spriteSheet();
  //ASSERT(spriteSheet.isValid());

  _i->_batches.push_back( RenderBatch() );
  RenderBatch &renderBatch = _i->_batches.back();

  renderBatch._indexOffset = (unsigned)_i->_indices.size();
  renderBatch._vertexOffset = (unsigned)_i->_vertices.size();

  renderBatch._spriteSheet = spriteSheet;

  unsigned index = renderBatch._vertexOffset;

  for ( RD_Composition::CompositionNodeCol_t::const_iterator i = iStart ; i!=iEnd ; ++i )
  {
    const RD_Composition::CompositionNode *node = *i;
    if ( node->_spriteName.empty() )
      continue;

    float u1, v1, u2, v2;
    RD_SpriteSheet::Rect rect;
    if ( spriteSheet->rect( node->_spriteName, rect ) &&
         spriteSheet->uvs( rect, u1, v1, u2, v2 ) )
    {
      //  Calculate current color with opacity.
      unsigned int layerColor = (unsigned int)(node->_opacity * (color & 0xFF)) |
                                ((unsigned int)(node->_opacity * ((color >> 8) & 0xFF)) << 8) |
                                ((unsigned int)(node->_opacity * ((color >> 16) & 0xFF)) << 16) |
                                ((unsigned int)(node->_opacity * ((color >> 24) & 0xFF)) << 24);

      float x_2 = float(rect._w >> 1);
      float y_2 = float(rect._h >> 1);

      float  x1 = -x_2, y1 =  y_2,
             x2 =  x_2, y2 =  y_2,
             x3 =  x_2, y3 = -y_2,
             x4 = -x_2, y4 = -y_2;

      node->_matrix.multiplyPoint( &x1, &y1 );
      node->_matrix.multiplyPoint( &x2, &y2 );
      node->_matrix.multiplyPoint( &x3, &y3 );
      node->_matrix.multiplyPoint( &x4, &y4 );

      _i->_vertices.push_back( VertexData(x1, y1, u1, v1, layerColor) );
      _i->_vertices.push_back( VertexData(x2, y2, u2, v1, layerColor) );
      _i->_vertices.push_back( VertexData(x3, y3, u2, v2, layerColor) );
      _i->_vertices.push_back( VertexData(x4, y4, u1, v2, layerColor) );

      //  1st triangle.
      _i->_indices.push_back( index );
      _i->_indices.push_back( index+1 );
      _i->_indices.push_back( index+2 );

      //  2nd triangle.
      _i->_indices.push_back( index );
      _i->_indices.push_back( index+2 );
      _i->_indices.push_back( index+3 );

      index += 4;
    }
  }

  renderBatch._nIndices = (unsigned)(_i->_indices.size() - renderBatch._indexOffset);
  renderBatch._nVertices = index - renderBatch._vertexOffset;
}

