
#include "XML_SkeletonSaxParser.h"

#include "TR_NodeTreeBuilder.h"

#include "STD_Types.h"

#include <stdio.h>
#include <string.h>

//  Common tags
#define kNameTag                 "name"
#define kFilenameTag             "filename"
#define kRepeatTag               "repeat"

//  Skeleton group main tags
#define kSkeletonColTag          "skeletons"
#define kSkeletonTag             "skeleton"
#define kNodeColTag              "nodes"
#define kNodeTag                 "node"
#define kNodeIdTag               "id"
#define kPegTag                  "peg"
#define kReadTag                 "read"
#define kLinkColTag              "links"
#define kLinkTag                 "link"

#define kCutterTag               "cutter"
#define kInverseCutterTag        "inverseCutter"
#define kMatteTag                "matte"

#define kDeformationTag          "deform"
#define kDeformationRootTag      "deformRoot"
#define kDeformationBoneTag      "bone"

//  Skeleton group variable tags
#define kNodeInTag               "in"
#define kNodeOutTag              "out"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_SkeletonSaxParser::SimpleNodeTree
#endif

XML_SkeletonSaxParser::SimpleNodeTree::SimpleNodeTree(const STD_String &name, const STD_String &id, Type type) :
  _name(name),
  _id(id),
  _type(type)
{
}


XML_SkeletonSaxParser::SimpleNodeTree::SimpleNodeTree(const STD_String &name, Type type) :
  _name(name),
  _id(name),
  _type(type)
{
}

XML_SkeletonSaxParser::SimpleNodeTreePtr_t XML_SkeletonSaxParser::SimpleNodeTree::find(const STD_String &id) const
{
  if ( _id.compare(id) == 0 )
    return const_cast<SimpleNodeTree*>(this);

  for ( ChildCol_t::const_iterator i = _childs.begin(), iEnd = _childs.end() ; i!=iEnd ; ++i )
  {
    SimpleNodeTreePtr_t pNodeTree = (*i)->find(id);
    if (pNodeTree.get())
      return pNodeTree;
  }

  return SimpleNodeTreePtr_t(0);
}

XML_SkeletonSaxParser::SimpleNodeTreePtr_t XML_SkeletonSaxParser::SimpleNodeTree::append(const STD_String &name, const STD_String &id, Type type)
{
  SimpleNodeTreePtr_t pNodeTree = new SimpleNodeTree(name, id, type);
  return append(pNodeTree);
}

XML_SkeletonSaxParser::SimpleNodeTreePtr_t XML_SkeletonSaxParser::SimpleNodeTree::append(const STD_String &name, Type type)
{
  SimpleNodeTreePtr_t pNodeTree = new SimpleNodeTree(name, type);
  return append(pNodeTree);
}

XML_SkeletonSaxParser::SimpleNodeTreePtr_t XML_SkeletonSaxParser::SimpleNodeTree::append(const SimpleNodeTreePtr_t &pNodeTree)
{
  _childs.push_back(pNodeTree);
  return pNodeTree;
}

