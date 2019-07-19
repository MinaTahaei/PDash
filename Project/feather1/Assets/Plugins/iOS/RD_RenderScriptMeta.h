
#ifndef _RD_RENDER_SCRIPT_META_H_
#define _RD_RENDER_SCRIPT_META_H_

#include "TV_NodeTreeView.h"

#include "UT_ShareBase.h"
#include "UT_SharedPtr.h"

#include "STD_Types.h"
#include "STD_Containers.h"

class RD_RenderScriptMeta;

typedef UT_SharedPtr<RD_RenderScriptMeta> RD_RenderScriptMetaPtr_t;

/*!
 *  @class RD_RenderScriptMeta
 *  Container for meta information that can modify a render script
 *  default rendering.
 */
class RD_RenderScriptMeta : public UT_ShareBase
{
public:

  /*!
   *  @struct PropData
   */
  struct PropData
  {
    PropData(const STD_String &name, const TV_NodeTreeViewPtr_t &nodeTreeView) :
      _name(name),
      _nodeTreeView(nodeTreeView),
      _frame(1.0f)
    {
    }

    STD_String           _name;
    TV_NodeTreeViewPtr_t _nodeTreeView;
    float                _frame;
  };

  /*!
   *  @struct AnchorData
   */
  struct AnchorData
  {
    AnchorData(const STD_String &name, const STD_String &nodeName) :
      _name(name),
      _nodeName(nodeName)
    {
    }

    STD_String _name;
    STD_String _nodeName;
  };

  typedef STD_Pair<PropData*, AnchorData*> PropAnchorDataPair_t;

  typedef STD_Map<int, PropData*> PropDataCol_t;
  typedef STD_Map<int, PropAnchorDataPair_t> PropAnchorDataCol_t;

public:

  RD_RenderScriptMeta();
  virtual ~RD_RenderScriptMeta();

  //! Create a new prop entry.
  int  createProp( const STD_String &name, const TV_NodeTreeViewPtr_t &nodeTreeView );
  //! Update specified prop entry to new frame.
  void updateProp( int propId, float frame );
  //! Anchor specified prop entry to a node in parent hierarchy.
  void anchorProp( int propId, const STD_String &name, const STD_String &nodeName );
  //! Unanchor specified prop entry.
  void unanchorProp( int propId );

  //! Retrieve specified prop entry current frame.
  float frame( int propId ) const;

  //! Retrieve anchor collection.
  const PropAnchorDataCol_t &anchors() const;

  //! Toggle specified node visibility. 
  void setNodeVisibility( const STD_String &name, const STD_String &nodeName, bool visible );
  //! Clear visibility flags.
  void clearVisibilityFlags();

private:

  int                 _idCount;

  PropDataCol_t       _props;
  PropAnchorDataCol_t _anchors;
};

#endif /* _RD_RENDER_SCRIPT_META_H_ */
