#ifndef _XML_ANIMATION_SAX_PARSER_H_
#define _XML_ANIMATION_SAX_PARSER_H_

#include "XML_SaxParser.h"

#include "TR_Types.h"

#include "MEM_Override.h"
#include "STD_Containers.h"
#include "STD_Types.h"

#include <string>

class TR_NodeTree;
class TR_NodeTreeBuilder;

/*!
 * @class XML_AnimationSaxParser
 * XML Parser for animation structure.
 */
class XML_AnimationSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:

  enum State
  {
    eRoot,
    eAnimations,
    eAnimation,
    eAttrLinks,
    eTimedValues
  };

  typedef STD_Pair< TR_Types::DataRef_t, TR_Types::CurveChannel_t > AttrLinkData;
  typedef STD_Set< AttrLinkData > AttrLinkCol_t;
  typedef STD_Map< STD_String, AttrLinkCol_t > TvLinkCol_t;

  typedef STD_Set< STD_String > StringCol_t;

public:
  XML_AnimationSaxParser( const STD_String &animationName, TR_NodeTree *nodeTree );
  virtual ~XML_AnimationSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  STD_String          _animationName;
  TR_NodeTree        *_nodeTree;

  TvLinkCol_t         _tvLinks;
  StringCol_t         _linkedNodes;

  State               _state;
};

/*!
 * @class XML_BezierSaxParser
 * XML Parser for bezier curve structure.
 */
class XML_BezierSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:
  typedef  XML_AnimationSaxParser::AttrLinkCol_t AttrLinkCol_t;
  typedef  STD_Set< TR_Types::DataRef_t > DataRefCol_t;

public:
  XML_BezierSaxParser( const AttrLinkCol_t &attrLinks, TR_NodeTree *nodeTree );
  virtual ~XML_BezierSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  TR_NodeTreeBuilder          *_builder;
  TR_Types::DataRef_t          _channelDataRef;
  DataRefCol_t                 _duplicateChannels;
};

/*!
 * @class XML_CatmullSaxParser
 * XML Parser for catmull curve structure.
 */
class XML_CatmullSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:
  typedef  XML_AnimationSaxParser::AttrLinkCol_t AttrLinkCol_t;
  typedef  STD_Set< TR_Types::DataRef_t > DataRefCol_t;

public:
  XML_CatmullSaxParser( const AttrLinkCol_t &attrLinks, TR_NodeTree *nodeTree, float scaleX, float scaleY, float scaleZ );
  virtual ~XML_CatmullSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  TR_NodeTreeBuilder          *_builder;
  TR_Types::DataRef_t          _channelDataRef;
  DataRefCol_t                 _duplicateChannels;
};

/*!
 * @class XML_LinearSaxParser
 * XML Parser for linear curve structure.
 */
class XML_LinearSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:
  typedef  XML_AnimationSaxParser::AttrLinkCol_t AttrLinkCol_t;
  typedef  STD_Set< TR_Types::DataRef_t > DataRefCol_t;

public:
  XML_LinearSaxParser( const AttrLinkCol_t &attrLinks, TR_NodeTree *nodeTree );
  virtual ~XML_LinearSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  TR_NodeTreeBuilder          *_builder;
  TR_Types::DataRef_t          _channelDataRef;
  DataRefCol_t                 _duplicateChannels;
};

/*!
 * @class XML_PivotSaxParser
 * XML Parser for animated pivot structure.
 */
class XML_PivotSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:
  typedef  XML_AnimationSaxParser::AttrLinkCol_t AttrLinkCol_t;
  typedef  STD_Set< TR_Types::DataRef_t > DataRefCol_t;

public:
  XML_PivotSaxParser( const AttrLinkCol_t &attrLinks, TR_NodeTree *nodeTree );
  virtual ~XML_PivotSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  TR_NodeTreeBuilder          *_builder;
  TR_Types::DataRef_t          _channelDataRef;
  DataRefCol_t                 _duplicateChannels;
};


#endif /* _XML_ANIMATION_SAX_PARSER_H_ */
