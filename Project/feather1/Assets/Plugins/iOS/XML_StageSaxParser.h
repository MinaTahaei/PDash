#ifndef _XML_STAGE_SAX_PARSER_H_
#define _XML_STAGE_SAX_PARSER_H_

#include "XML_SaxParser.h"

#include "MEM_Override.h"
#include "STD_Containers.h"
#include "STD_Types.h"

class RD_ClipDataCore;

/*!
 *  @class XML_StageSaxParser
 *  XML Parser for stage clip.
 */
class XML_StageSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:

  enum State
  {
    eRoot,
    eStages,
    eStage
  };

public:
  XML_StageSaxParser( const STD_String &projectFolder, const STD_String &clipName, RD_ClipDataCore *clip );
  virtual ~XML_StageSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  STD_String           _projectFolder;
  STD_String           _clipName;
  State                _state;

  RD_ClipDataCore *_clip;
};

/*!
 *  @class XML_StageNamesSaxParser
 *  XML Parser for stage name collection.
 */
class XML_StageNamesSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:

  enum State
  {
    eRoot,
    eStages,
    eStage
  };

  typedef STD_Vector< STD_String > StringCol_t;

public:
  XML_StageNamesSaxParser( const STD_String &projectFolder, StringCol_t &clipNames );
  virtual ~XML_StageNamesSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  STD_String   _projectFolder;
  State        _state;

  StringCol_t &_clipNames;
};

#endif /* _XML_STAGE_SAX_PARSER_H_ */
