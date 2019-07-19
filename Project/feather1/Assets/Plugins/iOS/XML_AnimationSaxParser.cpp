
#include "XML_AnimationSaxParser.h"

#include "TR_NodeTree.h"
#include "TR_NodeTreeBuilder.h"
#include "TR_Utils.h"

#include "STD_Types.h"

#include <string.h>
#include <stdio.h>

//  Common tags
#define kNameTag                 "name"
#define kFilenameTag             "filename"
#define kRepeatTag               "repeat"
#define kTimeTag                 "time"

//  Animation group main tags
#define kAnimationColTag         "animations"
#define kAnimationTag            "animation"
#define kAttrLinkColTag          "attrlinks"
#define kAttrLinkTag             "attrlink"
#define kTimedValueColTag        "timedvalues"
#define kTimedValueTag           "timedvalue"
#define kConstantValueTag        "value"
#define kBezierTag               "bezier"
#define kBezierPtTag             "pt"
#define kCatmullTag              "catmull"
#define kCatmullPtTag            "pt"
#define kLinearTag               "linear"
#define kLinearPtTag             "pt"
#define kPivotTag                "pivot"
#define kPivotPtTag              "pt"

//  Animation group variable tags
#define kAttrTag                 "attr"
#define kBezierConstSegTag       "constSeg"
#define kBezierXCoordTag         "x"
#define kBezierYCoordTag         "y"
#define kBezierRXCoordTag        "rx"
#define kBezierRYCoordTag        "ry"
#define kBezierLXCoordTag        "lx"
#define kBezierLYCoordTag        "ly"
#define kCatmullXCoordTag        "x"
#define kCatmullYCoordTag        "y"
#define kCatmullZCoordTag        "z"
#define kCatmullTensionTag       "tension"
#define kCatmullContinuityTag    "continuity"
#define kCatmullBiasTag          "bias"
#define kCatmullTimeTag          "lockedInTime"
#define kCatmullScaleXTag        "scaleX"
#define kCatmullScaleYTag        "scaleY"
#define kCatmullScaleZTag        "scaleZ"
#define kLinearXCoordTag         "x"
#define kLinearYCoordTag         "y"
#define kPivotXCoordTag          "x"
#define kPivotYCoordTag          "y"
#define kPivotZCoordTag          "z"
#define kPivotStartTimeTag       "start"

#define kNodeTag                 "node"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_AnimationSaxParser
#endif

XML_AnimationSaxParser::XML_AnimationSaxParser( const STD_String &animationName, TR_NodeTree *nodeTree ) :
  _animationName(animationName),
  _nodeTree(nodeTree),
  _state(eRoot)
{
}

XML_AnimationSaxParser::~XML_AnimationSaxParser()
{
}

