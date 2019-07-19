
#include "RD_RenderScriptFx.h"
#include "RD_Renderer.h"

#include "PL_Mutex.h"
#include "STD_Types.h"
#include "STD_Containers.h"

#include "clipper.h"

#include <float.h>

#define MAX_ACTIVE_EFFECTS 1

#define HIGH_QUALITY_DISCRETIZATION   10.0
#define MEDIUM_QUALITY_DISCRETIZATION 20.0
#define LOW_QUALITY_DISCRETIZATION    50.0

#define PRE_MATRIX_INDEX    0
#define MATRIX_INDEX        1
#define POST_MATRIX_INDEX   2

#define FX_U_INDEX          0
#define FX_V_INDEX          1
#define FX_ADD_INDEX        2
#define FX_MULT_INDEX       3
#define FX_VP_U1_INDEX      4
#define FX_VP_V1_INDEX      5
#define FX_VP_U2_INDEX      6
#define FX_VP_V2_INDEX      7

#define COORD_X0            0
#define COORD_Y0            1
#define COORD_X1            2
#define COORD_Y1            3
#define COORD_X2            4
#define COORD_Y2            5
#define COORD_X3            6
#define COORD_Y3            7

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

namespace
{
  typedef void* CacheKey_t;

  /*!
   *  @struct BoneMapping
   *  Single vertex mapping to bone hierarchy.
   */
  struct BoneMapping
  {
    float    _boneIndex1;
    float    _boneWeight1;

    float    _boneIndex2;
    float    _boneWeight2;
  };

  static BoneMapping g_defaultMapping = { 0.0f, 1.0f, 0.0f, 0.0f };

  /*!
   *  @struct SpriteMapping
   *  Sprite discretization and mapped data.
   */
  struct SpriteMapping
  {
    SpriteMapping( unsigned xSteps, unsigned ySteps ) :
      _xSteps(xSteps),
      _ySteps(ySteps)
    {
      _bones = new BoneMapping[ (xSteps+1) * (ySteps+1) ];
    }

    ~SpriteMapping()
    {
      delete [] _bones;
    }

    BoneMapping *boneMapping( unsigned x, unsigned y )
    {
      return &_bones[ (_xSteps+1) * y + x ];
    }

    unsigned       _xSteps;
    unsigned       _ySteps;

    BoneMapping   *_bones;
  };

  typedef STD_Map< CacheKey_t, SpriteMapping* > SpriteMappingCol_t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RenderScriptFx::Impl
#endif

class RD_RenderScriptFx::Impl
{
  friend class RD_RenderScriptFx;

public:

  Impl() :
    _verticesCount(0),
    _indicesCount(0),
    _discretizationStep(MEDIUM_QUALITY_DISCRETIZATION),
    _frame(0.0f),
    _renderDirty(true)
  {
  }

  ~Impl()
  {
  }

private:

  VertexDataCol_t         _vertices;
  size_t                  _verticesCount;

  IndexDataCol_t          _indices;
  size_t                  _indicesCount;

  RD_ClipDataPtr_t        _clipData;

  STD_String              _sheetResolution;

  int                     _discretizationStep;

  SpriteMappingCol_t      _cachedMapping;

  RenderBatchCol_t        _batches;

  float                   _frame;

  bool                    _renderDirty;

  PL_Mutex                _mutex;
};

namespace
{
  void calculateBoneWeight( RD_Composition::BoneEffect *boneEffect, float xCoord, float yCoord, float &distanceSq, float &weight, unsigned &index )
  {
    //  v1: start to coord.
    float v1x = xCoord - (float)boneEffect->_restStart.x();
    float v1y = yCoord - (float)boneEffect->_restStart.y();

    //  v2: start to end.
    float v2x = float(boneEffect->_restEnd.x() - boneEffect->_restStart.x());
    float v2y = float(boneEffect->_restEnd.y() - boneEffect->_restStart.y());

    //  v1 dot v2
    float dot = v1x * v2x + v1y * v2y;

    float lengthSq = v2x*v2x + v2y*v2y;

    //  coordinate is behind bone.
    if ( dot <= 0 )
    {
      distanceSq = v1x*v1x + v1y*v1y;

      //  Weight is projected length square to start.
      weight = dot * dot / lengthSq;
      index = PRE_MATRIX_INDEX;
      return;
    }

    //  coordinate is in front of bone.
    if ( dot >= lengthSq )
    {
      float v3x = xCoord - (float)boneEffect->_restEnd.x();
      float v3y = yCoord - (float)boneEffect->_restEnd.y();

      distanceSq = v3x*v3x + v3y*v3y;

      //  Weight is projected length square to end.
      float dot2 = v3x * v2x + v3y * v2y;
      weight = dot2 * dot2 / lengthSq;

      index = POST_MATRIX_INDEX;
      return;
    }

    float cross = v1x * v2y - v1y * v2x;

    distanceSq = weight = cross * cross / lengthSq;
    index = MATRIX_INDEX;
  }

