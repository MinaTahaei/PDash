#ifndef _BIN_PROJECT_LOADER_H_
#define _BIN_PROJECT_LOADER_H_

#include "MEM_Override.h"
#include "STD_Containers.h"
#include "STD_Types.h"

#include <string>

class RD_ClipDataCore;
class RD_SpriteSheetCore;

/*!
 *  @namespace BIN_ProjectLoader
 */
namespace BIN_ProjectLoader
{
  typedef STD_Vector< STD_String > StringCol_t;

  typedef STD_Pair< STD_String, STD_String > StringPair_t;
  typedef STD_Vector< StringPair_t > StringPairCol_t;

  //! Build a list of all stage clips in specified project folder.
  void loadStageClipNames( const STD_String &projectFolder, StringCol_t &clipNames );
  //! Load specified stage clip to clip data structure.
  bool loadStageClip( const STD_String &projectFolder, const STD_String &clipName, RD_ClipDataCore *clipData );

  //! Build a list of all sprite sheet names in specified project folder.
  void loadSpriteSheetNames( const STD_String &projectFolder, StringPairCol_t &sheetNames );
  //! Build a list of all sprite sheet resolution names for specified sprite sheet.
  void loadSpriteSheetResolutionNames( const STD_String &projectFolder, const STD_String &sheetName, StringPairCol_t &resolutionNames );

  //! Load specified sprite sheet with sheet resolution to sprite sheet data structure.
  bool loadSpriteSheet( const STD_String &projectFolder, const STD_String &sheetName, const STD_String &resolutionName, RD_SpriteSheetCore *sheet );
  //! Load specified sprite sheet with default sheet resolution to sprite sheet data structure.
  bool loadSpriteSheet( const STD_String &projectFolder, const STD_String &sheetName, RD_SpriteSheetCore *sheet );
}

#endif /* _BIN_PROJECT_LOADER_H_ */
