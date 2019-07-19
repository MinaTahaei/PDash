#ifndef _XML_SKELETON_SAX_PARSER_H_
#define _XML_SKELETON_SAX_PARSER_H_

#include "XML_SaxParser.h"

#include "TR_Types.h"

#include "UT_ShareBase.h"
#include "UT_SharedPtr.h"
#include "MEM_Override.h"
#include "STD_Containers.h"
#include "STD_Types.h"

#include <string>

class TR_NodeTree;

/*!
 * @class XML_SkeletonSaxParser
 * XML Parser for skeleton structure.
 */
class XML_SkeletonSaxParser : public XML_SaxParserComponent
{
  MEM_OVERRIDE

public:

  enum State
  {
    eRoot,
    eSkeletons,
    eSkeleton,
    eNodes,
    eLinks
  };

  class SimpleNodeTree;

  typedef UT_SharedPtr<SimpleNodeTree> SimpleNodeTreePtr_t;

  class SimpleNodeTree : public UT_ShareBase
  {
    MEM_OVERRIDE

  public:

    enum Type
    {
      eNone,
      eRead,
      ePeg,
      eCutter,
      eInverseCutter,
      eMatte,
      eDeformation,
      eDeformationRoot
    };

  public:
    SimpleNodeTree(const STD_String &name, const STD_String &id, Type type = ePeg);
    SimpleNodeTree(const STD_String &name, Type type = ePeg);

    SimpleNodeTreePtr_t find(const STD_String &id) const;

    SimpleNodeTreePtr_t append(const STD_String &name, const STD_String &id, Type type = ePeg);
    SimpleNodeTreePtr_t append(const STD_String &name, Type type = ePeg);
    SimpleNodeTreePtr_t append(const SimpleNodeTreePtr_t &pNodeTree);

    SimpleNodeTreePtr_t remove(const STD_String &id);

  public:
    STD_String  _name;
    STD_String  _id;
    Type        _type;

    typedef STD_Vector<SimpleNodeTreePtr_t > ChildCol_t;
    ChildCol_t  _childs;
  };


public:
  XML_SkeletonSaxParser( const STD_String &skeletonName, TR_NodeTree *nodeTree );
  virtual ~XML_SkeletonSaxParser();

  virtual XML_SaxParserComponentPtr_t startComponent(const char *nodeName, const XML_AttributeCol_t &attributes);
  virtual void endComponent(const char *nodeName);

private:

  void addToBuilder(const SimpleNodeTreePtr_t &buildingTree, TR_Types::DataRef_t parentNodeRef );
  void dumpTree(const SimpleNodeTreePtr_t &buildingTree, const STD_String &prefix="" ) const;

private:

  STD_String          _skeletonName;
  TR_NodeTree         *_nodeTree;

  SimpleNodeTreePtr_t _buildingTree;

  State               _state;

};

#endif /* _XML_SKELETON_SAX_PARSER_H_ */
