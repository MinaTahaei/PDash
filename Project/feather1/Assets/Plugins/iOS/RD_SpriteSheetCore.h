#ifndef _RD_SPRITE_SHEET_CORE_H_
#define _RD_SPRITE_SHEET_CORE_H_

#include "MT_Matrix4x4.h"
#include "MEM_Override.h"
#include "STD_Containers.h"
#include "STD_Types.h"

#include <string>

class RD_SpriteSheetCore;
class IO_PersistentStore;

/*!
 * @class RD_SpriteSheetCore
 * Maps single texture into several sprites.
 */
class RD_SpriteSheetCore
{
  MEM_OVERRIDE

  friend class XML_SpriteSheetSaxParser;

public:

  /*!
   * @struct Rect
   */
  struct Rect
  {
    MEM_OVERRIDE

    int _x, _y;
    int _w, _h;
  };

  /*!
   * @struct SpriteData
   */
  struct SpriteData
  {
    MEM_OVERRIDE

    Rect            _rect;   //!< Sprite coordinates in the sprite sheet.
    Math::Matrix4x4 _matrix; //!< Local sprite transform.
  };

  typedef STD_Map< STD_String, SpriteData > SpriteCol_t;

public:

  RD_SpriteSheetCore();
  virtual ~RD_SpriteSheetCore();

  //! Retrieve sprite sheet filename.
  const STD_String &sheetFilename() const;
  //! Retrieve sprite sheet name.
  const STD_String &sheetName() const;
  //! Retrieve sprite sheet resolution name.
  const STD_String &sheetResolution() const;

public:

  //! Retrieve named sprite data.
  const SpriteData *sprite( const STD_String &name ) const;

  //! Retrieve named sprite coordinates in the sprite sheet.
  bool rect( const STD_String &name, Rect &sprite ) const;
  //! Retrieve named sprite matrix.
  bool matrix( const STD_String &name, Math::Matrix4x4 &matrix ) const;

  //! Retrieve number of sprites in the sprite sheet.
  size_t size() const;

  //! Returns true if there are no sprites in the sprite sheet.
  bool empty() const;

  //! Store sprite sheet data to stream.
  void store( IO_PersistentStore &store ) const;
  //! Load sprite sheet data from stream.
  bool load( IO_PersistentStore &store );

public:

  //! Retrieve first iterator on sprite sheet data.
  SpriteCol_t::const_iterator begin() const;
  //! Retrieve last iterator on sprite sheet data.
  SpriteCol_t::const_iterator end() const;

protected:

  void storeHeader( IO_PersistentStore &store ) const;
  bool loadHeader( IO_PersistentStore &store ) const;

  void addSprite( const STD_String &name, const Rect &rect, float offsetX, float offsetY, float scaleX, float scaleY , float angleDegrees);
  void addSprite( const STD_String &name, const Rect &rect, const Math::Matrix4x4 &matrix );

private:

  STD_String   _projectFolder;
  STD_String   _sheetFilename;
  STD_String   _sheetName;
  STD_String   _sheetResolution;

  SpriteCol_t  _sprites;

};

#endif /* _RD_SPRITE_SHEET_CORE_H_ */