XML_SaxParserComponentPtr_t XML_AnimationSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  if ( _state == eRoot )
  {
    if ( strcmp( nodeName, kAnimationColTag ) == 0 )
    {
      _state = eAnimations;
    }
  }
  else if ( _state == eAnimations )
  {
    if ( strcmp( nodeName, kAnimationTag ) == 0 )
    {
      //  Only enter stage mode if we encounter the xml tag we search for.
      STD_String value;
      if (findAttribute( attributes, kNameTag, value ) && (value.compare(_animationName) == 0))
      {
        _state = eAnimation;
      }
    }
  }
  else if ( _state == eAnimation )
  {
    if ( strcmp( nodeName, kAttrLinkColTag ) == 0 )
    {
      _tvLinks.clear();
      _state = eAttrLinks;
    }
    else if ( strcmp( nodeName, kTimedValueColTag ) == 0 )
    {
      _state = eTimedValues;
    }
  }
  else if ( _state == eAttrLinks )
  {
    if ( strcmp( nodeName, kAttrLinkTag ) == 0 )
    {
      STD_String nodeName, attrName, tvName, constantValue;
      if ( findAttribute( attributes, kNodeTag, nodeName ) &&
           findAttribute( attributes, kAttrTag, attrName ) )
      {
        //  Retrieve first node with specified name.  Create animation data and link
        //  to that node.
        TR_Types::DataRef_t nodeRef = _nodeTree->nodeDataRef(nodeName);

        TR_Types::CurveChannel_t channel;
        bool validChannel = TR_Utils::nameToChannel(attrName, channel);

        if ( validChannel && (nodeRef != TR_Types::g_nullOffset) )
        {
          if ( findAttribute( attributes, kTimedValueTag, tvName ) )
          {
            AttrLinkData data;
            data.first = nodeRef;
            data.second = channel;

            _tvLinks[ tvName ].insert(data);
          }
          else if ( findAttribute( attributes, kConstantValueTag, constantValue ) )
          {
            float value = (float)atof(constantValue.c_str());

            TR_NodeTreeBuilder builder(_nodeTree);
            builder.addConstantValue(channel, nodeRef, value);
          }

          //  Add node to linkedNodes collection.  Once all functions have been created,
          //  we will link animation data to the duplicate nodes so that they share the same data.
          _linkedNodes.insert( nodeName );
        }
      }
    }
  }
  else if ( _state == eTimedValues )
  {
    if ( strcmp( nodeName, kBezierTag ) == 0 )
    {
      STD_String name;
      if ( findAttribute( attributes, kNameTag, name ) )
      {
        TvLinkCol_t::const_iterator i = _tvLinks.find(name);
        if ( i != _tvLinks.end() )
        {
          return new XML_BezierSaxParser( i->second, _nodeTree);
        }
      }
    }
    else if ( strcmp( nodeName, kCatmullTag ) == 0 )
    {
      STD_String name;
      if ( findAttribute( attributes, kNameTag, name ) )
      {
        TvLinkCol_t::const_iterator i = _tvLinks.find(name);
        if ( i != _tvLinks.end() )
        {
          float scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;

          for ( XML_AttributeCol_t::const_iterator iAtt = attributes.begin(), iAttEnd = attributes.end() ; iAtt!=iAttEnd ; ++iAtt )
          {
            if ( strcmp( iAtt->first, kCatmullScaleXTag ) == 0 )
              scaleX = (float)atof(iAtt->second);
            else if ( strcmp( iAtt->first, kCatmullScaleYTag ) == 0 )
              scaleY = (float)atof(iAtt->second);
            else if ( strcmp( iAtt->first, kCatmullScaleZTag ) == 0 )
              scaleZ = (float)atof(iAtt->second);
          }

          return new XML_CatmullSaxParser(i->second, _nodeTree, scaleX, scaleY, scaleZ);
        }
      }
    }
    else if ( strcmp( nodeName, kLinearTag ) == 0 )
    {
      STD_String name;
      if ( findAttribute( attributes, kNameTag, name ) )
      {
        TvLinkCol_t::const_iterator i = _tvLinks.find(name);
        if ( i != _tvLinks.end() )
        {
          return new XML_LinearSaxParser(i->second, _nodeTree);
        }
      }
    }
    else if ( strcmp( nodeName, kPivotTag ) == 0 )
    {
      STD_String name;
      if ( findAttribute( attributes, kNameTag, name ) )
      {
        TvLinkCol_t::const_iterator i = _tvLinks.find(name);
        if ( i != _tvLinks.end() )
        {
          return new XML_PivotSaxParser(i->second, _nodeTree);
        }
      }
    }
  }

  return this;
}

