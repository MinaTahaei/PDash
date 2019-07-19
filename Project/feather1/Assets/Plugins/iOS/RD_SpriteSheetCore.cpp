#include "RD_SpriteSheetCore.h"
#include "XML_ProjectLoader.h"
#include "XML_SpriteSheetSaxParser.h"

#include "IO_PersistentStore.h"
#include "STD_Types.h"

#define RD_SpriteSheetTag             "SPSD"
#define RD_SpriteSheetMajorVersion    (int)3
#define RD_SpriteSheetMinorVersion    (int)0

RD_SpriteSheetCore::RD_SpriteSheetCore()
{
}

RD_SpriteSheetCore::~RD_SpriteSheetCore()
{
}

const STD_String &RD_SpriteSheetCore::sheetFilename() const
{
  return _sheetFilename;
}

const STD_String &RD_SpriteSheetCore::sheetName() const
{
  return _sheetName;
}

const STD_String &RD_SpriteSheetCore::sheetResolution() const
{
  return _sheetResolution;
}

const RD_SpriteSheetCore::SpriteData *RD_SpriteSheetCore::sprite( const STD_String &name ) const
{
  SpriteCol_t::const_iterator i = _sprites.find( name );
  if ( i != _sprites.end() )
  {
    return &i->second;
  }

  return NULL;
}

bool RD_SpriteSheetCore::rect( const STD_String &name, Rect &rect ) const
{
  SpriteCol_t::const_iterator i = _sprites.find( name );
  if ( i != _sprites.end() )
  {
    rect = i->second._rect;
    return true;
  }

  return false;
}

bool RD_SpriteSheetCore::matrix( const STD_String &name, Math::Matrix4x4 &matrix ) const
{
  SpriteCol_t::const_iterator i = _sprites.find( name );
  if ( i != _sprites.end() )
  {
    matrix = i->second._matrix;
    return true;
  }

  return false;
}

size_t RD_SpriteSheetCore::size() const
{
  return _sprites.size();
}

bool RD_SpriteSheetCore::empty() const
{
  return (size() == 0);
}

void RD_SpriteSheetCore::store( IO_PersistentStore &store ) const
{
  storeHeader( store );

  store << _sheetFilename;
  store << _sheetName;
  store << _sheetResolution;

  store << (int)_sprites.size();
  for ( SpriteCol_t::const_iterator i=_sprites.begin(), iEnd=_sprites.end() ; i!=iEnd ; ++i )
  {
    store << i->first;

    store << i->second._rect._x << i->second._rect._y << i->second._rect._w << i->second._rect._h;

    store << i->second._matrix(Math::Row_0, Math::Column_0) << i->second._matrix(Math::Row_0, Math::Column_1)
          << i->second._matrix(Math::Row_0, Math::Column_2) << i->second._matrix(Math::Row_0, Math::Column_3)
          << i->second._matrix(Math::Row_1, Math::Column_0) << i->second._matrix(Math::Row_1, Math::Column_1)
          << i->second._matrix(Math::Row_1, Math::Column_2) << i->second._matrix(Math::Row_1, Math::Column_3)
          << i->second._matrix(Math::Row_2, Math::Column_0) << i->second._matrix(Math::Row_2, Math::Column_1)
          << i->second._matrix(Math::Row_2, Math::Column_2) << i->second._matrix(Math::Row_2, Math::Column_3)
          << i->second._matrix(Math::Row_3, Math::Column_0) << i->second._matrix(Math::Row_3, Math::Column_1)
          << i->second._matrix(Math::Row_3, Math::Column_2) << i->second._matrix(Math::Row_3, Math::Column_3);
  }
}

bool RD_SpriteSheetCore::load( IO_PersistentStore &store )
{
  if ( !loadHeader( store ) )
    return false;

  store >> _sheetFilename;
  store >> _sheetName;
  store >> _sheetResolution;

  int nSprites;
  store >> nSprites;

  while ( nSprites-- )
  {
    STD_String spriteName;
    store >> spriteName;

    Rect rect;
    store >> rect._x >> rect._y >> rect._w >> rect._h;

    Math::Matrix4x4 matrix;
    store >> matrix(Math::Row_0, Math::Column_0) >> matrix(Math::Row_0, Math::Column_1)
          >> matrix(Math::Row_0, Math::Column_2) >> matrix(Math::Row_0, Math::Column_3)
          >> matrix(Math::Row_1, Math::Column_0) >> matrix(Math::Row_1, Math::Column_1)
          >> matrix(Math::Row_1, Math::Column_2) >> matrix(Math::Row_1, Math::Column_3)
          >> matrix(Math::Row_2, Math::Column_0) >> matrix(Math::Row_2, Math::Column_1)
          >> matrix(Math::Row_2, Math::Column_2) >> matrix(Math::Row_2, Math::Column_3)
          >> matrix(Math::Row_3, Math::Column_0) >> matrix(Math::Row_3, Math::Column_1)
          >> matrix(Math::Row_3, Math::Column_2) >> matrix(Math::Row_3, Math::Column_3);

    addSprite( spriteName, rect, matrix );
  }

  return true;
}

RD_SpriteSheetCore::SpriteCol_t::const_iterator RD_SpriteSheetCore::begin() const
{
  return _sprites.begin();
}

RD_SpriteSheetCore::SpriteCol_t::const_iterator RD_SpriteSheetCore::end() const
{
  return _sprites.end();
}

void RD_SpriteSheetCore::storeHeader( IO_PersistentStore &store ) const
{
  store << RD_SpriteSheetTag[0];
  store << RD_SpriteSheetTag[1];
  store << RD_SpriteSheetTag[2];
  store << RD_SpriteSheetTag[3];

  store << RD_SpriteSheetMajorVersion;
  store << RD_SpriteSheetMinorVersion;
}

bool RD_SpriteSheetCore::loadHeader( IO_PersistentStore &store ) const
{
  char mainTag[5];
  store >> mainTag[0];
  store >> mainTag[1];
  store >> mainTag[2];
  store >> mainTag[3];
  mainTag[4] = '\0';

  if ( strcmp( mainTag, RD_SpriteSheetTag ) != 0 )
    return false;

  int majorVersion, minorVersion;
  store >> majorVersion;
  store >> minorVersion;

  //  Do not attempt to read file if major version number
  //  is different.
  if ( majorVersion != RD_SpriteSheetMajorVersion )
    return false;

  return true;
}

void RD_SpriteSheetCore::addSprite( const STD_String &name, const Rect &rect, float offsetX, float offsetY, float scaleX, float scaleY , float angleDegrees)
{
  SpriteData &data = _sprites[ name ];

  data._rect = rect;

  data._matrix.clear();

  data._matrix.rotateDegrees(-1*angleDegrees);
  data._matrix.scale( scaleX, scaleY );
  data._matrix.translate( offsetX, offsetY );
}


void RD_SpriteSheetCore::addSprite( const STD_String &name, const Rect &rect, const Math::Matrix4x4 &matrix )
{
  SpriteData &data = _sprites[ name ];
  data._rect = rect;
  data._matrix = matrix;
}

