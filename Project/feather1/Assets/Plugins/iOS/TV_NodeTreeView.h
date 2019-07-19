#ifndef _TV_NODE_TREE_VIEW_H_
#define _TV_NODE_TREE_VIEW_H_

#include "UT_ShareBase.h"
#include "UT_SharedPtr.h"
#include "MEM_Override.h"
#include "STD_Containers.h"
#include "STD_Types.h"

#include "TV_FloatDataView.h"
#include "TV_Pos3dDataView.h"
#include "TV_EffectDataView.h"

#include "TR_Types.h"

#include <string>
#include <vector>
#include <map>

class TV_NodeTreeView;

class TR_NodeTree;
class TR_AnimationDataObject;
class TR_DrawingAnimationDataObject;
class TR_DrawingDataObject;
class TR_DataObject;

namespace Math
{
  class Matrix4x4;
}

typedef UT_SharedPtr<TV_NodeTreeView> TV_NodeTreeViewPtr_t;

/*!
 * @class TV_NodeTreeView
 * Read-only view on node tree data structure.
 */
class TV_NodeTreeView : public UT_ShareBase
{
  MEM_OVERRIDE

  friend class BrotherIterator;
  friend class TV_EffectDataView;

public:

  class BrotherIterator
  {
    MEM_OVERRIDE

  public:

    BrotherIterator();
    BrotherIterator(TV_NodeTreeViewPtr_t nodeTreeView);
    ~BrotherIterator();

    BrotherIterator &operator++();
    BrotherIterator operator++(int);

    bool operator==(const BrotherIterator &it) const;
    bool operator!=(const BrotherIterator &it) const;

    const TV_NodeTreeView &operator*() const;
    TV_NodeTreeView &operator*();

    const TV_NodeTreeView *operator->() const;
    TV_NodeTreeView *operator->();

  private:

    TV_NodeTreeViewPtr_t _nodeTreeView;
  };

  typedef STD_Vector< TV_EffectDataViewPtr_t > EffectCol_t;
  typedef EffectCol_t::const_iterator EffectIterator;

public:

  static TV_NodeTreeViewPtr_t create( const TR_NodeTree *nodeTree );

  virtual ~TV_NodeTreeView();

  bool                   operator==(const TV_NodeTreeView &nodeTreeView) const;
  bool                   operator<(const TV_NodeTreeView &nodeTreeView) const;

  //! Validate that specified node tree and current node tree have the same root.
  bool                   hasCommonRoot(const TV_NodeTreeView &nodeTreeView) const;

  //! Retrieve parent node in the node tree hierarchy.
  TV_NodeTreeViewPtr_t   parent() const;

  //! Find a child node in the node tree hierarchy with specified name. 
  TV_NodeTreeViewPtr_t   find( const STD_String &nodeName ) const;

  //! Find a child node in the node tree hierarchy using specified predicate.
  template<typename Predicate>
  TV_NodeTreeViewPtr_t   findIf( const Predicate &pred ) const
  {
    if ( pred( *this ) )
    {
      return TV_NodeTreeViewPtr_t(const_cast<TV_NodeTreeView*>(this));
    }

    for ( BrotherIterator i = childBegin(), iEnd = childEnd() ; i!=iEnd ; ++i )
    {
      TV_NodeTreeViewPtr_t nodeTreeView = i->findIf( pred );
      if ( nodeTreeView.get() )
        return nodeTreeView;
    }

    return TV_NodeTreeViewPtr_t(0);
  }

  //! Retrieve first iterator on child nodes. 
  BrotherIterator        childBegin() const;
  //! Retrieve last iterator on child nodes.
  BrotherIterator        childEnd() const;

  //! Returns true if view contains valid node tree data. 
  bool                   isValid() const;

  //! Retrieve name of current node.
  STD_String             name() const;

  //! Retrieve animation total duration for current node tree. 
  float                  totalDuration() const;
  //! Retrieve current node depth in the node tree.
  unsigned               depth() const;

  /*!
   * Animated Data
   */
  // @{
  //! Returns true if current node has animation data.
  bool                   hasAnimation() const;
  //! Returns true if current node has deformation data.
  bool                   hasDeformation() const;

  //! Retrieve view on x axis offset animation channel.
  TV_FloatDataViewPtr_t  separateX() const;
  //! Retrieve view on y axis offset animation channel.
  TV_FloatDataViewPtr_t  separateY() const;
  //! Retrieve view on z axis offset animation channel.
  TV_FloatDataViewPtr_t  separateZ() const;

  //! Retrieve view on xyz offset animation channel.
  TV_Pos3dDataViewPtr_t  position() const;

  //! Retrieve view on x axis scale animation channel.
  TV_FloatDataViewPtr_t  scaleX() const;
  //! Retrieve view on y axis scale animation channel.
  TV_FloatDataViewPtr_t  scaleY() const;
  //! Retrieve view on uniform xy axis scale animation channel.
  TV_FloatDataViewPtr_t  scaleXY() const;

  //! Retrieve view on z axis rotation animation channel.
  TV_FloatDataViewPtr_t  rotationZ() const;

  //! Retrieve view on skew animation channel.
  TV_FloatDataViewPtr_t  skew() const;

  //! Retrieve view on pivot animation channel.
  TV_Pos3dDataViewPtr_t  pivot() const;

  //! Retrieve view on opacity animation channel.
  TV_FloatDataViewPtr_t  opacity() const;