void XML_AnimationSaxParser::endComponent(const char *nodeName)
{
  if ( _state == eTimedValues || _state == eAttrLinks )
  {
    if ( (strcmp( nodeName, kAttrLinkColTag ) == 0) ||
         (strcmp( nodeName, kTimedValueColTag ) == 0) )
    {
      _state = eAnimation;
    }
  }
  else if ( strcmp( nodeName, kAnimationTag ) == 0 )
  {
    //  Iterate on linked nodes collection and link animation data to duplicate nodes.
    for ( StringCol_t::const_iterator i = _linkedNodes.begin(), iEnd = _linkedNodes.end() ; i!=iEnd ; ++i )
    {
      TR_NodeTree::DataRefCol_t dataRefs;
      _nodeTree->nodeDataRefs( *i, dataRefs );

      TR_NodeTree::DataRefCol_t::const_iterator iDataRef = dataRefs.begin();
      TR_NodeTree::DataRefCol_t::const_iterator iDataRefEnd = dataRefs.end();

      TR_Types::DataRef_t srcDataRef = *iDataRef;

      for ( ++iDataRef ; iDataRef!=iDataRefEnd ; ++iDataRef )
      {
        TR_Types::DataRef_t dstDataRef = *iDataRef;

        TR_NodeTreeBuilder builder(_nodeTree);
        builder.linkNodeAnimation( srcDataRef, dstDataRef );
      }
    }

    _linkedNodes.clear();

    _state = eAnimations;
  }
  else if ( strcmp( nodeName, kAnimationColTag ) == 0 )
  {
    _state = eRoot;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_BezierSaxParser
#endif

XML_BezierSaxParser::XML_BezierSaxParser( const AttrLinkCol_t &attrLinks, TR_NodeTree *nodeTree )
{
  _builder = new TR_NodeTreeBuilder(nodeTree);

  //  Create channel entries for duplicate channels.
  AttrLinkCol_t::const_iterator i = attrLinks.begin();
  AttrLinkCol_t::const_iterator iEnd = attrLinks.end();
  for ( ++i ; i!=iEnd ; ++i )
  {
    _duplicateChannels.insert( _builder->createEmptyChannel( i->second, i->first ) );
  }

  const XML_AnimationSaxParser::AttrLinkData &attrLink = *attrLinks.begin();
  _channelDataRef = _builder->beginBezierCurve( attrLink.second, attrLink.first );
}

XML_BezierSaxParser::~XML_BezierSaxParser()
{
  _builder->endBezierCurve();

  //  Create separate channels for remaining attributes with the same function.
  if ( _channelDataRef != TR_Types::g_nullOffset )
  {
    for ( DataRefCol_t::const_iterator i=_duplicateChannels.begin(), iEnd = _duplicateChannels.end() ; i!=iEnd ; ++i )
    {
      _builder->linkChannel( _channelDataRef, *i );
    }
  }

  delete _builder;
}

XML_SaxParserComponentPtr_t XML_BezierSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  if ( _channelDataRef == TR_Types::g_nullOffset )
    return this;

  if ( strcmp( nodeName, kBezierPtTag ) == 0 )
  {
    float x = 0.0f;
    float y = 0.0f;
    float leftx = 0.0f;
    float lefty = 0.0f;
    float rightx = 0.0f;
    float righty = 0.0f;

    bool constSeg = false;

    for ( XML_AttributeCol_t::const_iterator i = attributes.begin(), iEnd = attributes.end() ; i!=iEnd ; ++i )
    {
      if ( strcmp( i->first, kBezierXCoordTag ) == 0 )
        x = (float)atof(i->second);
      else if ( strcmp( i->first, kBezierYCoordTag ) == 0 )
        y = (float)atof(i->second);
      else if ( strcmp( i->first, kBezierRXCoordTag ) == 0 )
        rightx = (float)atof(i->second);
      else if ( strcmp( i->first, kBezierRYCoordTag ) == 0 )
        righty = (float)atof(i->second);
      else if ( strcmp( i->first, kBezierLXCoordTag ) == 0 )
        leftx = (float)atof(i->second);
      else if ( strcmp( i->first, kBezierLYCoordTag ) == 0 )
        lefty = (float)atof(i->second);
      else if ( strcmp( i->first, kBezierConstSegTag ) == 0 )
        constSeg = ( strcmp( i->second, "true" ) == 0 );
    }

    _builder->addBezierPoint(x, y, leftx, lefty, rightx, righty, constSeg);
  }

  return this;
}

void XML_BezierSaxParser::endComponent(const char * /*nodeName*/)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_CatmullSaxParser
#endif

XML_CatmullSaxParser::XML_CatmullSaxParser( const AttrLinkCol_t &attrLinks, TR_NodeTree *nodeTree, float scaleX, float scaleY, float scaleZ )
{
  _builder = new TR_NodeTreeBuilder(nodeTree);

  //  Create channel entries for duplicate channels.
  AttrLinkCol_t::const_iterator i = attrLinks.begin();
  AttrLinkCol_t::const_iterator iEnd = attrLinks.end();
  for ( ++i ; i!=iEnd ; ++i )
  {
    _duplicateChannels.insert( _builder->createEmptyChannel( i->second, i->first ) );
  }

  const XML_AnimationSaxParser::AttrLinkData &attrLink = *attrLinks.begin();
  _channelDataRef = _builder->beginCatmullCurve( attrLink.second, attrLink.first, scaleX, scaleY, scaleZ );
}

XML_CatmullSaxParser::~XML_CatmullSaxParser()
{
  _builder->endCatmullCurve();

  //  Create separate channels for remaining attributes with the same function.
  if ( _channelDataRef != TR_Types::g_nullOffset )
  {
    for ( DataRefCol_t::const_iterator i=_duplicateChannels.begin(), iEnd = _duplicateChannels.end() ; i!=iEnd ; ++i )
    {
      _builder->linkChannel( _channelDataRef, *i );
    }
  }

  delete _builder;
}

XML_SaxParserComponentPtr_t XML_CatmullSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  if ( _channelDataRef == TR_Types::g_nullOffset )
    return this;

  if ( strcmp( nodeName, kBezierPtTag ) == 0 )
  {
    float frame = -1.0f;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float tension = 0.0f;
    float continuity = 0.0f;
    float bias = 0.0f;

    for ( XML_AttributeCol_t::const_iterator i = attributes.begin(), iEnd = attributes.end() ; i!=iEnd ; ++i )
    {
      if ( strcmp( i->first, kCatmullTimeTag ) == 0 )
        frame = (float)atof(i->second);
      else if ( strcmp( i->first, kCatmullXCoordTag ) == 0 )
        x = (float)atof(i->second);
      else if ( strcmp( i->first, kCatmullYCoordTag ) == 0 )
        y = (float)atof(i->second);
      else if ( strcmp( i->first, kCatmullZCoordTag ) == 0 )
        z = (float)atof(i->second);
      else if ( strcmp( i->first, kCatmullTensionTag ) == 0 )
        tension = (float)atof(i->second);
      else if ( strcmp( i->first, kCatmullContinuityTag ) == 0 )
        continuity = (float)atof(i->second);
      else if ( strcmp( i->first, kCatmullBiasTag ) == 0 )
        bias = (float)atof(i->second);
    }

    _builder->addCatmullPoint(frame, x, y, z, tension, continuity, bias);
  }

  return this;
}

