#ifndef _TR_NODE_TREE_BUILDER_H_
#define _TR_NODE_TREE_BUILDER_H_

#include "TR_NodeTree.h"
#include "TR_Types.h"

#include "MEM_Override.h"
#include "STD_Types.h"

/*!
 *  @class TR_NodeTreeBuilder
 *  Builder for Node Tree data structure.
 */
class TR_NodeTreeBuilder
{
  MEM_OVERRIDE

public:

  typedef TR_NodeTree::Data_t Data_t;

public:

  TR_NodeTreeBuilder( TR_NodeTree *nodeTree );
  virtual ~TR_NodeTreeBuilder();

  TR_Types::DataRef_t addNode( const STD_String &name );
  TR_Types::DataRef_t addNode( const STD_String &name, TR_Types::DataRef_t parentNodeRef );

  //! @name Reusing data structures
  // @{
  void                linkNodeAnimation( TR_Types::DataRef_t srcNodeRef, TR_Types::DataRef_t dstNodeRef );
  void                linkChannel( TR_Types::DataRef_t srcChannelRef, TR_Types::DataRef_t dstChannelRef );

  void                linkNodeDrawingAnimation( TR_Types::DataRef_t srcNodeRef, TR_Types::DataRef_t dstNodeRef );

  TR_Types::DataRef_t createEmptyChannel( TR_Types::CurveChannel_t channel, TR_Types::DataRef_t nodeRef );
  // @}

  //! @name Managing effects
  // @{
  bool                createEffect( TR_Types::DataRef_t nodeRef, TR_Types::EffectId_t effectId );

  TR_Types::DataRef_t addMatteNode( const STD_String &name, TR_Types::DataRef_t nodeRef );
  // @}

  //! @name Handling bezier curve
  // @{
  TR_Types::DataRef_t beginBezierCurve( TR_Types::CurveChannel_t channel, TR_Types::DataRef_t nodeRef );
  void                endBezierCurve();

  void                addBezierPoint( float x, float y, float leftx=0.0f, float lefty=0.0f, float rightx=0.0f, float righty=0.0f, bool constSeg = false );
  // @}

  //! @name Handling catmull curve
  // @{
  TR_Types::DataRef_t beginCatmullCurve( TR_Types::CurveChannel_t channel, TR_Types::DataRef_t nodeRef, float scaleX = 1.0f, float scaleY = 1.0f, float scaleZ = 1.0f );
  void                endCatmullCurve();

  void                addCatmullPoint( float time, float x, float y, float z, float tension = 0.0f, float continuity = 0.0f, float bias = 0.0f );
  // @}

  //! @name Handling catmull curve
  // @{
  TR_Types::DataRef_t beginLinearCurve( TR_Types::CurveChannel_t channel, TR_Types::DataRef_t nodeRef );
  void                endLinearCurve();

  void                addLinearPoint( float x, float y );
  // @}

  //! @name Handling animated pivot
  // @{
  TR_Types::DataRef_t beginAnimatedPivot( TR_Types::CurveChannel_t channel, TR_Types::DataRef_t nodeRef );
  void                endAnimatedPivot();

  void                addPivotPoint( float time, float x, float y, float z );
  // @}

  //! Constant value for all frames.
  TR_Types::DataRef_t addConstantValue( TR_Types::CurveChannel_t channel, TR_Types::DataRef_t nodeRef, float value );

  //! @name Handling drawing animation
  // @{
  bool                beginDrawingSequence( TR_Types::DataRef_t nodeRef );
  void                endDrawingSequence();

  void                addDrawing( const STD_String &spriteSheetName, const STD_String &spriteName, float time, float repeat );
  // @}

protected:

  TR_EffectDataObject *retrieveEffectData( TR_Types::DataRef_t nodeRef ) const;

  TR_Types::DataRef_t createChannelData( TR_Types::CurveChannel_t channelType, TR_Types::DataRef_t nodeRef );

  TR_Types::DataRef_t retrieveDrawingAnimationData( TR_Types::DataRef_t nodeRef );
  TR_Types::DataRef_t createDrawingAnimationData( TR_Types::DataRef_t nodeRef );

  TR_CatmullPointDataObject *pointAt(TR_CatmullCurveDataObject *catmullData, unsigned idx);
  void computeCatmullDistances( TR_Types::DataRef_t curveRef );
  void computeCatmullDerivatives( TR_Types::DataRef_t curveRef );

private:

  //  Do not allow copy.
  TR_NodeTreeBuilder( const TR_NodeTreeBuilder& );
  TR_NodeTreeBuilder &operator=( const TR_NodeTreeBuilder& );

  class Impl;
  Impl *_i;
};

#endif /* _TR_NODE_TREE_BUILDER_H_ */
