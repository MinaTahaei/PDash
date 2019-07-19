#include "XML_SpriteSheetSaxParser.h"

#include "RD_SpriteSheetCore.h"

#include "STD_Types.h"

#include <string.h>

//  Sprite sheet group main tags
#define kSpriteSheetColTag         "spritesheets"
#define kSpriteSheetTag            "spritesheet"
#define kSpriteTag                 "sprite"

#define kSpriteSheetResolutionTag  "resolution"
#define kSpriteSheetNameTag        "name"
#define kSpriteSheetFilenameTag    "filename"

#define kSpriteRectTag             "rect"
#define kSpriteNameTag             "name"
#define kSpriteOffsetXTag          "offsetX"
#define kSpriteOffsetYTag          "offsetY"
#define kSpriteScaleXTag           "scaleX"
#define kSpriteScaleYTag           "scaleY"
#define kSpriteAngleDegreesTag     "angleDegrees"

#define kNameTag                   "name"

namespace
{
  bool extract4Tuple( const STD_String &str, int &val1, int &val2, int &val3, int &val4 )
  {
    const char sep = ',';

    size_t idx1 = str.find_first_of( sep );

    if (idx1 == STD_String::npos)
      return false;

    size_t idx2 = str.find_first_of( sep, idx1+1 );

    if (idx2 == STD_String::npos)
      return false;

    size_t idx3 = str.find_first_of( sep, idx2+1 );

    if (idx3 == STD_String::npos)
      return false;

    size_t errIdx = str.find_first_of( sep, idx3+1 );

    if (errIdx != STD_String::npos)
      return false;

    STD_String strVal1 = str.substr( 0, idx1 );
    STD_String strVal2 = str.substr( idx1+1, idx2-idx1-1 );
    STD_String strVal3 = str.substr( idx2+1, idx3-idx2-1 );
    STD_String strVal4 = str.substr( idx3+1 );

    val1 = atoi(strVal1.c_str());
    val2 = atoi(strVal2.c_str());
    val3 = atoi(strVal3.c_str());
    val4 = atoi(strVal4.c_str());

    return true;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_SpriteSheetSaxParser
#endif

XML_SpriteSheetSaxParser::XML_SpriteSheetSaxParser( const STD_String &projectFolder, const STD_String &sheetName, const STD_String &sheetResolution, RD_SpriteSheetCore *sheet ) :
  _sheet(sheet),
  _state(eRoot)
{
  _sheet->_projectFolder = projectFolder;
  _sheet->_sheetName = sheetName;
  _sheet->_sheetResolution = sheetResolution;
}

XML_SpriteSheetSaxParser::~XML_SpriteSheetSaxParser()
{
}

XML_SaxParserComponentPtr_t XML_SpriteSheetSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  if ( _state == eRoot )
  {
    if ( strcmp( nodeName, kSpriteSheetColTag ) == 0 )
    {
      _state = eSpriteSheets;
    }
  }
  else if ( _state == eSpriteSheets )
  {
    if ( strcmp( nodeName, kSpriteSheetTag ) == 0 )
    {
      //  Only enter stage mode if we encounter the xml tag we search for.
      STD_String value;
      if (findAttribute( attributes, kSpriteSheetNameTag, value ) && (value.compare(_sheet->_sheetName) == 0) &&
          findAttribute( attributes, kSpriteSheetResolutionTag, value ) && (value.compare(_sheet->_sheetResolution) == 0) )
      {
        _state = eSpriteSheet;

        //  Retrieve sprite sheet filename.
        if ( findAttribute( attributes, kSpriteSheetFilenameTag, value ) )
        {
          _sheet->_sheetFilename = value;
        }
      }
    }
  }
  else if ( _state == eSpriteSheet )
  {
    if ( strcmp( nodeName, kSpriteTag ) == 0 )
    {
      RD_SpriteSheetCore::Rect rect;

      STD_String nameValue;
      STD_String rectValue;

      float scaleX = 1.0f, scaleY = 1.0f;
      float offsetX = 0.0f, offsetY = 0.0f;
	  float angleDegrees = 0.0f;

      for ( XML_AttributeCol_t::const_iterator i = attributes.begin(), iEnd = attributes.end() ; i!=iEnd ; ++i )
      {
        if ( strcmp( i->first, kSpriteNameTag ) == 0 )
          nameValue = i->second;
        else if ( strcmp( i->first, kSpriteRectTag ) == 0 )
          rectValue = i->second;
        else if ( strcmp( i->first, kSpriteOffsetXTag ) == 0 )
          offsetX = (float)atof(i->second);
        else if ( strcmp( i->first, kSpriteOffsetYTag ) == 0 )
          offsetY = (float)atof(i->second);
        else if ( strcmp( i->first, kSpriteScaleXTag ) == 0 )
          scaleX = (float)atof(i->second);
        else if ( strcmp( i->first, kSpriteScaleYTag ) == 0 )
          scaleY = (float)atof(i->second);
		else if ( strcmp( i->first, kSpriteAngleDegreesTag ) == 0 ){
          angleDegrees = (float)atof(i->second);
		}
      }

      if ( !nameValue.empty() && extract4Tuple( rectValue, rect._x, rect._y, rect._w, rect._h ) )
      {
        _sheet->addSprite( nameValue, rect, offsetX, offsetY, scaleX, scaleY ,angleDegrees);
      }
    }
  }

  return this;
}

void XML_SpriteSheetSaxParser::endComponent(const char *nodeName)
{
  if ( _state == eSpriteSheet )
  {
    if ( strcmp( nodeName, kSpriteSheetTag ) == 0)
    {
      _state = eSpriteSheets;
    }
  }
  else if ( strcmp( nodeName, kSpriteSheetColTag ) == 0 )
  {
    _state = eRoot;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_SpriteSheetNamesSaxParser
#endif

XML_SpriteSheetNamesSaxParser::XML_SpriteSheetNamesSaxParser( StringPairCol_t &namePairs ) :
  _state(eRoot),
  _namePairs(namePairs)
{
}

XML_SpriteSheetNamesSaxParser::XML_SpriteSheetNamesSaxParser( const STD_String &sheetName, StringPairCol_t &namePairs ) :
  _sheetName(sheetName),
  _state(eRoot),
  _namePairs(namePairs)
{
}

XML_SpriteSheetNamesSaxParser::~XML_SpriteSheetNamesSaxParser()
{
}

XML_SaxParserComponentPtr_t XML_SpriteSheetNamesSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  if ( _state == eRoot )
  {
    if ( strcmp( nodeName, kSpriteSheetColTag ) == 0 )
    {
      _state = eSpriteSheets;
    }
  }
  else if ( _state == eSpriteSheets )
  {
    if ( strcmp( nodeName, kSpriteSheetTag ) == 0 )
    {
      //  Only enter stage mode if we encounter the xml tag we search for.
      STD_String sheetName, resolutionName;
      if (findAttribute( attributes, kSpriteSheetNameTag, sheetName ) && (_sheetName.empty() || (sheetName.compare(_sheetName) == 0)) &&
          findAttribute( attributes, kSpriteSheetResolutionTag, resolutionName ))
      {
        _namePairs.push_back( STD_MakePair(sheetName, resolutionName) );
      }

      _state = eSpriteSheet;
    }
  }

  return this;
}

void XML_SpriteSheetNamesSaxParser::endComponent(const char *nodeName)
{
  if ( _state == eSpriteSheet )
  {
    if ( strcmp( nodeName, kSpriteSheetTag ) == 0)
    {
      _state = eSpriteSheets;
    }
  }
  else if ( strcmp( nodeName, kSpriteSheetColTag ) == 0 )
  {
    _state = eRoot;
  }
}
