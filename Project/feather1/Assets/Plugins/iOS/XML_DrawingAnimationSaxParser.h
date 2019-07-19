#ifndef _XML_DRAWING_ANIMATION_SAX_PARSER_H_
#define _XML_DRAWING_ANIMATION_SAX_PARSER_H_

#include "XML_SaxParser.h"

#include "TR_Types.h"

#include "MEM_Override.h"
#include "STD_Types.h"

#include <string>
#include <set>

class TR_NodeTree;
class TR_NodeTreeBuilder;

/*!
 * @class XML_DrawingAnimationSaxParser
 * XML Parser for drawing animation structure.
 */
class XML_DrawingAnimationSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:

  enum State
  {
    eRoot,
    eDrawingAnimations,
    eDrawingAnimation,
  };

  typedef STD_Set< STD_String > StringCol_t;


public:
  XML_DrawingAnimationSaxParser( const STD_String &drawingAnimationName, TR_NodeTree *nodeTree );
  virtual ~XML_DrawingAnimationSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  STD_String          _drawingAnimationName;
  STD_String          _currentSpriteSheetName;
  TR_NodeTree        *_nodeTree;

  State               _state;

};

/*!
 * @class XML_DrawingSaxParser
 * XML Parser for drawing structure.
 */
class XML_DrawingSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:
  typedef STD_Set<TR_Types::DataRef_t> DataRefCol_t;

public:
  XML_DrawingSaxParser( const STD_String &spriteSheetName, const STD_String &nodeName, TR_NodeTree *nodeTree );
  virtual ~XML_DrawingSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  STD_String          _spriteSheetName;
  STD_String          _nodeName;

  DataRefCol_t                 _dataRefs;
  DataRefCol_t::const_iterator _it;
  DataRefCol_t::const_iterator _itEnd;

  TR_NodeTree        *_nodeTree;
  TR_NodeTreeBuilder *_builder;

};

#endif /* _XML_DRAWING_ANIMATION_SAX_PARSER_H_ */
