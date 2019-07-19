
#ifndef _RD_RENDER_SCRIPT_H_
#define _RD_RENDER_SCRIPT_H_

#include "RD_Composition.h"
#include "RD_ClipData.h"
#include "RD_RenderObjectManager.h"
#include "RD_RenderScriptMeta.h"

#include "UT_SharedWeakPtr.h"
#include "STD_Types.h"

class RD_Renderer;
class RD_RenderScript;

typedef UT_SharedPtr<RD_RenderScript> RD_RenderScriptPtr_t;
typedef UT_SharedWeakPtr<RD_RenderScript> RD_RenderScriptWeakPtr_t;

typedef RD_RenderObjectManager<int, RD_RenderScript> RD_RenderScriptManager;

/*!
 *  @class RD_RenderScript
 *  Rendering data for a single clip.
 */
class RD_RenderScript : public UT_SharedWeakBase
{
public:

  /*!
   *  @struct BoundingBox
   *  Basic bounding box container.
   */
  struct BoundingBox
  {
    float _x1, _y1;
    float _x2, _y2;
  };

  /*!
   *  @struct BoundingQuad
   *  Basic quad data structure.
   */
  struct BoundingQuad
  {
    float _x1, _y1;
    float _x2, _y2;
    float _x3, _y3;
    float _x4, _y4;
  };

  /*!
   *  @struct BoundingVertex
   *  Basic vertex for polygon data structure.
   */
  struct BoundingVertex
  {
    BoundingVertex( float x, float y ) :
      _x(x),
      _y(y)
    {
    }

    BoundingVertex() :
      _x(0.0f),
      _y(0.0f)
    {
    }

    BoundingVertex( const BoundingVertex &v ) :
      _x(v._x),
      _y(v._y)
    {
    }

    float _x, _y;
  };

  /*!
   *  @enum Feature
   *  Features implemented by current script.
   */
  enum Feature
  {
    eNullFeature                = 0,
    ePlainFeature               = 1<<0,
    eCutterFeature              = 1<<1,
    eDeformationFeature         = 1<<2
  };

public:

  RD_RenderScript();
  RD_RenderScript(const RD_RenderScriptMetaPtr_t &meta);
  virtual ~RD_RenderScript();

  //! Update render script.  Composition is updated at new frame and vertices/indices are
  //! updated with new values.  If clip data or sprite sheet resolution changes between two
  //! renders, the render script will be invalidated and the composition will be recalculated.
  virtual void update( const RD_ClipDataPtr_t &pClipData, const STD_String &projectFolder, const STD_String &sheetResolution, float frame, unsigned int color, int discretizationStep ) =0;
  //! Update render Script with Blending
  virtual void updateWithBlending( const RD_ClipDataPtr_t &pClipDataFrom, const STD_String &projectFolder, const STD_String &sheetResolution, float frameFrom, float frameTo, unsigned int color, int discretizationStep , float fullBlendTime, float currentBlendTime,int blendID ) =0;
  //! Render script with specified matrices.
  virtual void render( RD_Renderer *renderer, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &modelViewMatrix ) =0;

  //! Create a new prop entry in this render script
  int          createProp( const STD_String &name, const TV_NodeTreeViewPtr_t &nodeTreeView );
  //! Update specified prop entry to new frame.
  void         updateProp( int propId, float frame );
  //! Anchor specified prop entry to a node in parent hierarchy.
  void         anchorProp( int propId, const STD_String &name, const STD_String &nodeName );
  //! Unanchor specified prop entry.
  void         unanchorProp( int propId );

  //! Calculate axis aligned bounding box for current script.
  virtual void calculateBoundingBox( BoundingBox &box ) =0;

  //! Calculate a convex hull using current script aligned sprite boxes.
  virtual void calculateConvexHull( BoundingVertex* &convexHullArray, int &convexHullSize ) =0;
  //! Deallocate memory for convex hull.
  void         deallocateConvexHull( BoundingVertex* convexHullArray );

  //! Calculate a concave polygon using current script aligned sprite boxes.
  virtual void calculatePolygons( BoundingVertex* &polygonArray, int &polygonSize, int* &subPolygonArray, int &subPolygonSize ) =0;
  //! Deallocate memory for polygon.
  void         deallocatePolygons( BoundingVertex* polygonArray, int *subPolygonArray );

  //! Test if current script handle specified feature.
  //! Scripts can be optimized to only render certain features and thus restrict resources usage.
  virtual bool supportsFeature( Feature feature ) const = 0;

  //! Retrive meta container.
  const RD_RenderScriptMetaPtr_t &meta() const;

protected:

  unsigned		buildComposition( const RD_ClipDataPtr_t &pClipData, float frame, const STD_String &projectFolder, const STD_String &sheetResolution );
  unsigned		buildCompositionWithBlending( const RD_ClipDataPtr_t &pClipData, float frameFrom,float frameTo, const STD_String &projectFolder, const STD_String &sheetResolution, float fullBlendTime, float currentBlendTime,int blendID );
  void			updateComposition( float frame );
  void			updateCompositionWithBlending( float frameFrom, float frameTo, float fullBlendTime, float currentBlendTime,int blendID );
  void			updateRequests();

  void         lock();
  void         unlock();

  virtual void cleanup();

private:
  RD_RenderScript(const RD_RenderScript &); // not implemented...
  RD_RenderScript &operator=(const RD_RenderScript &); // not implemented...

protected:

  RD_RenderScriptMetaPtr_t _meta;

  typedef STD_Vector<RD_Composition*> CompositionCol_t;
  CompositionCol_t         _compositions;

  typedef STD_Map<int, RD_Composition*> PropCompositionCol_t;
  PropCompositionCol_t     _props;

  struct RenderBatchRequest
  {
    typedef RD_Composition::CompositionNodeCol_t::const_iterator Iterator_t;
    typedef STD_Pair< Iterator_t, Iterator_t > Range_t;

    RenderBatchRequest( RD_Composition *composition ) :
      _composition(composition),
      _range(STD_MakePair(composition->nodeBegin(), composition->nodeEnd()))
    {
    }

    RenderBatchRequest( RD_Composition *composition, Iterator_t first, Iterator_t second ) :
      _composition(composition),
      _range(STD_MakePair(first, second))
    {
    }

    RD_Composition *_composition;
    Range_t         _range;
  };

  typedef STD_List<RenderBatchRequest> RenderBatchRequestCol_t;
  RenderBatchRequestCol_t  _requests;

  PL_Mutex                 _mutex;
};

#endif /* _RD_RENDER_SCRIPT_H_ */
