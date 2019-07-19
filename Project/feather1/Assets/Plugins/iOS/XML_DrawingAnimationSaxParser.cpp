#include "XML_DrawingAnimationSaxParser.h"

#include "TR_NodeTree.h"
#include "TR_NodeTreeBuilder.h"

#include "STD_Types.h"

#include <string.h>

//  Common tags
#define kNameTag                   "name"
#define kFilenameTag               "filename"
#define kRepeatTag                 "repeat"
#define kTimeTag                   "time"

//  Drawing animation group main tags
#define kDrawingAnimationColTag    "drawingAnimations"
#define kDrawingAnimationTag       "drawingAnimation"
#define kDrawingColTag             "drawing"
#define kDrawingTag                "drw"

//  Drawing animation group variable tags
#define kFrameTag                  "frame"
#define kDrawingOffsetXCoordTag    "offsetX"
#define kDrawingOffsetYCoordTag    "offsetY"

#define kSpriteSheetTag            "spritesheet"
#define kNodeTag                   "node"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_DrawingAnimationSaxParser
#endif

XML_DrawingAnimationSaxParser::XML_DrawingAnimationSaxParser( const STD_String &drawingAnimationName, TR_NodeTree *nodeTree ) :
  _drawingAnimationName(drawingAnimationName),
  _nodeTree(nodeTree),
  _state(eRoot)
{
}

XML_DrawingAnimationSaxParser::~XML_DrawingAnimationSaxParser()
{
}

XML_SaxParserComponentPtr_t XML_DrawingAnimationSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  if ( _state == eRoot )
  {
    if ( strcmp( nodeName, kDrawingAnimationColTag ) == 0 )
    {
      _state = eDrawingAnimations;
    }
  }
  else if ( _state == eDrawingAnimations )
  {
    _currentSpriteSheetName.clear();

    if ( strcmp( nodeName, kDrawingAnimationTag ) == 0 )
    {
      //  Only enter drawingAnimation mode if we encounter the xml tag we search for.
      STD_String value;
      if (findAttribute( attributes, kNameTag, value ) && (value.compare(_drawingAnimationName) == 0))
      {
        if (findAttribute( attributes, kSpriteSheetTag, _currentSpriteSheetName ))
        {
          _state = eDrawingAnimation;
        }
      }
    }
  }
  else if ( _state == eDrawingAnimation )
  {
    if ( strcmp( nodeName, kDrawingColTag ) == 0 )
    {
      STD_String nodeName;
      if (findAttribute( attributes, kNodeTag, nodeName ))
      {
        return new XML_DrawingSaxParser(_currentSpriteSheetName, nodeName, _nodeTree);
      }
    }
  }

  return this;
}

void XML_DrawingAnimationSaxParser::endComponent(const char *nodeName)
{
  if ( strcmp( nodeName, kDrawingAnimationTag ) == 0 )
  {
    _state = eDrawingAnimations;
  }
  else if ( strcmp( nodeName, kDrawingAnimationColTag ) == 0 )
  {
    _state = eRoot;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_DrawingAnimationSaxParser
#endif

XML_DrawingSaxParser::XML_DrawingSaxParser( const STD_String &spriteSheetName, const STD_String &nodeName, TR_NodeTree *nodeTree ) :
  _spriteSheetName(spriteSheetName),
  _nodeTree(nodeTree)
{
  _builder = new TR_NodeTreeBuilder(_nodeTree);

  _nodeTree->nodeDataRefs( nodeName, _dataRefs );

  _it = _dataRefs.begin();
  _itEnd = _dataRefs.end();

  //  Start drawing animation sequence on first valid node.
  while ( _it != _itEnd )
  {
    TR_Types::DataRef_t dataRef = *_it;
    if ( _builder->beginDrawingSequence( dataRef ) )
      break;

    ++_it;
  }
}

XML_DrawingSaxParser::~XML_DrawingSaxParser()
{
  if ( _it != _itEnd )
  {
    //  Stop drawing animation sequence.
    _builder->endDrawingSequence();

    //  Iterate on other nodes and link drawing animation if available.
    TR_Types::DataRef_t srcDataRef = *_it;
    ++_it;

    while ( _it != _itEnd )
    {
      TR_Types::DataRef_t dstDataRef = *_it;
      _builder->linkNodeDrawingAnimation( srcDataRef, dstDataRef );

      ++_it;
    }
  }

  delete _builder;
}

XML_SaxParserComponentPtr_t XML_DrawingSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  //  No valid drawing animation, bail out.
  if ( _it == _itEnd )
    return this;

  if ( strcmp( nodeName, kDrawingTag ) == 0 )
  {
    STD_String drawingName;
    if ( findAttribute( attributes, kNameTag, drawingName ) )
    {
      float frame = 1.0f;
      float repeat = 1.0f;

      STD_String value;
      for ( XML_AttributeCol_t::const_iterator i = attributes.begin(), iEnd = attributes.end() ; i!=iEnd ; ++i )
      {
        if ( strcmp( i->first, kFrameTag ) == 0 )
          frame = (float)atof(i->second);
        else if ( strcmp( i->first, kRepeatTag ) == 0 )
          repeat = (float)atof(i->second);
      }

      _builder->addDrawing( _spriteSheetName, drawingName, frame, repeat );
    }
  }

  return this;
}

void XML_DrawingSaxParser::endComponent(const char * /*nodeName*/)
{
}