  SpriteMapping *createSpriteMapping( const RD_Composition::CompositionNode *node, const RD_SpriteSheet::SpriteData *spriteData, int discretizationStep )
  {
    bool isDeformed = !node->_bones.empty();

    unsigned xSteps = 1u;
    unsigned ySteps = 1u;

    //  Deformed sprite, discretize geometry for smoother deformation.
	if ( isDeformed )
	{
		const int WIDTH = spriteData->_rect._w;
		const int HEIGHT = spriteData->_rect._h;
		const double ASPECT_RATIO = (double)WIDTH / (double)HEIGHT;

		double discretizationX, discretizationY;
		if (WIDTH > HEIGHT)
		{
			discretizationX = (double)discretizationStep * ASPECT_RATIO;
			discretizationY = (double)discretizationStep;
		}
		else
		{
			discretizationX = (double)discretizationStep;
			discretizationY = (double)discretizationStep * 1.0/ASPECT_RATIO;
		}

		xSteps = std::max( 1u, unsigned(discretizationX));
		ySteps = std::max( 1u, unsigned(discretizationY));
	}

	return new SpriteMapping( xSteps, ySteps );
  }

  void updateBoneMapping( const RD_Composition::CompositionNode *node, float xCoord, float yCoord, BoneMapping *boneMapping )
  {
    bool isDeformed = !node->_bones.empty();

    if ( isDeformed )
    {
      float bestDistanceSq = FLT_MAX;

      //  Note.  Using a quad tree could speed up calculations.
      for ( RD_Composition::BoneEffectCol_t::const_iterator i = node->_bones.begin(), iEnd = node->_bones.end() ; i!=iEnd ; ++i )
      {
        RD_Composition::BoneEffect *boneEffect = *i;
        float distanceSq;
        float weight;
        unsigned index;

        calculateBoneWeight( boneEffect, xCoord, yCoord, distanceSq, weight, index );

        //  Better match.
        if ( distanceSq < bestDistanceSq )
        {
          boneMapping->_boneIndex1 = float(boneEffect->_effectIdx*3 + index + 1);
          bestDistanceSq = distanceSq;

          if ( index == PRE_MATRIX_INDEX )
          {
            if ( boneEffect->_parentBone == NULL )
            {
              //  First bone, only use this bone in skinning.
              boneMapping->_boneWeight1 = 1.0;
              boneMapping->_boneIndex2 = boneMapping->_boneWeight2 = 0.0f;
            }
            else
            {
              //  Skinning is a combination of this bone and its parent.
              float distanceSq2;
              float weight2;
              unsigned index2;
              calculateBoneWeight( boneEffect->_parentBone, xCoord, yCoord, distanceSq2, weight2, index2 );

              boneMapping->_boneWeight1 = weight2 / (weight+weight2);

              boneMapping->_boneIndex2 = float(boneEffect->_parentBone->_effectIdx*3 + index2 + 1);
              boneMapping->_boneWeight2 = weight / (weight+weight2);

            }
          }
          else if ( index == POST_MATRIX_INDEX )
          {
            if ( boneEffect->_childBones.empty() )
            {
              // Last bone, only use this bone in skinning.
              boneMapping->_boneWeight1 = 1.0;
              boneMapping->_boneIndex2 = boneMapping->_boneWeight2 = 0.0;
            }
            else
            {
              //  Skinning is a combination of this bone and its nearest child.
              float bestDistanceSq2 = FLT_MAX;
              float bestWeight2;
              unsigned bestIndex2 = 0u;
              RD_Composition::BoneEffect *bestChildBone = NULL;

              //  Note! There might be a more effective way of finding the nearest bone.
              for ( RD_Composition::BoneEffectCol_t::const_iterator i = boneEffect->_childBones.begin(), iEnd = boneEffect->_childBones.end() ; i!=iEnd ; ++i )
              {
                RD_Composition::BoneEffect *childBone = *i;

                float distanceSq2;
                float weight2;
                unsigned index2;
                calculateBoneWeight( *i, xCoord, yCoord, distanceSq2, weight2, index2 );

                if ( distanceSq2 < bestDistanceSq2 )
                {
                  bestDistanceSq2 = distanceSq2;
                  bestWeight2 = weight2;
                  bestIndex2 = index2;
                  bestChildBone = childBone;
                }
              }

              if ( bestChildBone != NULL )
              {
                boneMapping->_boneWeight1 = bestWeight2 / (weight + bestWeight2);

                boneMapping->_boneIndex2 = float(bestChildBone->_effectIdx*3 + bestIndex2 + 1);
                boneMapping->_boneWeight2 = weight / (weight + bestWeight2);
              }
              else
              {
                //  Shouldn't happen.
                boneMapping->_boneWeight1 = 1.0;
                boneMapping->_boneIndex2 = boneMapping->_boneWeight2 = 0.0f;
              }
            }
          }
          else
          {
            //  Only skin with this bone.
            boneMapping->_boneWeight1 = 1.0;
            boneMapping->_boneIndex2 = boneMapping->_boneWeight2 = 0.0;
          }
        }
      }
    }
    else
    {
      //  Default skinning.
      //  1st bone uses 1st bone index (identity matrix) with a weight value of 1.
      boneMapping->_boneIndex1 = 0.0f;
      boneMapping->_boneWeight1 = 1.0f;
      //  2nd bone uses (whatever) bone index with a weight value of 0.
      boneMapping->_boneIndex2 = 0.0f;
      boneMapping->_boneWeight2 = 0.0f;
    }
  }

