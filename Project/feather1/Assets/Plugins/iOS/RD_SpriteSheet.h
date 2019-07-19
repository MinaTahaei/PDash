#ifndef _RD_SPRITE_SHEET_H_
#define _RD_SPRITE_SHEET_H_

#include "RD_RenderObjectManager.h"

#include "UT_SharedWeakPtr.h"
#include "STD_Types.h"

#include "RD_SpriteSheetCore.h"
#include "IM_Image.h"

// typedef void* RD_TexturePtr_t

class RD_SpriteSheet;
typedef UT_SharedPtr< RD_SpriteSheet > RD_SpriteSheetPtr_t;
typedef UT_SharedWeakPtr< RD_SpriteSheet > RD_SpriteSheetWeakPtr_t;

/*!
 *  @struct RD_SpriteSheetKey
 *  Tuple to identify a unique sprite sheet.
 */
struct RD_SpriteSheetKey
{
  RD_SpriteSheetKey( const STD_String &projectFolder, const STD_String &sheetName, const STD_String &resolutionName ) :
    _projectFolder(projectFolder),
    _sheetName(sheetName),
    _resolutionName(resolutionName)
  {
  }

  bool operator< ( const RD_SpriteSheetKey &key ) const
  {
    //  To be reviewed, sorting could probably be improved if
    //  it becomes a bottleneck.
    if ( _projectFolder.compare(key._projectFolder) == 0 )
    {
      if ( _sheetName.compare(key._sheetName) == 0 )
      {
        return _resolutionName < key._resolutionName;
      }

      return ( _sheetName < key._sheetName );
    }

    return _projectFolder < key._projectFolder;
  }

  STD_String _projectFolder;
  STD_String _sheetName;
  STD_String _resolutionName;
};

typedef RD_RenderObjectManager<RD_SpriteSheetKey, RD_SpriteSheet> RD_SpriteSheetManager;

/*!
 *  @class RD_SpriteSheet
 *  Sprite Sheet data structure.
 */
class RD_SpriteSheet : public UT_SharedWeakBase
{
public:

  typedef RD_SpriteSheetCore::Rect Rect;
  typedef RD_SpriteSheetCore::SpriteData SpriteData;

public:
  static RD_SpriteSheetPtr_t create( const RD_SpriteSheetKey &key );
  static RD_SpriteSheetPtr_t createOrLoad( const RD_SpriteSheetKey &key );

private:
  RD_SpriteSheet( const STD_String &projectFolder, RD_SpriteSheetCore *spriteSheet );

public:
  virtual ~RD_SpriteSheet();

  //! Load sprite sheet image.
  bool buildSpriteSheet();
  //! Free sprite sheet image.
  void freeSpriteSheetData();

  //! Retrieve specified named sprite in sprite sheet.
  const SpriteData *sprite( const STD_String &name ) const;

  //! Retrieve specified sprite rectangle in sprite sheet.
  bool rect( const STD_String &name, Rect &sprite ) const;

  //! Retrieve specified sprite matrix in sprite sheet.
  bool matrix( const STD_String &name, Math::Matrix4x4 &matrix ) const;

  //! Retrieve project folder.
  const STD_String &projectFolder() const;
  //! Retrieve sprite sheet filename.
  STD_String        sheetFilename() const;
  //! Retrieve sprite sheet name.
  const STD_String &sheetName() const;
  //! Retrieve sprite sheet resolution.
  const STD_String &sheetResolution() const;

  //! Retrieve texture uvs of specified sprite.
  bool uvs( const STD_String &name, float &u1, float &v1, float &u2, float &v2 ) const;
  //! Retrieve texture uvs of specified sprite.
  bool uvs( const STD_String &name, float uvs[4] ) const;

  //! Convert rectangle coordinates to texture uvs.
  bool uvs( const Rect &rect, float &u1, float &v1, float &u2, float &v2 ) const;
  //! Convert rectangle coordinates to texture uvs.
  bool uvs( const Rect &rect, float uvs[4] ) const;

  //! Returns true if sprite sheet has its associated image loaded.
  bool hasImage() const;
  //! Retrieve sprite sheet image.
  const IM_ImagePtr_t &image() const;

  //! Retrieve sprite sheet image width.
  unsigned width() const;
  //! Retrieve sprite sheet image height.
  unsigned height() const;

private:

  class Impl;
  Impl *_i;
};

#endif /* _RD_SPRITE_SHEET_H_ */
