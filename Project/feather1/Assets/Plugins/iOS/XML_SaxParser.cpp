#include "XML_SaxParser.h"
#include "STD_Types.h"

#include "tinyxml2.h"

#include <string.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_SaxHandler
#endif

class XML_SaxHandler : public tinyxml2::XMLVisitor
{
  MEM_OVERRIDE

public:
	XML_SaxHandler() : _parser(0) {};

	virtual bool VisitEnter( const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute );
	virtual bool VisitExit( const tinyxml2::XMLElement& element );
	virtual bool Visit( const tinyxml2::XMLText& text );
	virtual bool Visit( const tinyxml2::XMLUnknown&){ return true; }

	void setParser( XML_SaxParser* parser)
	{
		_parser = parser;
	}

private:
	XML_SaxParser *_parser;
};


bool XML_SaxHandler::VisitEnter( const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute )
{
	STD_Vector<const char* > attsVector;
	for( const tinyxml2::XMLAttribute* attrib = firstAttribute; attrib; attrib = attrib->Next() )
	{
		attsVector.push_back(attrib->Name());
		attsVector.push_back(attrib->Value());
	}

  attsVector.push_back(NULL);

	XML_SaxParser::startElement(_parser, (const unsigned char *)element.Value(), (const unsigned char **)(&attsVector[0]));
	return true;
}
bool XML_SaxHandler::VisitExit( const tinyxml2::XMLElement& element )
{
	XML_SaxParser::endElement(_parser, (const unsigned char *)element.Value());
	return true;
}

bool XML_SaxHandler::Visit( const tinyxml2::XMLText& text )
{
	XML_SaxParser::textHandler(_parser, (const unsigned char *)text.Value(), (int)strlen(text.Value()));
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_SaxParser
#endif
XML_SaxParser::XML_SaxParser() :
  _delegator(0)
{
}

XML_SaxParser::~XML_SaxParser()
{
}

bool XML_SaxParser::parse(const char* xmlData, unsigned int dataLength)
{
	tinyxml2::XMLDocument tinyDoc;
	tinyDoc.Parse(xmlData, dataLength);

  XML_SaxHandler handler;
	handler.setParser(this);

	return tinyDoc.Accept( &handler );
}

bool XML_SaxParser::parse(const char *xmlFile)
{
	tinyxml2::XMLDocument tinyDoc;
  tinyDoc.LoadFile(xmlFile);

  XML_SaxHandler handler;
	handler.setParser(this);

	return tinyDoc.Accept( &handler );
}

void XML_SaxParser::startElement(void *ctx, const unsigned char *name, const unsigned char **atts)
{
  ((XML_SaxParser*)(ctx))->_delegator->startElement(ctx, (char*)name, (const char**)atts);
}

void XML_SaxParser::endElement(void *ctx, const unsigned char *name)
{
  ((XML_SaxParser*)(ctx))->_delegator->endElement(ctx, (char*)name);
}
void XML_SaxParser::textHandler(void *ctx, const unsigned char *name, int len)
{
  ((XML_SaxParser*)(ctx))->_delegator->textHandler(ctx, (char*)name, len);
}

void XML_SaxParser::setDelegator(XML_SaxParserDelegator* delegator)
{
  _delegator = delegator;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_SaxParserDelegator
#endif

XML_SaxParserDelegator::XML_SaxParserDelegator(XML_SaxParserComponentPtr_t mainComponent)
{
  _components.push_back(mainComponent);
}

XML_SaxParserDelegator::~XML_SaxParserDelegator()
{
  _components.pop_back();
}

void XML_SaxParserDelegator::startElement(void * /*ctx*/, const char *name, const char **atts)
{
  _attributes.clear();
  for ( const char **iAtt = atts ; *iAtt != 0 ; iAtt=iAtt+2 )
  {
    XML_Attribute_t attribute;
    attribute.first = *iAtt;
    attribute.second = *(iAtt+1);

    _attributes.push_back( attribute );
  }

  XML_SaxParserComponentPtr_t pComponent = _components.back()->startComponent(name, _attributes);
  _components.push_back(pComponent);
}

void XML_SaxParserDelegator::endElement(void * /*ctx*/, const char *name)
{
  _components.back()->endComponent(name);
  _components.pop_back();
}

void XML_SaxParserDelegator::textHandler(void * /*ctx*/, const char * /*s*/, int /*len*/)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_SaxParserComponent
#endif

bool XML_SaxParserComponent::findAttribute( const XML_AttributeCol_t &attributes, const STD_String &name, STD_String &value )
{
  for ( XML_AttributeCol_t::const_iterator i = attributes.begin(), iEnd = attributes.end() ; i!=iEnd ; ++i )
  {
    if ( name.compare( i->first ) == 0 )
    {
      value = i->second;
      return true;
    }
  }

  return false;
}

const char *XML_SaxParserComponent::findAttribute( const XML_AttributeCol_t &attributes, const char *name )
{
  for ( XML_AttributeCol_t::const_iterator i = attributes.begin(), iEnd = attributes.end() ; i!=iEnd ; ++i )
  {
    if ( strcmp( i->first, name ) == 0 )
    {
      return i->second;
    }
  }

  return 0;
}
