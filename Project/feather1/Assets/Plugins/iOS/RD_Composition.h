
#ifndef _RD_COMPOSITION_DATA_H_
#define _RD_COMPOSITION_DATA_H_

#include "TV_NodeTreeView.h"
#include "RD_SpriteSheet.h"
#include "RD_ClipData.h"
#include "MT_Matrix4x4.h"
#include "STD_Types.h"
#include "STD_Containers.h"

/*!
 *  @class RD_Composition
 *  Single Batch commposition.
 */
class RD_Composition
{
public:

  struct CutterEffect;
  typedef STD_Vector< CutterEffect* > CutterEffectCol_t;

  /*!
   *  @struct CutterEffect
   *  Cutter Effect applied to one or several composition nodes.
   */
  struct CutterEffect
  {
    TV_EffectDataViewPtr_t        _effectDataView;
    TV_NodeTreeViewPtr_t          _nodeTreeView;

    STD_String                    _spriteName;
    Math::Matrix4x4               _matrix;

    unsigned                      _effectIdx;
  };

  struct BoneEffect;
  typedef STD_Vector< BoneEffect* > BoneEffectCol_t;

  /*!
   *  @struct BoneEffect
   *  Deformation Effect applied to one or several composition nodes.
   */
  struct BoneEffect
  {
    TV_EffectDataViewPtr_t        _effectDataView;
    TV_NodeTreeViewPtr_t          _nodeTreeView;

    BoneEffect                   *_parentBone;
    BoneEffectCol_t               _childBones;

    Math::Point3d                 _restStart;
    Math::Point3d                 _restEnd;

    Math::Point3d                 _deformStart;
    Math::Point3d                 _deformEnd;

    Math::Matrix4x4               _preMatrix;
    Math::Matrix4x4               _matrix;
    Math::Matrix4x4               _postMatrix;

    unsigned                      _effectIdx;
  };

  struct CompositionNode;
  typedef STD_Vector< CompositionNode* > CompositionNodeCol_t;

  /*!
   *  @struct CompositionNode
   *  Sprite node that is part of composition.
   */
  struct CompositionNode
  {
    TV_NodeTreeViewPtr_t          _nodeTreeView;

    STD_String                    _spriteName;
    Math::Matrix4x4               _matrix;

    float                         _opacity;

    double                        _offsetZ;
    unsigned                      _orderIdx;

    CutterEffectCol_t             _cutters;
    BoneEffectCol_t               _bones;

    bool operator<( const CompositionNode &node ) const
    {
      // We sort layers using z offset and composition ordering.
      // A layer with lower z value will be rendered before.
      // If two layers have the same z offset, then, layer with
      // the higher order index will be rendered before.

      if ( _offsetZ == node._offsetZ )
      {
        return _orderIdx > node._orderIdx;
      }

      return ( _offsetZ < node._offsetZ );
    }
  };

public:

  RD_Composition( const STD_String &name, const RD_SpriteSheetPtr_t &spriteSheet, const TV_NodeTreeViewPtr_t &nodeTreeView, const TV_NodeTreeViewPtr_t &parentNodeTreeView = 0 );
  virtual ~RD_Composition();

  //! Build a new composition at specified frame. 
  void build( float frame, float parentFrame = 0.0f );
  void buildWithBlending( float frameFrom,float frameTo,float fullBlendTime, float currentBlendTime,int blendID, float parentFrame = 0.0f );  
  //! Update existing composition to specified frame.
  void update( float frame, float parentFrame = 0.0f );
  void updateWithBlending( float frameFrom, float frameTo, float parentFrame, float fullBlendTime, float currentBlendTime,int blendID );

  //! Retrieve first iterator on composition nodes.
  CompositionNodeCol_t::const_iterator nodeBegin() const;
  //! Retrieve last iterator on composition nodes.
  CompositionNodeCol_t::const_iterator nodeEnd() const;

  //! Retrieve number of composition nodes.
  unsigned nodeCount() const;

  //! Retrieve first iterator on cutter effects.
  CutterEffectCol_t::const_iterator cutterBegin() const;
  //! Retrieve last iterator on cutter effects.
  CutterEffectCol_t::const_iterator cutterEnd() const;

  //! Retrieve number of cutter effects.
  unsigned cutterCount() const;

  //! Retrieve first iterator on bone effects. 
  BoneEffectCol_t::const_iterator boneBegin() const;
  //! Retrieve last iterator on bone effects.
  BoneEffectCol_t::const_iterator boneEnd() const;

  //! Retrieve number of bone effects.
  unsigned boneCount() const;

  //! Retrieve composition name.
  const STD_String &name() const;
  //! Retrieve sprite sheet.
  const RD_SpriteSheetPtr_t &spriteSheet() const;
  //! Retrieve parent node tree.  Can be null.
  const TV_NodeTreeViewPtr_t &parentNodeTreeView() const;
  //! Retrieve node tree.
  const TV_NodeTreeViewPtr_t &nodeTreeView() const;

protected:

  void cleanup();

private:

  //  Not implemented.
  RD_Composition(const RD_Composition &);
  RD_Composition &operator=(const RD_Composition &);

private:

  class Impl;
  Impl *_i;

};


#endif /* _RD_COMPOSITION_DATA_H_ */
