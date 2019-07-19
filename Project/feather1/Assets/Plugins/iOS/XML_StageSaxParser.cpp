#include "XML_StageSaxParser.h"
#include "XML_ProjectLoader.h"

#include "TR_NodeTree.h"
#include "TR_NodeTreeBuilder.h"

#include "RD_ClipDataCore.h"

#include "STD_Types.h"

#include <string.h>

//  Common tags
#define kNameTag               "name"
#define kFilenameTag           "filename"
#define kRepeatTag             "repeat"

//  Stage group main tags
#define kStageColTag           "stages"
#define kStageTag              "stage"
#define kPropColTag            "props"
#define kPlayTag               "play"
#define kPropTag               "prop"
#define kSoundTag              "sound"

//  Stage group variable tags
#define kSkeletonTag           "skeleton"
#define kAnimationTag          "animation"
#define kDrawingAnimationTag   "drawingAnimation"

#define kTimeTag               "time"
#define kParentSkeletonTag     "parent"
#define kParentBoneTag         "bone"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_StageSaxParser
#endif

XML_StageSaxParser::XML_StageSaxParser( const STD_String &projectFolder, const STD_String &clipName, RD_ClipDataCore *clip ) :
  _projectFolder(projectFolder),
  _clipName(clipName),
  _state(eRoot),
  _clip(clip)
{
  _clip->beginClipData();
}

XML_StageSaxParser::~XML_StageSaxParser()
{
  _clip->endClipData();
}

XML_SaxParserComponentPtr_t XML_StageSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  if ( _state == eRoot )
  {
    if ( strcmp( nodeName, kStageColTag ) == 0 )
    {
      _state = eStages;
    }
  }
  else if ( _state == eStages )
  {
    if ( (strcmp( nodeName, kStageTag ) == 0) ||
         (strcmp( nodeName, kPropColTag ) == 0) )
    {
      //  Only enter stage mode if we encounter the xml tag we search for.
      STD_String value;
      if (findAttribute( attributes, kNameTag, value ) && (value.compare(_clipName) == 0))
      {
        _state = eStage;
      }
    }
  }
  else // if ( _state == eStage )
  {
    if ( (strcmp( nodeName, kPlayTag ) == 0) ||
         (strcmp( nodeName, kPropTag ) == 0) )
    {
      TR_NodeTree *nodeTree = new TR_NodeTree;

      STD_String value;
      if (findAttribute(attributes, kSkeletonTag, value))
      {
        XML_ProjectLoader::loadSkeleton(_projectFolder, value, nodeTree);
      }
      if (findAttribute(attributes, kAnimationTag, value))
      {
        XML_ProjectLoader::loadAnimation(_projectFolder, value, nodeTree);
      }
      if (findAttribute(attributes, kDrawingAnimationTag, value))
      {
        XML_ProjectLoader::loadDrawingAnimation(_projectFolder, value, nodeTree);
      }

      STD_String playName;
      findAttribute(attributes, kNameTag, playName);

      //nodeTree->dump();
      _clip->addNodeTree(nodeTree, playName );
    }
    else if ( strcmp( nodeName, kSoundTag ) == 0 )
    {
      float startFrame = 1.0f; // default at first frame.
      STD_String soundName;

      STD_String value;
      if (findAttribute(attributes, kNameTag, value))
      {
        soundName = value;
      }
      if (findAttribute(attributes, kTimeTag, value))
      {
        startFrame = (float)atof(value.c_str());
      }

      _clip->addSoundEvent(soundName, startFrame);
    }
  }

  return this;
}

void XML_StageSaxParser::endComponent(const char *nodeName)
{
  if ( strcmp( nodeName, kStageTag ) == 0 )
  {
    _state = eStages;
  }
  else if ( strcmp( nodeName, kStageColTag ) == 0 )
  {
    _state = eRoot;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - XML_StageNamesSaxParser
#endif

XML_StageNamesSaxParser::XML_StageNamesSaxParser( const STD_String &projectFolder, StringCol_t &clipNames ) :
  _projectFolder(projectFolder),
  _state(eRoot),
  _clipNames(clipNames)
{
}

XML_StageNamesSaxParser::~XML_StageNamesSaxParser()
{
}

XML_SaxParserComponentPtr_t XML_StageNamesSaxParser::startComponent(const char *nodeName, const XML_AttributeCol_t &attributes)
{
  if ( _state == eRoot )
  {
    if ( strcmp( nodeName, kStageColTag ) == 0 )
    {
      _state = eStages;
    }
  }
  else if ( _state == eStages )
  {
    if ( strcmp( nodeName, kStageTag ) == 0 )
    {
      //  Only enter stage mode if we encounter the xml tag we search for.
      STD_String clipName;
      if (findAttribute( attributes, kNameTag, clipName ))
      {
        _clipNames.push_back(clipName);
        _state = eStage;
      }
    }
  }

  return this;
}

void XML_StageNamesSaxParser::endComponent(const char *nodeName)
{
  if ( strcmp( nodeName, kStageTag ) == 0 )
  {
    _state = eStages;
  }
  else if ( strcmp( nodeName, kStageColTag ) == 0 )
  {
    _state = eRoot;
  }
}

