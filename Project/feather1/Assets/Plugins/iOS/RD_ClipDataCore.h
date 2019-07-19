#ifndef _RD_CLIP_DATA_CORE_H_
#define _RD_CLIP_DATA_CORE_H_

#include "TV_NodeTreeView.h"

#include "MEM_Override.h"
#include "STD_Types.h"

class TR_NodeTree;
class IO_PersistentStore;

/*!
 *  @class RD_ClipDataCore
 *  Clip data container
 */
class RD_ClipDataCore
{
  MEM_OVERRIDE

  friend class XML_StageSaxParser;

public:

  RD_ClipDataCore();
  virtual ~RD_ClipDataCore();

  //! Build a node tree view data structure for node tree at index.
  TV_NodeTreeViewPtr_t nodeTreeView( unsigned idx ) const;
  //! Build a node tree view data structure for named node tree.
  TV_NodeTreeViewPtr_t nodeTreeView( const STD_String &name ) const;

  //! Retrieve node tree name at index.
  const STD_String    &name( unsigned idx ) const;

  //! Retrieve number of node trees in current clip data.
  size_t count() const;

  //! Store clip data to stream.
  void store( IO_PersistentStore &store ) const;
  //! Load clip data from stream.
  bool load( IO_PersistentStore &store );

  //! Returns true if clip has effects (Deformation, Cutters).
  bool  fxEnabled() const;
  //! Retrieve clip animation total duration.
  float totalDuration() const;

private:

  void beginClipData();
  void addNodeTree ( TR_NodeTree *, const STD_String &name );
  void addSoundEvent( const STD_String &soundName, float startFrame );
  void endClipData();

  void storeHeader( IO_PersistentStore &store ) const;
  bool loadHeader( IO_PersistentStore &store ) const;

  void updateFxFlag();
  void eraseAll();

private:

  class Impl;
  Impl *_i;
};

#endif /* _RD_CLIP_DATA_CORE_H_ */
