#ifndef _RD_CLIP_DATA_H_
#define _RD_CLIP_DATA_H_

#include "RD_RenderObjectManager.h"

#include "UT_ShareBaseSafe.h"
#include "UT_SharedPtr.h"
#include "STD_Types.h"

#include "RD_ClipDataCore.h"

class TR_NodeTree;

class RD_ClipData;
typedef UT_SharedPtr<RD_ClipData> RD_ClipDataPtr_t;

/*!
 *  @struct RD_ClipDataKey
 *  Tuple to identify a unique clip.
 */
struct RD_ClipDataKey
{
  RD_ClipDataKey( const STD_String &projectFolder, const STD_String &clipName ) :
    _projectFolder(projectFolder),
    _clipName(clipName)
  {
  }

  bool operator< ( const RD_ClipDataKey &key ) const
  {
    //  To be reviewed, sorting could probably be improved if
    //  it becomes a bottleneck.
    if ( _projectFolder.compare(key._projectFolder) == 0 )
    {
      return ( _clipName < key._clipName );
    }

    return _projectFolder < key._projectFolder;
  }

  STD_String _projectFolder;
  STD_String _clipName;
};

typedef RD_RenderObjectManager<RD_ClipDataKey, RD_ClipData> RD_ClipDataManager;

/*!
 *  @class RD_ClipData
 *  Clip data structure.
 */
class RD_ClipData : public UT_ShareBaseSafe
{
public:

  RD_ClipData( RD_ClipDataCore *pClipData );
  virtual ~RD_ClipData();

  //! Build a node tree view data structure for node tree at index.
  TV_NodeTreeViewPtr_t nodeTreeView( unsigned idx ) const;
  //! Build a node tree view data structure for named node tree.
  TV_NodeTreeViewPtr_t nodeTreeView( const STD_String &name ) const;

  //! Retrieve node tree name at index.
  const STD_String    &name( unsigned idx ) const;

  //! Retrieve number of node trees in current clip data.
  size_t count() const;

  //! Returns true if clip has effects (Deformation, Cutters).
  bool  fxEnabled() const;
  //! Retrieve clip animation total duration.
  float totalDuration() const;

private:

  class Impl;
  Impl *_i;
};

#endif /* _RD_CLIP_DATA_H_ */