void XML_CatmullSaxParser::endComponent(const char * /*nodeName*/)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_LinearSaxParser
#endif

XML_LinearSaxParser::XML_LinearSaxParser( const AttrLinkCol_t &attrLinks, TR_NodeTree *nodeTree )
{
  _builder = new TR_NodeTreeBuilder(nodeTree);

  //  Create channel entries for duplicate channels.
  AttrLinkCol_t::const_iterator i = attrLinks.begin();
  AttrLinkCol_t::const_iterator iEnd = attrLinks.end();
  for ( ++i ; i!=iEnd ; ++i )
  {
    _duplicateChannels.insert( _builder->createEmptyChannel( i->second, i->first ) );
  }

  const XML_AnimationSaxParser::AttrLinkData &attrLink = *attrLinks.begin();
  _channelDataRef = _builder->beginLinearCurve( attrLink.second, attrLink.first );
}

XML_LinearSaxParser::~XML_LinearSaxParser()
{
  _builder->endLinearCurve();

  //  Create separate channels for remaining attributes with the same function.
  if ( _channelDataRef != TR_Types::g_nullOffset )
  {
    for ( DataRefCol_t::const_iterator i=_duplicateChannels.begin(), iEnd = _duplicateChannels.end() ; i!=iEnd ; ++i )
    {
      _builder->linkChannel( _channelDataRef, *i );
    }
  }

  delete _builder;
}

