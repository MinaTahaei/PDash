#ifndef _XML_SAX_PARSER_DELEGATOR_H_
#define _XML_SAX_PARSER_DELEGATOR_H_

#include "UT_ShareBase.h"
#include "UT_SharedPtr.h"
#include "MEM_Override.h"
#include "STD_Containers.h"
#include "STD_Types.h"

#include <utility>
#include <string>

class XML_SaxParserComponent;

typedef STD_Pair< const char *, const char * > XML_Attribute_t;
typedef STD_Vector<XML_Attribute_t > XML_AttributeCol_t;

typedef UT_SharedPtr<XML_SaxParserComponent> XML_SaxParserComponentPtr_t;
typedef STD_Vector<XML_SaxParserComponentPtr_t > XML_SaxParserComponentStack_t;

class XML_SaxParserDelegator;

/*!
 *  @class XML_SaxParser
 *  XML Sax Parser.
 */
class XML_SaxParser
{
  MEM_OVERRIDE

public:

  XML_SaxParser();
  ~XML_SaxParser();

  bool parse(const char *xmlData, unsigned int dataLength);
  bool parse(const char *xmlFile);

  void setDelegator(XML_SaxParserDelegator* pDelegator);

  static void startElement(void *ctx, const unsigned char *name, const unsigned char **atts);
  static void endElement(void *ctx, const unsigned char *name);

  static void textHandler(void *ctx, const unsigned char *name, int len);

private:

  XML_SaxParserDelegator *_delegator;
};


/*!
 *  @class XML_SaxParserDelegator
 *  Communication interface with Sax Xml Parser.
 */
class XML_SaxParserDelegator
{
  MEM_OVERRIDE

public:

  XML_SaxParserDelegator(XML_SaxParserComponentPtr_t mainComponent);
  virtual ~XML_SaxParserDelegator();

  virtual void startElement(void *ctx, const char *name, const char **atts);
  virtual void endElement(void *ctx, const char *name);
  virtual void textHandler(void *ctx, const char *s, int len);

private:

  XML_SaxParserComponentStack_t _components;
  XML_AttributeCol_t            _attributes;
};

/*!
 *  @class XML_SaxParserComponent
 *  Base xml parsing component.
 */
class XML_SaxParserComponent : public UT_ShareBase
{
  MEM_OVERRIDE

public:

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes) =0;
  virtual void endComponent(const char *nodeName) =0;

protected:

  bool findAttribute( const XML_AttributeCol_t &attributes, const STD_String &name, STD_String &value );
  const char *findAttribute( const XML_AttributeCol_t &attributes, const char *name );

};

#endif /* _XML_SAX_PARSER_DELEGATOR_H_ */
