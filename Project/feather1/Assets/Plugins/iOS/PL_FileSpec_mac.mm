//========== Copyright ToonBoom Technologies Inc. 2001 ============
//
// SOURCE FILE : PL_FileSpec.cpp
// MODULE NAME : 
// DESCRIPTION : 
//
//=================================================================
// Author : Sandy Martel
// Modif  : 
//
// Creation Date		 : 2001-9-25
//========================VSS Auto=================================
// $Revision: 1.36.2.3.2.8 $
// $Date: 2007/05/10 18:20:18 $
// $Author: begin $
//=================================================================
// REVISION: 
// 
//=========== Copyright ToonBoom Technologies Inc. 2001  ==========

//=================================================================
//						INCLUDE
//=================================================================
#include "PL_FileSpec.h"

#include <algorithm>
#include <limits>
#include <errno.h>

#include <string.h>
#include <fcntl.h>

#include <time.h>
#include <utime.h>

#import "Foundation/Foundation.h"

#include <vector>

//	Platform includes and path separator define
#include <sys/stat.h>
#include <unistd.h>

#if 0
#pragma mark -
#pragma mark *** Special init ***
#endif

int PL_FileSpec::InitUserDocumentsFolder()
{
  NSFileManager *filemgr = [ NSFileManager defaultManager ];
  NSURL *url = [[ filemgr URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject];
  if( url )
  {
    NSString *string = [ url path ];
    if( string )
    {
      m_path = UT_String( [ string UTF8String  ] );
      return 0;
    }
  }
  return -1;
}

UT_String PL_FileSpec::UserDocumentDirectory()
{
  UT_String ret;
  
  NSFileManager *fManager = [NSFileManager defaultManager];
  NSString *path = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents/GamePreviewer"];
  
  BOOL isDir;
  if( ![fManager fileExistsAtPath:path isDirectory:&isDir] )
  {
    if( ![fManager createDirectoryAtPath:path attributes:nil] )
    {
      NSLog(@"Error : Can't create folder %@", path);
      return ret;
    }
  }
  ret = [path UTF8String];
  
  return ret;
}

#if 0
#pragma mark -
#pragma mark *** Actions ***
#endif

int PL_FileSpec::FileCreate( const UT_String &fileName ) const
{
  std::string splitter ("/");
  std::string dest( m_path.begin(), m_path.end() );
  if( std::strcmp( &dest[dest.size()-1], splitter.c_str() ) != 0 )
    dest += splitter;

  dest.append( fileName.begin(), fileName.end() );

  int fd = -1;
  
  NSString *path = [NSString stringWithUTF8String:dest.c_str()];
  NSFileManager* fManager = [NSFileManager defaultManager];

  if( ![fManager createFileAtPath:path contents:[NSData data] attributes:nil] )
    NSLog( @"Can't create file at path : %@", path );
  
  fd = open( dest.c_str(), S_IWOTH );
  return fd;
}