XML_SaxParserComponentPtr_t XML_LinearSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  if ( _channelDataRef == TR_Types::g_nullOffset )
    return this;

  if ( strcmp( nodeName, kBezierPtTag ) == 0 )
  {
    float x = 0.0f;
    float y = 0.0f;

    for ( XML_AttributeCol_t::const_iterator i = attributes.begin(), iEnd = attributes.end() ; i!=iEnd ; ++i )
    {
      if ( strcmp( i->first, kLinearXCoordTag ) == 0 )
        x = (float)atof(i->second);
      else if ( strcmp( i->first, kLinearYCoordTag ) == 0 )
        y = (float)atof(i->second);
    }

    _builder->addLinearPoint(x, y);
  }

  return this;
}

void XML_LinearSaxParser::endComponent(const char * /*nodeName*/)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_PivotSaxParser
#endif

XML_PivotSaxParser::XML_PivotSaxParser( const AttrLinkCol_t &attrLinks, TR_NodeTree *nodeTree )
{
  _builder = new TR_NodeTreeBuilder(nodeTree);

  //  Create channel entries for duplicate channels.
  AttrLinkCol_t::const_iterator i = attrLinks.begin();
  AttrLinkCol_t::const_iterator iEnd = attrLinks.end();
  for ( ++i ; i!=iEnd ; ++i )
  {
    _duplicateChannels.insert( _builder->createEmptyChannel( i->second, i->first ) );
  }

  const XML_AnimationSaxParser::AttrLinkData &attrLink = *attrLinks.begin();
  _channelDataRef = _builder->beginAnimatedPivot( attrLink.second, attrLink.first );
}

XML_PivotSaxParser::~XML_PivotSaxParser()
{
  _builder->endAnimatedPivot();

  //  Create separate channels for remaining attributes with the same function.
  if ( _channelDataRef != TR_Types::g_nullOffset )
  {
    for ( DataRefCol_t::const_iterator i=_duplicateChannels.begin(), iEnd = _duplicateChannels.end() ; i!=iEnd ; ++i )
    {
      _builder->linkChannel( _channelDataRef, *i );
    }
  }

  delete _builder;
}

XML_SaxParserComponentPtr_t XML_PivotSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  if ( _channelDataRef == TR_Types::g_nullOffset )
    return this;

  if ( strcmp( nodeName, kPivotPtTag ) == 0 )
  {
    float frame = -1.0f;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    for ( XML_AttributeCol_t::const_iterator i = attributes.begin(), iEnd = attributes.end() ; i!=iEnd ; ++i )
    {
      if ( strcmp( i->first, kPivotStartTimeTag ) == 0 )
        frame = (float)atof(i->second);
      else if ( strcmp( i->first, kPivotXCoordTag ) == 0 )
        x = (float)atof(i->second);
      else if ( strcmp( i->first, kPivotYCoordTag ) == 0 )
        y = (float)atof(i->second);
      else if ( strcmp( i->first, kPivotZCoordTag ) == 0 )
        z = (float)atof(i->second);
    }

    _builder->addPivotPoint(frame, x, y, z);
  }

  return this;
}

void XML_PivotSaxParser::endComponent(const char * /*nodeName*/)
{
}
