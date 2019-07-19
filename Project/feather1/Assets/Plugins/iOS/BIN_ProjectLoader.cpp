#include "BIN_ProjectLoader.h"

#include "PL_FileSpec.h"
#include "IO_PersistentStore.h"
#include "IO_File.h"
#include "STD_Types.h"

#include "RD_ClipDataCore.h"
#include "RD_SpriteSheetCore.h"

void BIN_ProjectLoader::loadStageClipNames( const STD_String &projectFolder, StringCol_t &clipNames )
{
  // Todo, remove need for PL_FileSpec here!
  PL_FileSpec fileSpec( (projectFolder + "/stages").c_str() );

  for ( PL_FileSpec::iterator i = fileSpec.begin(), iEnd = fileSpec.end() ; i!=iEnd ; ++i )
  {
    clipNames.push_back( i->GetName( false /* no extension */ ).utf8() );
  }
}

bool BIN_ProjectLoader::loadStageClip( const STD_String &projectFolder, const STD_String &clipName, RD_ClipDataCore *clipData )
{
  // Todo, remove need for PL_FileSpec here!
  PL_FileSpec fileSpec( (projectFolder + "/stages/" + clipName + ".bin").c_str() );
  if ( !fileSpec.IsFile() )
  {
    return false;
  }


  IO_File file;
  if ( !file.openForInput( fileSpec ) )
    return false;

  IO_PersistentStore store(file);
  clipData->load( store );

  file.close();

  return true;
}

void BIN_ProjectLoader::loadSpriteSheetNames( const STD_String &projectFolder, StringPairCol_t &sheetNames )
{
  // Todo, remove need for PL_FileSpec here!
  PL_FileSpec fileSpec( (projectFolder + "/spriteSheets").c_str() );

  for ( PL_FileSpec::iterator i = fileSpec.begin(), iEnd = fileSpec.end() ; i!=iEnd ; ++i )
  {
    if ( i->GetExtension() == UT_String( "bin" ) )
    {
      UT_String spriteSheetName = i->GetName( false /* no extension */ );

      size_t idx = spriteSheetName.find ( "-" );
      if ( idx != STD_String::npos )
      {
        StringPair_t sheetName;

        sheetName.first = spriteSheetName.substr( 0, idx ).utf8();
        sheetName.second = spriteSheetName.substr( idx+1 ).utf8();

        sheetNames.push_back( sheetName );
      }
    }
  }
}

void BIN_ProjectLoader::loadSpriteSheetResolutionNames( const STD_String &projectFolder, const STD_String &sheetName, StringPairCol_t &resolutionNames )
{
  // Todo, remove need for PL_FileSpec here!
  StringPairCol_t sheetNames;
  loadSpriteSheetNames( projectFolder, sheetNames );

  for ( StringPairCol_t::const_iterator i = sheetNames.begin(), iEnd = sheetNames.end() ; i!=iEnd ; ++i )
  {
    if ( i->first.compare( sheetName ) == 0 )
    {
      resolutionNames.push_back( *i );
    }
  }
}

bool BIN_ProjectLoader::loadSpriteSheet( const STD_String &projectFolder, const STD_String &sheetName, const STD_String &resolutionName, RD_SpriteSheetCore *sheet )
{
  PL_FileSpec fileSpec( (projectFolder + "/spriteSheets/" + sheetName + "-" + resolutionName + ".bin").c_str() );
  if ( !fileSpec.IsFile() )
  {
    return false;
  }

  IO_File file;
  if ( !file.openForInput( fileSpec ) )
    return false;

  IO_PersistentStore store(file);
  sheet->load( store );

  file.close();

  return true;
}

bool BIN_ProjectLoader::loadSpriteSheet( const STD_String &projectFolder, const STD_String &sheetName, RD_SpriteSheetCore *sheet )
{
  //  Retrieve a default resolution name
  StringPairCol_t sheetNames;
  loadSpriteSheetResolutionNames(projectFolder, sheetName, sheetNames );

  if ( !sheetNames.empty() )
  {
    return loadSpriteSheet(projectFolder, sheetName, sheetNames.front().second, sheet);
  }

  return false;
}