  //! Calculate position at specified frame.
  void                   position(float frame, float &offsetx, float &offsety, float &offsetz) const;
  //! Calculate scaling at specified frame.
  void                   scale(float frame, float &scalex, float &scaley) const;
  //! Calculate rotation at specfied frame.
  void                   rotation(float frame, float &rotationz) const;
  //! Calculate skewing at specified frame.
  void                   skew(float frame, float &skew) const;
  //! Calculate pivot at specified frame.
  void                   pivot(float frame, float &pivotx, float &pivoty, float &pivotz) const;
  //! Calculate opacity at specified frame.
  void                   opacity(float frame, float &opacity) const;

  //! Calculate local matrix at current frame.
  const Math::Matrix4x4 &localMatrix( float frame ) const;
  const Math::Matrix4x4 &localMatrixWithBlending( float frameFrom, float frameTo, float fullBlendTime, float currentBlendTime, int blendID ) const;
  //! Calculate model matrix at current frame.
  const Math::Matrix4x4 &modelMatrix( float frame ) const;
  const Math::Matrix4x4 &modelMatrixWithBlending( float frameFrom, float frameTo,  float fullBlendTime, float currentBlendTime, int blendID ) const;
  // @}

  /*!
   * Drawing data
   */
  // @{
  //! Returns true if current node has sprite animation.
  bool                   hasDrawingAnimation() const;

  //! Retrieve sprite sheet name associated with this node tree.
  bool                   spriteSheet( STD_String &spriteSheetName ) const;

  //! Retrieve the number of sprites set in current node sprite animation.
  unsigned               nSprites() const;

  //! Retrieve sprite name, start time and duration of exposure at specified index. 
  bool                   sprite( unsigned idx, STD_String &spriteName, float &start, float &duration ) const;
  //! Retrieve sprite name at specified frame.
  bool                   sprite( float frame, STD_String &spriteName ) const;
  // @}

  /*!
   * Effect Data
   */
  //! Retrieve first iterator on associated effect node trees.
  EffectIterator         effectBegin() const;
  //! Retrieve last iterator on associated effect node trees.
  EffectIterator         effectEnd() const;

  /*!
   * Deformation Data
   */
  // @{
  //! Retrieve resting bone offset at specified frame.
  void                   restOffset( float frame, float &offsetx, float &offsety) const;
  //! Retrieve resting bone length at specified frame.
  void                   restLength( float frame, float &length ) const;
  //! Retrieve resting bone rotation at specified frame.
  void                   restRotation( float frame, float &rotation ) const;

  //! Retrieve deformed bone offset at specified frame.
  void                   deformOffset( float frame, float &offsetx, float &offsety ) const;
  //! Retrieve deformed bone length at specified frame.
  void                   deformLength( float frame, float &length ) const;
  //! Retrieve deformed bone rotation at specified frame.
  void                   deformRotation( float frame, float &rotation ) const;

  //! Retrieve resting bone start transform.
  const Math::Matrix4x4 &restStartMatrix( float frame ) const;
  //! Retrieve resting bone end transform.
  const Math::Matrix4x4 &restEndMatrix( float frame ) const;

  //! Retrieve deformed bone start transform.
  const Math::Matrix4x4 &deformStartMatrix( float frame ) const;
  //! Retrieve deformed bone end transform.
  const Math::Matrix4x4 &deformEndMatrix( float frame ) const;

  //

  //! Retrieve resting bone start transform.
  const Math::Matrix4x4 &restStartMatrixWithBlending( float frameFrom,float frameTo, float fullBlendTime, float currentBlendTime, int blendID ) const;
  //! Retrieve resting bone end transform.
  const Math::Matrix4x4 &restEndMatrixWithBlending( float frameFrom,float frameTo, float fullBlendTime, float currentBlendTime, int blendID ) const;

  //! Retrieve deformed bone start transform.
  const Math::Matrix4x4 &deformStartMatrixWithBlending( float frameFrom,float frameTo, float fullBlendTime, float currentBlendTime, int blendID ) const;
  //! Retrieve deformed bone end transform.
  const Math::Matrix4x4 &deformEndMatrixWithBlending( float frameFrom,float frameTo, float fullBlendTime, float currentBlendTime, int blendID ) const;

  //

  // @}

private:

  void createChannels();
  bool createChannel( TR_Types::CurveChannel_t channelType, TR_Types::DataRef_t dataRef );

private:

  const TR_DrawingAnimationDataObject *drawingAnimationData() const;

  TV_FloatDataViewPtr_t  floatChannel( TR_Types::CurveChannel_t channel ) const;
  TV_Pos3dDataViewPtr_t  pos3dChannel( TR_Types::CurveChannel_t channel ) const;

  const TR_DrawingDataObject *drawingAt( float frame ) const;
  const TR_DrawingDataObject *drawingAt( unsigned idx ) const;

private:
  TV_NodeTreeView( const TR_NodeTree *nodeTree, TR_Types::DataRef_t nodeRef, TV_NodeTreeViewPtr_t parentNode, TV_EffectDataViewPtr_t effectData = 0, TR_Types::EffectId_t matteId = TR_Types::eNoop );

  //  copy is not implemented.
  TV_NodeTreeView( const TV_NodeTreeView & );
  TV_NodeTreeView &operator= ( const TV_NodeTreeView & );

private:

  class Impl;
  Impl *_i;
};

#endif /* _TV_NODE_TREE_VIEW_H_ */
