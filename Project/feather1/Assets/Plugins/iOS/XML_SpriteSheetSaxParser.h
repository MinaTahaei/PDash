#ifndef _XML_SPRITE_SHEET_SAX_PARSER_H_
#define _XML_SPRITE_SHEET_SAX_PARSER_H_

#include "XML_SaxParser.h"

#include "MEM_Override.h"
#include "STD_Containers.h"
#include "STD_Types.h"

#include <string>

class RD_SpriteSheetCore;

/*!
 * @class XML_SpriteSheetSaxParser
 * XML Parser for sprite sheet structure.
 */
class XML_SpriteSheetSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:

  enum State
  {
    eRoot,
    eSpriteSheets,
    eSpriteSheet
  };

public:
  XML_SpriteSheetSaxParser( const STD_String &projectFolder, const STD_String &sheetName, const STD_String &sheetResolution, RD_SpriteSheetCore *sheet );
  virtual ~XML_SpriteSheetSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  RD_SpriteSheetCore     *_sheet;
  State                   _state;
};

/*!
 *  @class XML_StageNamesSaxParser
 *  XML Parser for stage name collection.
 */
class XML_SpriteSheetNamesSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:

  enum State
  {
    eRoot,
    eSpriteSheets,
    eSpriteSheet
  };

  typedef STD_Pair< STD_String, STD_String > StringPair_t;
  typedef STD_Vector< StringPair_t > StringPairCol_t;

public:
  XML_SpriteSheetNamesSaxParser( StringPairCol_t &namePairs );
  XML_SpriteSheetNamesSaxParser( const STD_String &sheetName, StringPairCol_t &namePairs );
  virtual ~XML_SpriteSheetNamesSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  STD_String       _sheetName;
  State            _state;

  StringPairCol_t &_namePairs;
};

#endif /* _XML_SPRITE_SHEET_SAX_PARSER_H_ */
