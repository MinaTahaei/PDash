#include "XML_ProjectLoader.h"
#include "XML_StageSaxParser.h"
#include "XML_SkeletonSaxParser.h"
#include "XML_AnimationSaxParser.h"
#include "XML_DrawingAnimationSaxParser.h"
#include "XML_SpriteSheetSaxParser.h"

#include "STD_Types.h"

namespace
{
  void parseXmlFile( const STD_String &filename, XML_SaxParserComponentPtr_t mainComponent )
  {
    XML_SaxParserDelegator delegator(mainComponent);
    XML_SaxParser sax;

    sax.setDelegator( &delegator );
    sax.parse( filename.c_str() );
  }
}

void XML_ProjectLoader::loadStageClipNames( const STD_String &projectFolder, StringCol_t &clipNames )
{
  XML_SaxParserComponentPtr_t mainComponent = new XML_StageNamesSaxParser(projectFolder, clipNames);
  parseXmlFile( projectFolder + "/stage.xml", mainComponent );
}

void XML_ProjectLoader::loadStageClip( const STD_String &projectFolder, const STD_String &clipName, RD_ClipDataCore *clip )
{
  XML_SaxParserComponentPtr_t mainComponent = new XML_StageSaxParser(projectFolder, clipName, clip);
  parseXmlFile( projectFolder + "/stage.xml", mainComponent );
}

void XML_ProjectLoader::loadSkeleton( const STD_String &projectFolder, const STD_String &skeletonName, TR_NodeTree *nodeTree )
{
  XML_SaxParserComponentPtr_t mainComponent = new XML_SkeletonSaxParser(skeletonName, nodeTree);
  parseXmlFile( projectFolder + "/skeleton.xml", mainComponent );
}

void XML_ProjectLoader::loadAnimation( const STD_String &projectFolder, const STD_String &animationName, TR_NodeTree *nodeTree )
{
  XML_SaxParserComponentPtr_t mainComponent = new XML_AnimationSaxParser(animationName, nodeTree);
  parseXmlFile( projectFolder + "/animation.xml", mainComponent );
}

void XML_ProjectLoader::loadDrawingAnimation( const STD_String &projectFolder, const STD_String &drawingAnimationName, TR_NodeTree *nodeTree )
{
  XML_SaxParserComponentPtr_t mainComponent = new XML_DrawingAnimationSaxParser(drawingAnimationName, nodeTree);
  parseXmlFile( projectFolder + "/drawingAnimation.xml", mainComponent );
}

void XML_ProjectLoader::loadSpriteSheetNames( const STD_String &projectFolder, StringPairCol_t &sheetNames )
{
  XML_SaxParserComponentPtr_t mainComponent = new XML_SpriteSheetNamesSaxParser(sheetNames);
  parseXmlFile( projectFolder + "/spriteSheets.xml", mainComponent );
}

void XML_ProjectLoader::loadSpriteSheetResolutionNames( const STD_String &projectFolder, const STD_String &sheetName, StringPairCol_t &sheetNames )
{
  XML_SaxParserComponentPtr_t mainComponent = new XML_SpriteSheetNamesSaxParser(sheetName, sheetNames);
  parseXmlFile( projectFolder + "/spriteSheets.xml", mainComponent );
}

void XML_ProjectLoader::loadSpriteSheet( const STD_String &projectFolder, const STD_String &sheetName, const STD_String &sheetResolution, RD_SpriteSheetCore *sheet )
{
  XML_SaxParserComponentPtr_t mainComponent = new XML_SpriteSheetSaxParser(projectFolder, sheetName, sheetResolution, sheet);
  parseXmlFile( projectFolder + "/spriteSheets.xml", mainComponent );
}

void XML_ProjectLoader::loadSpriteSheet( const STD_String &projectFolder, const STD_String &sheetName, RD_SpriteSheetCore *sheet )
{
  //  Retrieve a default resolution name
  StringPairCol_t sheetNames;
  loadSpriteSheetResolutionNames(projectFolder, sheetName, sheetNames );

  if ( !sheetNames.empty() )
  {
    loadSpriteSheet(projectFolder, sheetName, sheetNames.front().second, sheet);
  }
}