XML_SkeletonSaxParser::SimpleNodeTreePtr_t XML_SkeletonSaxParser::SimpleNodeTree::remove(const STD_String &id)
{
  for ( ChildCol_t::iterator i = _childs.begin(), iEnd = _childs.end() ; i!=iEnd ; ++i )
  {
    SimpleNodeTreePtr_t pNodeTree = *i;

    if ( pNodeTree->_id.compare(id) == 0 )
    {
      _childs.erase(i);
      return pNodeTree;
    }
    else
    {
      SimpleNodeTreePtr_t pRemovedNodeTree = pNodeTree->remove(id);
      if (pRemovedNodeTree.get())
        return pRemovedNodeTree;
    }
  }

  return SimpleNodeTreePtr_t(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_SkeletonSaxParser
#endif

XML_SkeletonSaxParser::XML_SkeletonSaxParser( const STD_String &skeletonName, TR_NodeTree *nodeTree ) :
  _skeletonName(skeletonName),
  _nodeTree(nodeTree),
  _state(eRoot)
{
}

XML_SkeletonSaxParser::~XML_SkeletonSaxParser()
{
}

XML_SaxParserComponentPtr_t XML_SkeletonSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  if ( _state == eRoot )
  {
    if ( strcmp( nodeName, kSkeletonColTag ) == 0 )
    {
      _state = eSkeletons;
    }
  }
  else if ( _state == eSkeletons )
  {
    if ( strcmp( nodeName, kSkeletonTag ) == 0 )
    {
      //  Only enter stage mode if we encounter the xml tag we search for.
      STD_String value;
      if (findAttribute( attributes, kNameTag, value ) && (value.compare(_skeletonName) == 0))
      {
        _state = eSkeleton;

        _buildingTree = SimpleNodeTreePtr_t(0);
        _buildingTree = new SimpleNodeTree( "" );
      }
    }
  }
  else if ( _state == eSkeleton )
  {
    if ( strcmp( nodeName, kNodeColTag ) == 0 )
    {
      _state = eNodes;
    }
    else if ( strcmp( nodeName, kLinkColTag ) == 0 )
    {
      _state = eLinks;
    }
  }
  else if ( _state == eNodes )
  {
    SimpleNodeTree::Type type = SimpleNodeTree::eNone;
    if ( strcmp( nodeName, kPegTag ) == 0 )
      type = SimpleNodeTree::ePeg;
    else if ( strcmp( nodeName, kReadTag ) == 0 )
      type = SimpleNodeTree::eRead;
    else if ( strcmp( nodeName, kCutterTag ) == 0 )
      type = SimpleNodeTree::eCutter;
    else if ( strcmp( nodeName, kInverseCutterTag ) == 0 )
      type = SimpleNodeTree::eInverseCutter;
    else if ( strcmp( nodeName, kMatteTag ) == 0 )
      type = SimpleNodeTree::eMatte;
    else if ( strcmp( nodeName, kDeformationTag ) == 0 )
      type = SimpleNodeTree::eDeformation;
    else if ( strcmp( nodeName, kDeformationRootTag ) == 0 )
      type = SimpleNodeTree::eDeformationRoot;
    else if ( strcmp( nodeName, kDeformationBoneTag ) == 0 )
      type = SimpleNodeTree::ePeg;

    if ( type != SimpleNodeTree::eNone )
    {
      STD_String name;
      if ( findAttribute( attributes, kNameTag, name ) )
      {
        STD_String id;
        if ( !findAttribute( attributes, kNodeIdTag, id ) )
          id = name;

        SimpleNodeTreePtr_t pNodeTree = _buildingTree->find(id);
        if ( !pNodeTree.get() )
        {
          _buildingTree->append(name, id, type);
        }
      }
    }
  }
  else if ( _state == eLinks )
  {
    if ( (strcmp( nodeName, kLinkTag ) == 0) )
    {
      STD_String in, out;
      if ( findAttribute( attributes, kNodeInTag, in ) &&
           findAttribute( attributes, kNodeOutTag, out ) )
      {
        SimpleNodeTreePtr_t pInNodeTree = _buildingTree->find(in);
        if ( !pInNodeTree.get() )
          pInNodeTree = _buildingTree->append(in);

        SimpleNodeTreePtr_t pOutNodeTree = _buildingTree->remove(out);
        if ( !pOutNodeTree.get() )
          pOutNodeTree = new SimpleNodeTree(out);

        pInNodeTree->append( pOutNodeTree );
      }
    }
  }

  return this;
}

void XML_SkeletonSaxParser::endComponent(const char *nodeName)
{
  if ( _state == eNodes || _state == eLinks )
  {
    if ( (strcmp( nodeName, kNodeColTag ) == 0) ||
         (strcmp( nodeName, kLinkColTag ) == 0) )
    {
      _state = eSkeleton;
    }
  }
  else if ( strcmp( nodeName, kSkeletonTag ) == 0 )
  {
    if ( _buildingTree.get() )
    {
      //dumpTree(_buildingTree);
      addToBuilder(_buildingTree, _nodeTree->firstNodeDataRef());

      //  Reset building tree.
      _buildingTree = SimpleNodeTreePtr_t(0);
    }

    _state = eSkeletons;
  }
  else if ( strcmp( nodeName, kSkeletonColTag ) == 0 )
  {
    _state = eRoot;
  }
}

void XML_SkeletonSaxParser::addToBuilder(const SimpleNodeTreePtr_t &buildingTree, TR_Types::DataRef_t parentNodeRef )
{
  TR_NodeTreeBuilder builder(_nodeTree);

  //  Add in reverse order that is natural order of composition.
  for ( SimpleNodeTree::ChildCol_t::const_iterator i = buildingTree->_childs.begin(),
        iEnd = buildingTree->_childs.end() ; i!=iEnd ; ++i )
  {
    SimpleNodeTreePtr_t pNodeTree = *i;

    if ( pNodeTree->_type == SimpleNodeTree::eRead )
    {
      TR_Types::DataRef_t nodeRef = builder.addNode( pNodeTree->_name, parentNodeRef );

      addToBuilder( pNodeTree, nodeRef );
    }
    else if ( pNodeTree->_type == SimpleNodeTree::ePeg )
    {
      TR_Types::DataRef_t nodeRef = builder.addNode( pNodeTree->_name, parentNodeRef );

      //  Create an empty drawing sequence so that no drawing sequence is assigned to this node.
      builder.beginDrawingSequence( nodeRef );
      builder.endDrawingSequence();

      addToBuilder( pNodeTree, nodeRef );
    }
    else if ( pNodeTree->_type == SimpleNodeTree::eCutter )
    {
      TR_Types::DataRef_t nodeRef = builder.addNode( pNodeTree->_name, parentNodeRef );
      if ( builder.createEffect( nodeRef, TR_Types::eCutter ) )
      {
        addToBuilder( pNodeTree, nodeRef );
      }
    }
    else if ( pNodeTree->_type == SimpleNodeTree::eInverseCutter )
    {
      TR_Types::DataRef_t nodeRef = builder.addNode( pNodeTree->_name, parentNodeRef );
      if ( builder.createEffect( nodeRef, TR_Types::eInverseCutter ) )
      {
        addToBuilder( pNodeTree, nodeRef );
      }
    }
    else if ( pNodeTree->_type == SimpleNodeTree::eMatte )
    {
      TR_Types::DataRef_t nodeRef = builder.addMatteNode( pNodeTree->_name, parentNodeRef );
      addToBuilder( pNodeTree, nodeRef );
    }
    else if ( pNodeTree->_type == SimpleNodeTree::eDeformation )
    {
      TR_Types::DataRef_t nodeRef = builder.addNode( pNodeTree->_name, parentNodeRef );
      if ( builder.createEffect( nodeRef, TR_Types::eDeformation ) )
      {
        addToBuilder( pNodeTree, nodeRef );
      }
    }
    else if ( pNodeTree->_type == SimpleNodeTree::eDeformationRoot )
    {
      TR_Types::DataRef_t nodeRef = builder.addMatteNode( pNodeTree->_name, parentNodeRef );
      addToBuilder( pNodeTree, nodeRef );
    }
  }
}

void XML_SkeletonSaxParser::dumpTree(const SimpleNodeTreePtr_t &buildingTree, const STD_String &prefix ) const
{
  printf( "%s[%s]\n", prefix.c_str(), buildingTree->_name.c_str() );

  for ( SimpleNodeTree::ChildCol_t::const_iterator i = buildingTree->_childs.begin(),
        iEnd = buildingTree->_childs.end() ; i!=iEnd ; ++i )
  {
    dumpTree( *i, prefix + "  " );
  }
}