  void applyLinearBaseSkinning( float weight1,
                                const float *matrix1,
                                float weight2,
                                const float *matrix2,
                                float x,
                                float y,
                                float &newx,
                                float &newy )
  {
    //  Skin current vertex using two bone matrices and their weights.
    //  This will be used as software fallback for gpu skinning.
    newx = (matrix1[0]*weight1 + matrix2[0]*weight2) * x + (matrix1[4]*weight1 + matrix2[4]*weight2) * y +
           (matrix1[12]*weight1 + matrix2[12]*weight2);
    newy = (matrix1[1]*weight1 + matrix2[1]*weight2) * x + (matrix1[5]*weight1 + matrix2[5]*weight2) * y +
           (matrix1[13]*weight1 + matrix2[13]*weight2);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RenderScriptFx
#endif

RD_RenderScriptFx::RD_RenderScriptFx()
{
  _i = new Impl;
}

RD_RenderScriptFx::RD_RenderScriptFx(const RD_RenderScriptMetaPtr_t &meta) :
  RD_RenderScript(meta)
{
  _i = new Impl;
}

RD_RenderScriptFx::~RD_RenderScriptFx()
{
  cleanup();
  delete _i;
}

void RD_RenderScriptFx::update( const RD_ClipDataPtr_t &pClipData, const STD_String &projectFolder, const STD_String &sheetResolution, float frame, unsigned int color, int discretizationStep )
{
  lock();

  //  Reset vertices and indices count.  Both array remain allocated to limit reallocations.
  _i->_verticesCount = 0;
  _i->_indicesCount = 0;

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

  if ( _i->_discretizationStep != discretizationStep )
  {
    _i->_discretizationStep = discretizationStep;
    clearSpriteMapping();
  }

  //  If there is no composition data, build from clip data.
  if ( _compositions.empty() )
  {
    //  Clear batches.
    //  It could be reused to limit memory allocations.
    _i->_batches.clear();
    buildComposition( pClipData, frame, projectFolder, sheetResolution );
  }
  else
  {
    //  Update existing compositions.
    updateComposition( frame );
  }

  updateRequests();

  //  Resize batch container if smaller than request size.
  if (_requests.size() > _i->_batches.size())
    _i->_batches.resize( _requests.size(), RenderBatch() );

  //  Create batches from requests.  This will generate vertices and indices.
  RenderBatchCol_t::iterator iBatch = _i->_batches.begin();
  for ( RenderBatchRequestCol_t::const_iterator i = _requests.begin(), iEnd = _requests.end() ; i!=iEnd ; ++i, ++iBatch )
  {
    updateBatch( i->_composition, i->_range.first, i->_range.second, color, *iBatch );
  }

  _i->_renderDirty = true;

  unlock();
}

void RD_RenderScriptFx::updateWithBlending( const RD_ClipDataPtr_t &pClipDataFrom,const STD_String &projectFolder, const STD_String &sheetResolution, float frameFrom, float frameTo, unsigned int color, int discretizationStep , float fullBlendTime, float currentBlendTime,int blendID ){
	lock();

	//  Reset vertices and indices count.  Both array remain allocated to limit reallocations.
	_i->_verticesCount = 0;
	_i->_indicesCount = 0;

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

	if ( _i->_discretizationStep != discretizationStep )
	{
		_i->_discretizationStep = discretizationStep;
		clearSpriteMapping();
	}

	//  If there is no composition data, build from clip data.
	if ( _compositions.empty() )
	{
		//  Clear batches.
		//  It could be reused to limit memory allocations.
		_i->_batches.clear();
		buildCompositionWithBlending( pClipDataFrom, frameFrom,frameTo, projectFolder, sheetResolution,fullBlendTime, currentBlendTime,blendID );
	}
	else
	{
		//  Update existing compositions.
		updateCompositionWithBlending( frameFrom,frameTo, fullBlendTime, currentBlendTime,blendID );
	}

	updateRequests();

	//  Resize batch container if smaller than request size.
	if (_requests.size() > _i->_batches.size())
		_i->_batches.resize( _requests.size(), RenderBatch() );

	//  Create batches from requests.  This will generate vertices and indices.
	RenderBatchCol_t::iterator iBatch = _i->_batches.begin();
	for ( RenderBatchRequestCol_t::const_iterator i = _requests.begin(), iEnd = _requests.end() ; i!=iEnd ; ++i, ++iBatch )
	{
		updateBatch( i->_composition, i->_range.first, i->_range.second, color, *iBatch );
	}

	_i->_renderDirty = true;

	unlock();
}

void RD_RenderScriptFx::render( RD_Renderer *renderer, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix )
{
  lock();

  renderer->beginComposition( this );

  renderer->setMatrices( projectionMatrix, modelViewMatrix );

  for ( RenderBatchCol_t::const_iterator i = _i->_batches.begin(), iEnd = _i->_batches.end() ; i!=iEnd ; ++i )
  {
    renderer->beginSpriteSheet( i->_spriteSheet );
    renderer->beginBatch( *i );
    renderer->renderVertices( &_i->_indices[0], i->_indexOffset, i->_nIndices, i->_vertexOffset, i->_nVertices );
    renderer->endBatch( *i );
    renderer->endSpriteSheet();
  }

  renderer->endComposition( this );

  _i->_renderDirty = false;

  unlock();
}

void RD_RenderScriptFx::calculateBoundingBox( BoundingBox &box )
{
  lock();

  if ( _i->_vertices.empty() )
  {
    box._x1 = box._x2 = box._y1 = box._y2 = 0.0f;
  }
  else
  {
    VertexDataCol_t::const_iterator iVertex = _i->_vertices.begin();
    VertexDataCol_t::const_iterator iVertexEnd = iVertex;

    INIT_BOUND( iVertex->_x, box._x1, box._x2 );
    INIT_BOUND( iVertex->_y, box._y1, box._y2 );

    for ( RenderBatchCol_t::const_iterator iBatch = _i->_batches.begin(), iBatchEnd = _i->_batches.end() ; iBatch!=iBatchEnd ; ++iBatch )
    {
      STD_Advance( iVertexEnd, iBatch->_nVertices );

      for ( ; iVertex!=iVertexEnd ; ++iVertex )
      {
        int boneIndex1 = (int)iVertex->_boneParams[0];
        int boneIndex2 = (int)iVertex->_boneParams[2];

        if ( (boneIndex1 > 0) || (boneIndex2 > 0) )
        {
          float skinX, skinY;
          applyLinearBaseSkinning( iVertex->_boneParams[1], (const float*)&iBatch->_uniformBoneMatrices[ boneIndex1 ],
                                   iVertex->_boneParams[3], (const float*)&iBatch->_uniformBoneMatrices[ boneIndex2 ],
                                   iVertex->_x, iVertex->_y, skinX, skinY );

          SET_BOUND( skinX, box._x1, box._x2 );
          SET_BOUND( skinY, box._y1, box._y2 );
        }
        else
        {
          SET_BOUND( iVertex->_x, box._x1, box._x2 );
          SET_BOUND( iVertex->_y, box._y1, box._y2 );
        }
      }
    }
  }

  unlock();
}

namespace
{
  typedef RD_RenderScript::BoundingVertex BoundingVertex;
  typedef STD_Vector< BoundingVertex > BoundingVertexCol_t;

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

void RD_RenderScriptFx::calculateConvexHull( BoundingVertex* &convexHullArray, int &convexHullSize )
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

    for ( RenderBatchCol_t::const_iterator iBatch = _i->_batches.begin(), iBatchEnd = _i->_batches.end() ; iBatch!=iBatchEnd ; ++iBatch )
    {
      VertexDataCol_t::const_iterator iVertex = _i->_vertices.begin();
      STD_Advance( iVertex, iBatch->_vertexOffset );

      VertexDataCol_t::const_iterator iVertexEnd = iVertex;
      STD_Advance( iVertexEnd, iBatch->_nVertices );

      for ( ; iVertex!=iVertexEnd ; ++iVertex, ++idx )
      {
        int boneIndex1 = (int)iVertex->_boneParams[0];
        int boneIndex2 = (int)iVertex->_boneParams[2];

        if ( (boneIndex1 > 0) || (boneIndex2 > 0) )
        {
          float skinX, skinY;
          applyLinearBaseSkinning( iVertex->_boneParams[1], (const float*)&iBatch->_uniformBoneMatrices[ boneIndex1 ],
                                   iVertex->_boneParams[3], (const float*)&iBatch->_uniformBoneMatrices[ boneIndex2 ],
                                   iVertex->_x, iVertex->_y, skinX, skinY );

          //  Round value for numerical stability.
          skinX = ROUND_VALUE(skinX);
          skinY = ROUND_VALUE(skinY);

          if ( (skinY < min_y) || ((skinY == min_y) && (skinX < min_x)) )
          {
            min_idx = idx;
            min_y = skinY;
            min_x = skinX;
          }

          points.push_back( BoundingVertex(skinX, skinY) );
        }
        else
        {
          //  Round value for numerical stability.
          float x = ROUND_VALUE(iVertex->_x);
          float y = ROUND_VALUE(iVertex->_y);

          if ( (y < min_y) || ((y == min_y) && (x < min_x)) )
          {
            min_idx = idx;
            min_y = y;
            min_x = x;
          }

          points.push_back( BoundingVertex(x, y) );
        }
      }
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

void RD_RenderScriptFx::calculatePolygons( BoundingVertex* &polygonArray, int &polygonSize, int* &subPolygonArray, int &subPolygonSize )
{
  polygonArray = NULL;
  polygonSize = 0;

  subPolygonArray = NULL;
  subPolygonSize = 0;

  ClipperLib::Clipper c;

  //  Iterate through composition to reconstruct a bounding polygon that is only the outer
  //  frame of the deformed sprite.
  CompositionCol_t::const_iterator iComposition = _compositions.begin();
  CompositionCol_t::const_iterator iCompositionEnd = _compositions.end();

  RenderBatchCol_t::const_iterator iBatch = _i->_batches.begin();
  //RenderBatchCol_t::const_iterator iBatchEnd = _i->_batches.end();

  float  coords[8];

  for ( ; iComposition!=iCompositionEnd ; ++iComposition, ++iBatch )
  {
    const RD_Composition *composition = *iComposition;
    const RenderBatch &renderBatch = *iBatch;
    const RD_SpriteSheetPtr_t spriteSheet = composition->spriteSheet();

    RD_Composition::CompositionNodeCol_t::const_iterator iNode = composition->nodeBegin();
    RD_Composition::CompositionNodeCol_t::const_iterator iNodeEnd = composition->nodeEnd();

    for ( ; iNode!=iNodeEnd ; ++iNode )
    {
      const RD_Composition::CompositionNode *node = *iNode;
      if ( node->_spriteName.empty() )
        continue;

      const RD_SpriteSheet::SpriteData *spriteData = spriteSheet->sprite( node->_spriteName );
      if ( !spriteData )
        continue;

      SpriteMapping *spriteMapping = NULL;
      SpriteMappingCol_t::const_iterator iMapping = _i->_cachedMapping.find( (CacheKey_t)spriteData );
      if ( iMapping != _i->_cachedMapping.end() )
      {
        spriteMapping = iMapping->second;
      }

      //  sprite mapping should have been created at this point, 
      if ( !spriteMapping )
        continue;

      RD_SpriteSheet::Rect rect = spriteData->_rect;

      //  Transform coordinates using current matrix.
      float rectw = float(rect._w);
      float recth = float(rect._h);
      float x_2 = rectw * 0.5f;
      float y_2 = recth * 0.5f;

      //  Calculate and transform bounding coordinates.
      coords[COORD_X0] = coords[COORD_X3] = -x_2;
      coords[COORD_X1] = coords[COORD_X2] =  x_2;
      coords[COORD_Y0] = coords[COORD_Y1] = -y_2;
      coords[COORD_Y2] = coords[COORD_Y3] =  y_2;

      node->_matrix.multiplyPoint( &coords[COORD_X0], &coords[COORD_Y0] );
      node->_matrix.multiplyPoint( &coords[COORD_X1], &coords[COORD_Y1] );
      node->_matrix.multiplyPoint( &coords[COORD_X2], &coords[COORD_Y2] );
      node->_matrix.multiplyPoint( &coords[COORD_X3], &coords[COORD_Y3] );

      ClipperLib::Path polygon;

      unsigned xSteps = spriteMapping->_xSteps;
      unsigned ySteps = spriteMapping->_ySteps;

      //  Bottom edge.
      for ( unsigned xIndex=0 ; xIndex<xSteps ; ++xIndex )
      {
        float tx = float(xIndex) / float(xSteps);
        float tx1 = 1.0f - tx;

        float x = coords[COORD_X0] * tx1 + coords[COORD_X1] * tx;
        float y = coords[COORD_Y0] * tx1 + coords[COORD_Y1] * tx;

        BoneMapping *boneMapping = spriteMapping->boneMapping( xIndex, 0 );

        const float *skinMatrix1 = (const float*)&renderBatch._uniformBoneMatrices[ (int)boneMapping->_boneIndex1 ];
        const float *skinMatrix2 = (const float*)&renderBatch._uniformBoneMatrices[ (int)boneMapping->_boneIndex2 ];
        applyLinearBaseSkinning( boneMapping->_boneWeight1, skinMatrix1, boneMapping->_boneWeight2, skinMatrix2, x, y, x, y );

        polygon.push_back( ClipperLib::IntPoint( WORLD_TO_CLIP(x), WORLD_TO_CLIP(y) ) );
      }

      //  Right edge.
      for ( unsigned yIndex=0 ; yIndex<ySteps ; ++yIndex )
      {
        float ty = float(yIndex) / float(ySteps);
        float ty1 = 1.0f - ty;

        float x = coords[COORD_X1] * ty1 + coords[COORD_X2] * ty;
        float y = coords[COORD_Y1] * ty1 + coords[COORD_Y2] * ty;

        BoneMapping *boneMapping = spriteMapping->boneMapping( xSteps, yIndex );

        const float *skinMatrix1 = (const float*)&renderBatch._uniformBoneMatrices[ (int)boneMapping->_boneIndex1 ];
        const float *skinMatrix2 = (const float*)&renderBatch._uniformBoneMatrices[ (int)boneMapping->_boneIndex2 ];
        applyLinearBaseSkinning( boneMapping->_boneWeight1, skinMatrix1, boneMapping->_boneWeight2, skinMatrix2, x, y, x, y );

        polygon.push_back( ClipperLib::IntPoint( WORLD_TO_CLIP(x), WORLD_TO_CLIP(y) ) );
      }

      //  Top edge.
      for ( unsigned xIndex=0 ; xIndex<xSteps ; ++xIndex )
      {
        float ty = float(xIndex) / float(xSteps);
        float ty1 = 1.0f - ty;

        float x = coords[COORD_X2] * ty1 + coords[COORD_X3] * ty;
        float y = coords[COORD_Y2] * ty1 + coords[COORD_Y3] * ty;

        BoneMapping *boneMapping = spriteMapping->boneMapping( xSteps-xIndex, ySteps );

        const float *skinMatrix1 = (const float*)&renderBatch._uniformBoneMatrices[ (int)boneMapping->_boneIndex1 ];
        const float *skinMatrix2 = (const float*)&renderBatch._uniformBoneMatrices[ (int)boneMapping->_boneIndex2 ];
        applyLinearBaseSkinning( boneMapping->_boneWeight1, skinMatrix1, boneMapping->_boneWeight2, skinMatrix2, x, y, x, y );

        polygon.push_back( ClipperLib::IntPoint( WORLD_TO_CLIP(x), WORLD_TO_CLIP(y) ) );
      }

      //  Left edge.
      for ( unsigned yIndex=0 ; yIndex<ySteps ; ++yIndex )
      {
        float ty = float(yIndex) / float(ySteps);
        float ty1 = 1.0f - ty;

        float x = coords[COORD_X3] * ty1 + coords[COORD_X0] * ty;
        float y = coords[COORD_Y3] * ty1 + coords[COORD_Y0] * ty;

        BoneMapping *boneMapping = spriteMapping->boneMapping( 0, ySteps-yIndex );

        const float *skinMatrix1 = (const float*)&renderBatch._uniformBoneMatrices[ (int)boneMapping->_boneIndex1 ];
        const float *skinMatrix2 = (const float*)&renderBatch._uniformBoneMatrices[ (int)boneMapping->_boneIndex2 ];
        applyLinearBaseSkinning( boneMapping->_boneWeight1, skinMatrix1, boneMapping->_boneWeight2, skinMatrix2, x, y, x, y );

        polygon.push_back( ClipperLib::IntPoint( WORLD_TO_CLIP(x), WORLD_TO_CLIP(y) ) );
      }

      c.AddPath( polygon, ClipperLib::ptSubject, true );
    }
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

bool RD_RenderScriptFx::supportsFeature( Feature feature ) const
{
  static Feature allFeatures = RD_RenderScriptFx::supportedFeatures();
  return (allFeatures & feature) == feature;
}

RD_RenderScript::Feature RD_RenderScriptFx::supportedFeatures()
{
  static Feature allFeatures = Feature(ePlainFeature | eCutterFeature | eDeformationFeature);
  return allFeatures;
}

bool RD_RenderScriptFx::isDirty( float frame ) const
{
  return ( frame != _i->_frame );
}

bool RD_RenderScriptFx::isRenderDirty() const
{
  return _i->_renderDirty;
}


const RD_RenderScriptFx::VertexData *RD_RenderScriptFx::vertices() const
{
  return &_i->_vertices[0];
}

size_t RD_RenderScriptFx::verticesCount() const
{
  return _i->_verticesCount;
}

const RD_RenderScriptFx::Index_t *RD_RenderScriptFx::indices() const
{
  return &_i->_indices[0];
}

size_t RD_RenderScriptFx::indicesCount() const
{
  return _i->_indicesCount;
}

void RD_RenderScriptFx::cleanup()
{
  RD_RenderScript::cleanup();
  clearSpriteMapping();
}

void RD_RenderScriptFx::clearSpriteMapping()
{
  //  Cleanup cached mapping
  for ( SpriteMappingCol_t::const_iterator i = _i->_cachedMapping.begin(), iEnd = _i->_cachedMapping.end() ; i!=iEnd ; ++i )
  {
    delete i->second;
  }
  _i->_cachedMapping.clear();
}

void RD_RenderScriptFx::updateBatch( const RD_Composition *composition,
                                     const RD_Composition::CompositionNodeCol_t::const_iterator iCompositionStart,
                                     const RD_Composition::CompositionNodeCol_t::const_iterator iCompositionEnd,
                                     unsigned int color,
                                     RenderBatch &renderBatch )
{
  const RD_SpriteSheetPtr_t spriteSheet = composition->spriteSheet();
  //ASSERT(spriteSheet.isValid());

  //const IM_ImagePtr_t &spriteSheetImage = spriteSheet->image();
  //ASSERT(spriteSheetImage.isValid());

  //_i->_batches.push_back( RenderBatch() );
  //RenderBatch &renderBatch = _i->_batches.back();

  renderBatch._indexOffset =  (unsigned)_i->_indicesCount;
  renderBatch._vertexOffset = (unsigned)_i->_verticesCount;

  renderBatch._spriteSheet = spriteSheet;

  unsigned index = renderBatch._vertexOffset;

  float  width = (float)spriteSheet->width();
  float  height = (float)spriteSheet->height();

  float  coords[8];
  float  coords2[4];

  //  Vertex parameters for cutter
  //  | fxU             | fxV             | fxAdd           | fxMult          |
  //  |-----------------|-----------------|-----------------|-----------------|
  //  | (float) 32 bits | (float) 32 bits | (float) 32 bits | (float) 32 bits |
  //
  //  | fxVp_u1         | fxVp_v1         | fxVp_u2         | fxVp_v2         |
  //  |-----------------|-----------------|-----------------|-----------------|
  //  | (float) 32 bits | (float) 32 bits | (float) 32 bits | (float) 32 bits |
  float  fxParams[MAX_ACTIVE_EFFECTS * 8];

  //  Three matrices per bone and an identity matrix for default bone.
  size_t newBoneMatricesSize = (composition->boneCount() * 3 + 1);
  if ( renderBatch._uniformBoneMatrices.size() < newBoneMatricesSize )
  {
    renderBatch._uniformBoneMatrices.resize(newBoneMatricesSize);
  }

  //  1st bone matrix is identity.
  Math::Matrix4x4().getValues( (float*)&renderBatch._uniformBoneMatrices[ 0 ] );

  //  Iterate on all bones and setup matrices used in uniform parameters.
  for ( RD_Composition::BoneEffectCol_t::const_iterator i = composition->boneBegin(), iEnd = composition->boneEnd() ; i!=iEnd ; ++i )
  {
    const RD_Composition::BoneEffect *boneEffect = *i;

    //  Copy bone matrix to render batch uniform parameters.
    boneEffect->_preMatrix.getValues( (float*)&renderBatch._uniformBoneMatrices[ (boneEffect->_effectIdx*3 + 1) ] );
    boneEffect->_matrix.getValues( (float*)&renderBatch._uniformBoneMatrices[ (boneEffect->_effectIdx*3 + 2) ] );
    boneEffect->_postMatrix.getValues( (float*)&renderBatch._uniformBoneMatrices[ (boneEffect->_effectIdx*3 + 3) ] );
  }

  //  Iterate on all composition nodes and create vertices to render. 
  for ( RD_Composition::CompositionNodeCol_t::const_iterator i = iCompositionStart, iEnd = iCompositionEnd ; i!=iEnd ; ++i )
  {
    const RD_Composition::CompositionNode *node = *i;
    if ( node->_spriteName.empty() )
      continue;

    const RD_SpriteSheet::SpriteData *spriteData = spriteSheet->sprite( node->_spriteName );
    if ( !spriteData )
      continue;

    float u1, v1, u2, v2;
    RD_SpriteSheet::Rect rect = spriteData->_rect;
    if ( spriteSheet->uvs( rect, u1, v1, u2, v2 ) )
    {
      bool boneMappingDirty = false;

      SpriteMapping *spriteMapping = NULL;
      SpriteMappingCol_t::const_iterator iMapping = _i->_cachedMapping.find( (CacheKey_t)spriteData );
      if ( iMapping != _i->_cachedMapping.end() )
      {
        //  Reuse existing sprite mapping for current sprite.
        spriteMapping = iMapping->second;
      }
      else
      {
        //  Create new sprite mapping for sprite and add to chached mappings.
        spriteMapping = createSpriteMapping( node, spriteData, _i->_discretizationStep );
        _i->_cachedMapping.insert( STD_MakePair( (CacheKey_t)spriteData, spriteMapping ) );

        boneMappingDirty = true;
      }

      unsigned xSteps = spriteMapping->_xSteps;
      unsigned ySteps = spriteMapping->_ySteps;

      //  Calculate current color with opacity.
      unsigned int layerColor = (unsigned int)(node->_opacity * (color & 0xFF)) |
                                ((unsigned int)(node->_opacity * ((color >> 8) & 0xFF)) << 8) |
                                ((unsigned int)(node->_opacity * ((color >> 16) & 0xFF)) << 16) |
                                ((unsigned int)(node->_opacity * ((color >> 24) & 0xFF)) << 24);

      //  Transform coordinates using current matrix.
      float rectw = float(rect._w);
      float recth = float(rect._h);
      float x_2 = rectw * 0.5f;
      float y_2 = recth * 0.5f;

      //  Calculate and transform bounding coordinates.
      coords[COORD_X0] = coords[COORD_X3] = -x_2;
      coords[COORD_X1] = coords[COORD_X2] =  x_2;
      coords[COORD_Y0] = coords[COORD_Y1] = -y_2;
      coords[COORD_Y2] = coords[COORD_Y3] =  y_2;

      node->_matrix.multiplyPoint( &coords[COORD_X0], &coords[COORD_Y0] );
      node->_matrix.multiplyPoint( &coords[COORD_X1], &coords[COORD_Y1] );
      node->_matrix.multiplyPoint( &coords[COORD_X2], &coords[COORD_Y2] );
      node->_matrix.multiplyPoint( &coords[COORD_X3], &coords[COORD_Y3] );

      //  Iterate on all effects to retrieve viewports and 
      //  mix terms that are constant to every vertices.
      unsigned idx = 0;
      size_t nEffects = 0;
      if ( node->_cutters.size() <= MAX_ACTIVE_EFFECTS )
      {
        //  Apply available effects.
        if ( !node->_cutters.empty() )
        {
          nEffects = node->_cutters.size();

          for ( ; idx < nEffects ; ++idx )
          {
            const RD_Composition::CutterEffect *cutter = node->_cutters[idx];

            //  Attached matte, calculate matte uvs
            if ( cutter->_nodeTreeView.isValid() )
            {
              //  idx * 8
              unsigned fxIndex = idx << 3;

              //  Translate uvs from drawing coordinates to effect coordinates.
              if ( spriteSheet->uvs( cutter->_spriteName, &fxParams[fxIndex + FX_VP_U1_INDEX] ) )
              {
                TR_Types::EffectId_t effectId = cutter->_effectDataView->effectId();

                float add = (effectId == TR_Types::eCutter) ? 1.0f : 0.0f;
                float mult = (effectId == TR_Types::eCutter) ? -1.0f : 1.0f;

                fxParams[fxIndex + FX_ADD_INDEX] = add;
                fxParams[fxIndex + FX_MULT_INDEX] = mult;
              }
            }
          }
        }
      }

      //  Set remaining effects to default (add = 1, mult = 0)
      for ( ; idx < MAX_ACTIVE_EFFECTS ; ++idx )
      {
        //  idx * 8
        unsigned fxIndex = idx << 3;

        //  Unused effect.
        fxParams[fxIndex + FX_ADD_INDEX] = 1.0f;     // add
        fxParams[fxIndex + FX_MULT_INDEX] = 0.0f;    // mult
      }

      //  Resize vertices array if required.
      size_t neededVertices = (xSteps+1)*(ySteps+1);
      size_t newVerticesSize = _i->_verticesCount + neededVertices;
      if ( _i->_vertices.size() < newVerticesSize )
        _i->_vertices.resize( newVerticesSize );

      VertexData *vertexData = &_i->_vertices[ _i->_verticesCount ];

      //  Create vertices.
      for ( unsigned yIndex=0 ; yIndex<=ySteps ; ++yIndex )
      {
        float ty = float(yIndex) / float(ySteps);
        float ty1 = 1.0f - ty;

        //  p1' = (x0,y0) + ((x3,y3) - (x0,y0)) * ty
        //  p2' = (x1,y1) + ((x2,y2) - (x1,y1)) * ty
        coords2[COORD_X0] = coords[COORD_X0] * ty1 + coords[COORD_X3] * ty;
        coords2[COORD_Y0] = coords[COORD_Y0] * ty1 + coords[COORD_Y3] * ty;

        coords2[COORD_X1] = coords[COORD_X1] * ty1 + coords[COORD_X2] * ty;
        coords2[COORD_Y1] = coords[COORD_Y1] * ty1 + coords[COORD_Y2] * ty;

        float vCoord = v2 + ty * (v1-v2);

        for ( unsigned xIndex=0 ; xIndex<=xSteps ; ++xIndex )
        {
          float tx = float(xIndex) / float(xSteps);
          float tx1 = 1.0f - tx;

          //  p3' = p1' + (p2' - p1') * tx
          vertexData->_x = coords2[COORD_X0] * tx1 + coords2[COORD_X1] * tx;
          vertexData->_y = coords2[COORD_Y0] * tx1 + coords2[COORD_Y1] * tx;
          vertexData->_z = 0.0f;

          vertexData->_color = layerColor;

          vertexData->_u0 = u1 + tx * (u2-u1);
          vertexData->_v0 = vCoord;

          //  Iterate on all effects to calculate matte uvs.
          for ( unsigned idx = 0 ; idx < nEffects ; ++idx )
          {
            //  idx * 8
            unsigned fxIndex = idx << 3;

            float xMatte = vertexData->_x;
            float yMatte = vertexData->_y;

            const RD_Composition::CutterEffect *cutter = node->_cutters[idx];
            cutter->_matrix.multiplyPoint( &xMatte, &yMatte );

            float *fxViewport = &fxParams[fxIndex + FX_VP_U1_INDEX];

            float u0 = (fxViewport[0] + fxViewport[2]) * 0.5f;
            float v0 = (fxViewport[1] + fxViewport[3]) * 0.5f;

            fxParams[fxIndex + FX_U_INDEX] = u0 + (xMatte / width);
            fxParams[fxIndex + FX_V_INDEX] = v0 - (yMatte / height);
          }

          //  Retrieve bone mapping and update if not set.
          BoneMapping *boneMapping = spriteMapping->boneMapping( xIndex, yIndex );
          if ( boneMappingDirty )
          {
            updateBoneMapping(node, vertexData->_x, vertexData->_y, boneMapping );
          }

          //  Copy cutter parameters to vertex data.
          memcpy( vertexData->_fxParams0, fxParams, sizeof(float) * MAX_ACTIVE_EFFECTS * 8 );

          //  Copy bone parameters to vertex data.
          if ( (boneMapping->_boneIndex1 < MAX_BONES_GPUf) &&
               (boneMapping->_boneIndex2 < MAX_BONES_GPUf) )
          {
            memcpy( vertexData->_boneParams, boneMapping, sizeof(float) * 4 );
          }
          //  Software fallback.  If bone indices are outside GPU array bounds, apply calculations directly on vertices.
          else
          {
            const float *skinMatrix1 = (const float*)&renderBatch._uniformBoneMatrices[ (int)boneMapping->_boneIndex1 ];
            const float *skinMatrix2 = (const float*)&renderBatch._uniformBoneMatrices[ (int)boneMapping->_boneIndex2 ];
            applyLinearBaseSkinning( boneMapping->_boneWeight1, skinMatrix1, boneMapping->_boneWeight2, skinMatrix2,
                vertexData->_x, vertexData->_y, vertexData->_x, vertexData->_y );

            //  Default mapping so that no deformation is applied on bone in GPU.
            memcpy( vertexData->_boneParams, &g_defaultMapping, sizeof(float) * 4 );
          }

          ++vertexData;
        }
      }

      _i->_verticesCount = newVerticesSize;

      //  Resize indices array if required.
      size_t neededIndices = (xSteps)*(ySteps)*6;
      size_t newIndicesSize = _i->_indicesCount + neededIndices;
      if ( _i->_indices.size() < newIndicesSize )
        _i->_indices.resize( newIndicesSize );

      Index_t *indexArray = &_i->_indices[ _i->_indicesCount ];

      //  Create indices for vertices.
      for ( unsigned yIndex=0 ; yIndex<ySteps ; ++yIndex )
      {
        for ( unsigned xIndex=0 ; xIndex<xSteps ; ++xIndex )
        {
          int index0 = index + yIndex * (xSteps+1) + xIndex;
          int index1 = index0 + 1;

          int index3 = index0 + (xSteps+1);
          int index2 = index3 + 1;

          //  1st triangle.
          indexArray[0] = index0;
          indexArray[1] = index1;
          indexArray[2] = index2;

          //  2nd triangle.
          indexArray[3] = index0;
          indexArray[4] = index2;
          indexArray[5] = index3;

          indexArray += 6;
        }
      }

      _i->_indicesCount = newIndicesSize;

      index += neededVertices;
    }
  }

  renderBatch._nIndices = (unsigned)(_i->_indicesCount - renderBatch._indexOffset);
  renderBatch._nVertices = index - renderBatch._vertexOffset;
}

