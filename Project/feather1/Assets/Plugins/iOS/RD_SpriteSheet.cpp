#include "RD_SpriteSheet.h"
#include "RD_SpriteSheet.h"

#include "IM_ImageIO.h"

#include "XML_ProjectLoader.h"
#include "BIN_ProjectLoader.h"

#include "STD_Types.h"

#if 0 
#include <fstream>
/*!
 *  Will write the bitmap passed in parameter in a ppm file
 *  named filename
 */
void WritePPMFile( const std::string &filename, const IM_ImagePtr_t &image)
{
  if ( image.get() == 0 )
    return;

  std::ofstream file(filename.c_str());

  unsigned nBytesPerLine = image->width() * 3;

  file << "P6\n# Creator stage\n";
  file << (nBytesPerLine/3) << " ";
  file << image->height();
  file << "\n255\n";

  for(unsigned int i=0 ; i<image->height() ; ++i)
  {
    for(unsigned int j=0 ; j<image->width() ; ++j)
    {
      file.write((const char *)image->pixel(j,i),   1);
      file.write((const char *)image->pixel(j,i)+1, 1);
      file.write((const char *)image->pixel(j,i)+2, 1);
    }
  }

  file.close();
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_SpriteSheet::Impl
#endif
class RD_SpriteSheet::Impl
{
  friend class RD_SpriteSheet;

public:
  Impl() :
    _width(0u),
    _height(0u)
  {
  }

  ~Impl()
  {
  }

private:

  IM_ImagePtr_t       _pSpriteSheetImage;
  unsigned            _width;
  unsigned            _height;

  STD_String          _projectFolder;
  RD_SpriteSheetCore *_pSpriteSheet;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_SpriteSheet
#endif

RD_SpriteSheetPtr_t RD_SpriteSheet::create( const RD_SpriteSheetKey &key )
{
  RD_SpriteSheetCore *spriteSheetCore = new RD_SpriteSheetCore;

  //  Try to load from binary file, fallback on xml format.
  if ( !BIN_ProjectLoader::loadSpriteSheet( key._projectFolder, key._sheetName, key._resolutionName, spriteSheetCore ) )
    XML_ProjectLoader::loadSpriteSheet( key._projectFolder, key._sheetName, key._resolutionName, spriteSheetCore );

  if ( !spriteSheetCore->empty() )
  {
    RD_SpriteSheetPtr_t pSpriteSheet = new RD_SpriteSheet(key._projectFolder, spriteSheetCore);
    pSpriteSheet->buildSpriteSheet();

    RD_SpriteSheetManager::instance()->addObject(key, pSpriteSheet);

    return pSpriteSheet;
  }

  delete spriteSheetCore;
  return RD_SpriteSheetPtr_t(0);
}

RD_SpriteSheetPtr_t RD_SpriteSheet::createOrLoad( const RD_SpriteSheetKey &key )
{
  RD_SpriteSheetPtr_t pSpriteSheet = RD_SpriteSheetManager::instance()->object( key );

  //  Resolution was changed, try loading new spritesheet.
  if (!pSpriteSheet.isValid())
  {
    pSpriteSheet = RD_SpriteSheet::create(key);
    if ( !pSpriteSheet.isValid() )
    {
      //  No sprite sheet could be found for sheetResolution.  Try to retrieve valid one.
      XML_ProjectLoader::StringPairCol_t resolutionNames;
      XML_ProjectLoader::loadSpriteSheetResolutionNames(key._projectFolder, key._sheetName, resolutionNames );

      if ( !resolutionNames.empty() )
      {
        STD_String defaultResolutionName = resolutionNames.front().second;
        RD_SpriteSheetKey defaultKey(key._projectFolder, key._sheetName, defaultResolutionName);
        pSpriteSheet = RD_SpriteSheetManager::instance()->object( defaultKey );
        if ( !pSpriteSheet.isValid() )
        {
          pSpriteSheet = RD_SpriteSheet::create(defaultKey);
        }
      }
    }
  }

  return pSpriteSheet;
}

RD_SpriteSheet::RD_SpriteSheet(const STD_String &projectFolder, RD_SpriteSheetCore *pSpriteSheet )
{
  _i = new Impl;
  _i->_projectFolder = projectFolder;
  _i->_pSpriteSheet = pSpriteSheet;  // owned by RD_SpriteSheet.
}

RD_SpriteSheet::~RD_SpriteSheet()
{
  delete _i->_pSpriteSheet;

  delete _i;
}


bool RD_SpriteSheet::buildSpriteSheet()
{
  IM_ImageIOContext context( true /* premultiplyAlpha */ );

  _i->_pSpriteSheetImage = IM_ImageIO::instance()->read( context, sheetFilename() );
  if ( _i->_pSpriteSheetImage.get() )
  {
    _i->_width  = _i->_pSpriteSheetImage->width();
    _i->_height = _i->_pSpriteSheetImage->height();
  }

  return _i->_pSpriteSheetImage.isValid();
}

void RD_SpriteSheet::freeSpriteSheetData()
{
  //  Frees image data, but not sprite sheet data structure.
  //  Can be used to load image into GL, and avoid keeping data double.
  _i->_pSpriteSheetImage = IM_ImagePtr_t(0);
}

const RD_SpriteSheet::SpriteData *RD_SpriteSheet::sprite( const STD_String &name ) const
{
  return _i->_pSpriteSheet->sprite( name );
}

bool RD_SpriteSheet::rect( const STD_String &name, Rect &rect ) const
{
  return _i->_pSpriteSheet->rect( name, rect );
}

bool RD_SpriteSheet::matrix( const STD_String &name, Math::Matrix4x4 &matrix ) const
{
  return _i->_pSpriteSheet->matrix( name, matrix );
}

const STD_String &RD_SpriteSheet::projectFolder() const
{
  return _i->_projectFolder;
}

STD_String RD_SpriteSheet::sheetFilename() const
{
  return _i->_projectFolder + "/" + _i->_pSpriteSheet->sheetFilename();
}

const STD_String &RD_SpriteSheet::sheetName() const
{
  return _i->_pSpriteSheet->sheetName();
}

const STD_String &RD_SpriteSheet::sheetResolution() const
{
  return _i->_pSpriteSheet->sheetResolution();
}

bool RD_SpriteSheet::uvs( const STD_String &name, float &u1, float &v1, float &u2, float &v2 ) const
{
  Rect r;
  if ( rect(name, r) )
  {
    return uvs( r, u1, v1, u2, v2 );
  }

  return false;
}


bool RD_SpriteSheet::uvs( const STD_String &name, float uvs[4] ) const
{
  Rect r;
  if ( rect(name, r) )
  {
    return this->uvs( r, uvs[0], uvs[1], uvs[2], uvs[3] );
  }

  return false;
}

bool RD_SpriteSheet::uvs( const Rect &rect, float &u1, float &v1, float &u2, float &v2 ) const
{
  if ( (_i->_width > 0u) && (_i->_height > 0u) )
  {
    float fwidth = (float)_i->_width;
    float fheight = (float)_i->_height;

    if ( fwidth > 0.0f )
    {
      u1 = rect._x / fwidth;
      u2 = (rect._x+rect._w) / fwidth;
    }
    else
    {
      u1 = u2 = 0.0f;
    }

    if ( fheight > 0.0f )
    {
      v1 = rect._y / fheight;
      v2 = (rect._y+rect._h) / fheight;
    }
    else
    {
      v1 = v2 = 0.0f;
    }

    return true;
  }

  return false;
}

bool RD_SpriteSheet::uvs( const Rect &rect, float uvs[4] ) const
{
  return this->uvs( rect, uvs[0], uvs[1], uvs[2], uvs[3] );
}

bool RD_SpriteSheet::hasImage() const
{
  return ( (_i->_pSpriteSheetImage.get() != 0) && (_i->_pSpriteSheetImage->width() > 0) && ( _i->_pSpriteSheetImage->height() > 0) );
}

const IM_ImagePtr_t &RD_SpriteSheet::image() const
{
  return _i->_pSpriteSheetImage;
}

unsigned RD_SpriteSheet::width() const
{
  return _i->_width;
}

unsigned RD_SpriteSheet::height() const
{
  return _i->_height;
}

