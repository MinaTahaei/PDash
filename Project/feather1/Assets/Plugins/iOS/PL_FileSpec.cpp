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

#if defined(TARGET_WIN32)
  #include <sys/utime.h>
  #include <Windows.h>
  #include <io.h>
  #define utime _utime
  #define open _open
#else
  #include <utime.h>
  #include <unistd.h>
#endif
#include <fstream>

#include <vector>

//	Platform includes and path separator define
#include <sys/stat.h>

const UniChar_t kPathSeparatorChar	= '/';
const UniChar_t kSameLevelPath[]	= { '.','/',0 };
//const UniChar_t kUpOneLevelPath[]	= { '.','.','/',0 };

//	introducinf std namespace
using namespace std;

//	destructor
PL_FileSpec::~PL_FileSpec()
{
}


#if 0
#pragma mark -
#pragma mark *** empty init ***
#endif

//	Init an empty FileSpec,
//	FileSpec will be invalid, all operation on it will fail
PL_FileSpec::PL_FileSpec()
{
}

void PL_FileSpec::IFileSpec()
{
  m_path.erase();
  m_info.Clear();
}

#if 0
#pragma mark -
#pragma mark *** copy constructor ***
#endif

//	resulting FileSpec will refer to the same position in
//	the file system
PL_FileSpec::PL_FileSpec( const PL_FileSpec &i_other )
{
  m_path = i_other.m_path;
  m_info = i_other.m_info;
}

int PL_FileSpec::IFileSpec( const PL_FileSpec &i_other )
{
  *this = i_other;
  return 0;
}

PL_FileSpec &PL_FileSpec::operator=( const PL_FileSpec &i_other )
{
  if ( this != &i_other )
  {
    m_path = i_other.m_path;
    m_info = i_other.m_info;
  }
  return *this;
}

#if 0
#pragma mark -
#pragma mark *** Special init ***
#endif

//	FileSpec will point to a "speciel position in the file system
int PL_FileSpec::IFileSpec( SpecialFolderType i_code )
{
  IFileSpec();
  int	err = 0;
  switch ( i_code )
  {
  case kTempFile:
    err = InitTempFile();
    break;
#if 0      
  case kUserFolder:
    err = InitUserFolder();
    break;
#endif      
  case kUserDocumentsFolder:
    err = InitUserDocumentsFolder();
    break;
#if 0      
  case kUserMoviesFolder:
    err = InitUserMoviesFolder();
    break;
  case kUserMusicFolder:
    err = InitUserMusicFolder();
    break;
  case kUserPicturesFolder:
    err = InitUserPicturesFolder();
    break;
#endif      
  case kUserAppDataFolder:
    err = InitUserAppDataFolder();
    break;
//  case kSessionTempFolder:
//    err = InitSessionTempFolder();
//    break;
  default:
    err = -1;
    break;
  }
  if ( err != 0 )
    IFileSpec();
  return err;
}


int PL_FileSpec::InitTempFile()
{
  return 0;
}

#if !( defined( TARGET_IOS ) || defined( TARGET_MAC ) )
int PL_FileSpec::InitUserDocumentsFolder()
{
  return 0;
}
#endif

int PL_FileSpec::InitUserAppDataFolder()
{
  return 0;
}


#if 0
#pragma mark -
#pragma mark *** relative path interface ***
#endif

// Init with a relative path.
//	relative path must be of unix style with / (\ are accepted)
//	no HFS path accepted
int PL_FileSpec::IFileSpec( const UT_String &i_relative, const PL_FileSpec &i_absolute )
{
  int	err = IFileSpec( i_absolute );
  if ( err == 0 )
  {
    //	work copy of relative path
    UT_String	relativePath( i_relative );

    while ( !relativePath.empty() && err == 0 )
    {
      UT_String::size_type	pos = relativePath.find( UniChar_t('/'), 1 );
      UT_String	component = relativePath.substr( 0, pos );
      relativePath.erase( 0, pos );
      relativePath.erase( 0, 1 );

      if ( component.empty() || component == "." )
      {
        //	do nothing
      }
      else if ( component == ".." )
      {
        //	up one level
        err = Up();
      }
      else
      {
        err = Add( component );
      }
    }
    if ( err != 0 )
      IFileSpec();
  }
  return err;
}


//	get a unix style relative path
int PL_FileSpec::GetRelativePath( UT_String &o_relative, const PL_FileSpec &i_absolute ) const
{
  // can't create portable savable path that is not relative
  if ( !i_absolute.IsValid() )
    return -1;

  //	start with an empty path
  o_relative.erase();

  //	get the 2 paths
  UT_String	absolutePath( i_absolute.m_path );
  UT_String	this_path( m_path );
#if _fl_win32
  //  replace \ with /
  replace( absolutePath.begin(), absolutePath.end(), UniChar_t('\\'), UniChar_t('/') );
  replace( this_path.begin(), this_path.end(), UniChar_t('\\'), UniChar_t('/') );
#endif

  //	find the shared part of the 2 paths
  pair<UT_String::const_iterator, UT_String::const_iterator>	diff =
    mismatch( this_path.begin(),
    this_path.begin() + std::min( this_path.length(), absolutePath.length() ),
    absolutePath.begin() );

  if ( diff.first == this_path.begin() )
    return -1;	//	shared part is empty!, can't create a relative path to another volume!

  //	back to the last folder
  while ( diff.first != this_path.begin() && *diff.first != UniChar_t('/') )
  {
    --diff.first;
    --diff.second;
  }

  //	count number of folder to go back
  int i = count<UT_String::const_iterator,char>( diff.second, absolutePath.end(), UniChar_t('/') );
  if ( i == 0 )
  {
    o_relative.append( "./" );
  }
  else
  {
    for ( ; i > 0; --i )
      o_relative.append( "../" );
  }
  UT_String::const_iterator first = diff.first + 1;
  UT_String::const_iterator last = this_path.end();
#ifndef _STLP_DEBUG
  o_relative.append( first, last );
#else
  for ( ; first < last; ++first )
    o_relative.push_back( *first );
#endif
  return 0;
}

#if 0
#pragma mark -
#pragma mark *** absolute path interface ***
#endif

PL_FileSpec::PL_FileSpec( const UT_String &i_path, const PL_FileInfo &info )
: m_path( i_path ), m_info( info )
{
#if _fl_win32
  std::replace( m_path.begin(), m_path.end(), L'\\', kPathSeparatorChar );
#endif
}

int PL_FileSpec::IFileSpec( const UT_String &i_path, const PL_FileInfo &info )
{
  m_path = i_path;
#if _fl_win32
  std::replace( m_path.begin(), m_path.end(), L'\\', kPathSeparatorChar );
#endif
  m_info = info;
  return 0;
}

//	Init with an absolute path
PL_FileSpec::PL_FileSpec( const UT_String& i_path )
{
  IFileSpec( i_path );
}

#if 0
PL_FileSpec::PL_FileSpec( const UT_RelPath& i_path )
{
  IFileSpec( i_path.ToString() );
}
#endif

int PL_FileSpec::IFileSpec( const UT_String &i_path )
{
  IFileSpec();
  m_path = i_path;
  
#if 0  
  
  //	check for absolute well formed path
  bool	absolutePath;
  QFileInfo	finfo( oswRemapPath::RemapPath2(i_path.ToQString()) );
  UT_String path( finfo.absFilePath() );
#if _fl_posix
  //	for unix, an absolute path starts with a slash
  absolutePath = (path.length() > 0) && (path[0] == kPathSeparatorChar);
#else
  //	replace backslashes with slashes
  replace( path.begin(), path.end(), UniChar_t('\\'), kPathSeparatorChar );
  //	win32 absolute path begin with a drive letter(local path)
  //	or a UNC (file server)
  //	check local path
  if ( (path.length() >= 2 &&
    path[0] >= 0 && path[0] <= numeric_limits<char>::max() &&
    isalpha( path[0] ) &&
    path[1] == UniChar_t(':')) )
  {
    m_path = path;
    while (  m_path.size() > 3 && m_path[m_path.length()-1] == kPathSeparatorChar )
      m_path.erase( m_path.length() - 1 );
    return 0;
  }
  else
  {
    absolutePath = false;
    //	check UNC
    if ( path.length() >= 5 &&	//	at least 5 chars for a UNC : //s/d where 's' is server name and 'd' is share
      path[0] == kPathSeparatorChar && path[1] == kPathSeparatorChar )
    {
      UT_String::size_type pos = path.find( kPathSeparatorChar, 2 );
      if ( pos != UT_String::npos && pos > 2 && (pos+1) < path.length() )
        absolutePath = true;
    }
  }
#endif
  if ( absolutePath )
  {
    m_path = path;
    while ( m_path.size()>1  && m_path[m_path.length()-1] == kPathSeparatorChar )
      m_path.erase( m_path.length() - 1 );
    return 0;
  }
  else
    return -1;
#else
  return -1;
#endif
}

#if !( defined( TARGET_IOS ) || defined( TARGET_MAC ) )
UT_String PL_FileSpec::UserDocumentDirectory()
{
  return "";
}
#endif

//	Get a complete file path
//		specify a unique file
UT_String PL_FileSpec::GetFilePath() const
{
  return m_path;
}

#if 0
#pragma mark -
#pragma mark *** Mac OS HFS+ API ***
#endif

#ifdef TU_MACOS //  Do we still need that ? 

//	init with HFS+ API reference
PL_FileSpec::PL_FileSpec( const FSRef &i_ref )
{
  IFileSpec( i_ref );
}

PL_FileSpec::PL_FileSpec( const FSRef &i_parentRef, const UT_String &i_name )
{
  IFileSpec( i_parentRef, i_name );
}

int PL_FileSpec::IFileSpec( const FSRef &i_ref )
{
  IFileSpec();
  int	err = FSGetCatalogInfo( &i_ref, kFSCatInfoNone, 0, 0, 0, 0 );
  if ( err == 0 )
  {
    UInt8   path[2048];
    err = ::FSRefMakePath( &i_ref, path, 2048 );
    if ( err == 0 )
      m_path.assign( (const char *)path );
  }
  return err;
}

int PL_FileSpec::IFileSpec( const FSRef &i_parentRef, const UT_String &i_name )
{
  IFileSpec();
  int	err = FSGetCatalogInfo( &i_parentRef, kFSCatInfoNone, 0, 0, 0, 0 );
  if ( err == 0 )
  {
    if ( i_name.empty() )
      err = IFileSpec( i_parentRef );
    else
    {
      UInt8   path[2048];
      err = ::FSRefMakePath( &i_parentRef, path, 2048 );
      if ( err == 0 )
        m_path.assign( (const char *)path );
      err = Add( i_name );
    }
  }
  return err;
}

//	Get HFS+ API reference
int PL_FileSpec::GetFSRef( FSRef &o_ref ) const
{
  return FSPathMakeRef( (const UInt8 *)GetUTF8Path().c_str(), &o_ref, 0 );
}

int PL_FileSpec::GetFSRef( FSRef &o_parentRef, UT_String &o_name ) const
{
  int	err = -1;
  if ( IsValid() )
  {
    PL_FileSpec parent( *this );
    parent.Up();
    o_name = GetName();
    err = parent.GetFSRef( o_parentRef );
  }
  return err;
}
#endif

#if 0
#pragma mark -
#pragma mark *** FSSpec API ***
#endif

#if 0 //_win32_ || defined(TU_MACOS)

//	old FSSpec API for using QuickTime
PL_FileSpec::PL_FileSpec( const FSSpec &i_spec )
{
  IFileSpec( i_spec );
}

int PL_FileSpec::IFileSpec( const FSSpec &i_spec )
{
#ifdef TARGET_WIN32
  //	todo: unicode?
  IFileSpec();
  char	buffer[256];
  int	err = FSSpecToNativePathName( &i_spec, buffer, 256, kFullNativePath );
  if ( err == 0 )
    UT_A2W( buffer, m_path );
  return err;

#else
  IFileSpec();
  //	try to extract FSRef directly
  FSRef   fsref;
  int	err = ::FSpMakeFSRef( &i_spec, &fsref );
  if ( err == fnfErr )
  {
    //	file don't exist yet
    //	get parent FSRef and unicode file name
    err = 0;
    FSSpec	spec = i_spec;
    if ( spec.name[0] == 0 )
      err = ::FSMakeFSSpec( spec.vRefNum, spec.parID, 0, &spec );

    if ( err == 0 )
    {
      FSSpec	parent;
      err = ::FSMakeFSSpec( spec.vRefNum, spec.parID, 0, &parent );
      if ( err == 0 )
      {
        err = ::FSpMakeFSRef( &parent, &fsref );
        if ( err == 0 )
        {
          err = IFileSpec( fsref );
          if ( err == 0 )
          {
            CFStringRef	strref = ::CFStringCreateWithPascalString( kCFAllocatorDefault, spec.name, CFStringGetSystemEncoding() );
            err = Add( strref );
            ::CFRelease( strref );
          }
        }
      }
    }
  }
  else
    err = IFileSpec( fsref );
  return err;
#endif
}

FSSpec &PL_FileSpec::GetFSSpec( FSSpec &o_spec ) const
{
  int	err = -1;
#ifdef TARGET_WIN32
  //	todo: unicode?
  UT_String s( GetFilePath() );

  // It seems that starting with QT 7.0.4, The function ::CreateMovieFile does not accept FSSpec with
  // backslash in filename. So now, we make sure that we put back the backslash in the filename (on Windows only).
  replace( s.begin(), s.end(), kPathSeparatorChar, UniChar_t('\\') );

  string tmp;
  UT_W2S( s, tmp );
  err = NativePathNameToFSSpec( const_cast<char *>( tmp.c_str() ), &o_spec, 0 );
#else
  FSRef	fsref;
  err = GetFSRef( fsref );
  if ( err == 0 )
  {
    //	file exist, get FSSpec
    err = ::FSGetCatalogInfo( &fsref, kFSCatInfoNone, 0, 0, &o_spec, 0 );
  }
  else if ( err == fnfErr )
  {
    UT_String   name;
    if ( GetFSRef( fsref, name ) == 0 )
    {
	  // Create file to get a valid FSSpec
	  err = ::FSCreateFileUnicode(&fsref, name.length(), name.c_str(), kFSCatInfoNone, NULL /*catalogInfo*/, NULL /*newRef*/, &o_spec);
	  /*
      //	This may failed if the name is longer than 31 characters!
      FSCatalogInfo	catalogInfo;
      err = FSGetCatalogInfo( &fsref, kFSCatInfoVolume+kFSCatInfoNodeID, &catalogInfo, 0, 0, 0 );
      if ( err == 0 )
      {
        o_spec.vRefNum = catalogInfo.volume;
        o_spec.parID = catalogInfo.nodeID;
        CFStringRef	strref = name.ToCFString();
        if ( not CFStringGetPascalString( strref, o_spec.name, sizeof( o_spec.name ), CFStringGetSystemEncoding() ) )
          err = -1;
        ::CFRelease( strref );
      }
	  */
    }
  }
#endif
  if ( err != 0 )
  {
    //	error, set to invalid
    o_spec.vRefNum = 0x7FFF;
    o_spec.parID = 0;
    o_spec.name[0] = 0;
  }
  return o_spec;
}
#endif

#if 0
#pragma mark -
#pragma mark *** Savable ID ***
#endif

#if 0
#pragma mark -
#pragma mark *** Accessors ***
#endif

// Get file name (file or folder).
UT_String PL_FileSpec::GetName( bool i_withExtension ) const
{
  UT_String   res;
  if ( IsValid() )
  {
    // find the last component of the path
    UT_String::size_type	pos = m_path.rfind( kPathSeparatorChar );
    if ( m_path.length() > 1 && pos == (m_path.length() - 1) )
      pos = m_path.rfind( kPathSeparatorChar, pos - 1 );

    if ( pos == UT_String::npos )
      pos = 0;
    else
      ++pos;

    //	find the . for the extension
    UT_String::size_type	size = UT_String::npos;
    if ( !i_withExtension )
    {
      size = m_path.find_last_of( kSameLevelPath );
      if ( size != UT_String::npos && size > pos )
        size -= pos;
    }
    res = m_path.substr( pos, size );
    if ( res.length() > 1 && res[res.length()-1] == kPathSeparatorChar )
      res.resize( res.length() - 1 );
  }
  return res;
}


// Get file extension without the .
UT_String &PL_FileSpec::GetExtension( UT_String &o_ext ) const
{
  UT_String::size_type	pos = m_path.rfind( UniChar_t('.') );
  if ( pos == 0 || pos == UT_String::npos )
    o_ext.erase();
  else
    o_ext = m_path.substr( pos + 1 );
  return o_ext;
}

UT_String PL_FileSpec::GetExtension() const
{
  UT_String	tmp;
  GetExtension( tmp );
  return tmp;
}

// Test file extension
bool PL_FileSpec::HasExtension( const UT_String &i_ext ) const
{
  UT_String	ext;
  GetExtension( ext );
  return i_ext == ext;
}

//	set the file name extension, will replace the existing extension
//	if there is one.
void PL_FileSpec::SetExtension( const UT_String &i_ext )
{
  if ( !IsValid() )
    return;

  m_info.Clear();
  UT_String::size_type	pos = m_path.find_last_of( kSameLevelPath, UT_String::npos );
  if ( pos != UT_String::npos && m_path[pos] != kPathSeparatorChar && pos > 1 && m_path[pos-1] == kPathSeparatorChar )
    pos = UT_String::npos;

  if ( pos == UT_String::npos || m_path[pos] == kPathSeparatorChar )
  {
    if ( !i_ext.empty() )
    {
      //	no extension, append
      m_path.append( 1, UniChar_t('.') );
      m_path.append( i_ext );
    }
  }
  else
  {
    //	replace existing extension
    if ( !i_ext.empty() )
      m_path.replace( pos + 1, UT_String::npos, i_ext );
    else
      m_path.erase( pos );
  }
}

//	append an extension to the file name
void PL_FileSpec::AddExtension( const UT_String &i_ext )
{

  if ( !IsValid() || i_ext.empty() )
    return;

  m_info.Clear();
  m_path.append( 1, UniChar_t('.') );
  m_path.append( i_ext );
}
bool PL_FileSpec::IsSymbolicLink() const
{
  if( m_info.IsInit() == false )
    m_info = PL_FileInfo( m_path );
  return m_info.IsSymbolicLink();
}


// Is this a file?
bool PL_FileSpec::IsFile() const
{
  if( m_info.IsInit() == false )
    m_info = PL_FileInfo( m_path );
  return m_info.IsFile();
}

// Is this a folder?
bool PL_FileSpec::IsFolder() const
{
  if( m_info.IsInit() == false )
    m_info = PL_FileInfo( m_path );
  return m_info.IsFolder();
}



//
//  True if the given path looks like a windows  share.  (//machine/share);
//
bool PL_FileSpec::IsShare() const
{
  if( m_path.size() > 3 && 
    m_path[0] == kPathSeparatorChar && 
    m_path[1] == kPathSeparatorChar && 
    m_path[2] != kPathSeparatorChar )
  {
    UT_String::const_iterator c = m_path.begin()+3;
    UT_String::const_iterator cEnd = m_path.end();
    while( c != cEnd && *c != kPathSeparatorChar )
    {
      ++c;
    }
    if( c != cEnd )
    {
      ++c; // skip separator.
      while( c != cEnd )
      {
        if( *c == kPathSeparatorChar )
          return false;
        ++c;
      }
      return true;
    }
  }
  return false;
}


// Is this file or folder spec is valid?
//	A valid FileSpec, specify a position in the current file
//	system, this mean that it specify an existing file or at least
//	that its parent folder exist
bool PL_FileSpec::IsValid() const
{
  if ( this == NULL )
    return false;

  return !m_path.empty();
}

//========================  METHOD DECLARATION  =======================
// METHOD NAME : TestAccess
//---------------------------------------------------------------------
/// \param bWriteAccess  - access for read/write access.
/// \return 0 if success ( folder/file exists ) or the error code 
//---------------------------------------------------------------------
// DESCRIPTION :
/// Is this file or folder exist?
//=====================================================================
int PL_FileSpec::TestAccess(bool /*bWriteAccess*/) const
{
  if( m_info.IsInit() == false )
    m_info = PL_FileInfo( m_path );

  bool baccess = true/*( bWriteAccess ? m_info.CanWrite() : m_info.CanRead() )*/;
  return baccess ? 0 : -1;
}

//	return the lenght of the specified file
//	or 0 if file doesn't exist
size_t PL_FileSpec::GetFileLength() const
{
  if( m_info.IsInit() == false )
    m_info = PL_FileInfo( m_path );
  return m_info.GetFileLength();
}


//	Return the file time creation and last modification
//		(beware: not sure if its the same on all platform)
int PL_FileSpec::GetFileTime( time_t &o_lastmodif ) const
{
  if( m_info.IsInit() == false )
    m_info = PL_FileInfo( m_path );
  o_lastmodif = m_info.GetModifTime();
  return m_info.IsError() ? -1 : 0;
}

int PL_FileSpec::SetFileTime( const std::string & /*time*/ )
{
#if 0
  struct stat newTime;
  struct tm *setTime;
  
  std::string filePath( m_path.begin(), m_path.end() );
  
  // Time structure is "NameDay numDay Month Years hh:mm:ss GMT"
  // So we need to parse it to extract the time only  
  stat( filePath.c_str(), &newTime );
  setTime = localtime( &newTime.st_mtime );
  
  std::size_t firstStep = time.find(":");
  std::size_t seconStep = time.rfind(":");
  setTime->tm_hour = atoi( time.substr( 0, firstStep ).c_str() );
  setTime->tm_min = atoi( time.substr( firstStep+1, seconStep).c_str() );
  setTime->tm_sec = atoi( time.substr( seconStep+1 ).c_str() );
  
  /*
  std::stringstream ss;
  ss << std::setfill('0')<< std::setw(2) << setTime->tm_hour << ":" 
  << std::setfill('0')<< std::setw(2) << setTime->tm_min << ":" 
  << std::setfill('0')<< std::setw(2) << setTime->tm_sec;
  
  std::cout << "    DEBUG generated time :" << ss.str() << "\n";
  */
  
#ifdef TARGET_WIN32
  struct _utimbuf sTime;
#else
  struct utimbuf sTime;
#endif
  sTime.actime = mktime( setTime );
  sTime.modtime = mktime( setTime );
  utime( filePath.c_str(), &sTime );
#endif
  return 0;
}

// refer to same file.
bool PL_FileSpec::operator == ( const PL_FileSpec &i_other ) const
{
  return m_path == i_other.m_path;
}

//========================  METHOD DECLARATION  =======================
// METHOD NAME : operator<
//---------------------------------------------------------------------
/// \param i_file to compare with this one
/// \return true if this one is smaller than i_file
//---------------------------------------------------------------------
// DESCRIPTION :
/// Compare 2 filespec (needed to insert PL_FileSpec in map or set)
//=====================================================================
bool PL_FileSpec::operator<(const PL_FileSpec& i_file) const
{
  return (m_path < i_file.m_path);
}


#ifdef TU_MACOS //  Do we still need that ? 
// File type management for Mac OS
int PL_FileSpec::GetTypeAndCreator( OSType &o_type, OSType &o_creator ) const
{
  FSRef	fsref;
  int	err = GetFSRef( fsref );
  if ( err == 0 )
  {
    FSCatalogInfo	catalogInfo;
    err = FSGetCatalogInfo( &fsref, kFSCatInfoFinderInfo, &catalogInfo, 0, 0, 0 );
    if ( err == 0 )
    {
      o_type = ((FileInfo *)catalogInfo.finderInfo)->fileType;
      o_creator = ((FileInfo *)catalogInfo.finderInfo)->fileCreator;
    }
  }
  return err;
}

int PL_FileSpec::SetTypeAndCreator( OSType i_type, OSType i_creator )
{
  FSRef	fsref;
  int	err = GetFSRef( fsref );
  if ( err == 0 )
  {
    FSCatalogInfo	catalogInfo;
    err = FSGetCatalogInfo( &fsref, kFSCatInfoFinderInfo, &catalogInfo, NULL, NULL, NULL );
    if ( err == 0 )
    {
      ((FileInfo *)catalogInfo.finderInfo)->fileType = i_type;
      ((FileInfo *)catalogInfo.finderInfo)->fileCreator = i_creator;
      err = FSSetCatalogInfo( &fsref, kFSCatInfoFinderInfo, &catalogInfo );
    }
  }
  return err;
}


bool PL_FileSpec::MakeBundle()
{
  OSErr err = -1;
  Boolean isDirectory;
  FSRef ref;
  
  err = FSPathMakeRef(
                      (const UInt8*)GetUTF8Path().c_str(), &ref, &isDirectory);
  
  if(!isDirectory)
    err = -1;
  
  FSCatalogInfo catalogInfo;
  if(!err)
    err = FSGetCatalogInfo(&ref, kFSCatInfoFinderInfo, &catalogInfo, nil, nil, nil);
  
  if(!err)
  {
    FolderInfo *finderInfo = (FolderInfo *)catalogInfo.finderInfo;
    /* set the package flag */
    finderInfo->finderFlags |= kHasBundle;
    err = FSSetCatalogInfo( &ref, kFSCatInfoFinderInfo, &catalogInfo );
  }
  return err == noErr;
}

bool PL_FileSpec::UnmakeBundle()
{
  OSErr err = -1;
  Boolean isDirectory;
  FSRef ref;
  
  err = FSPathMakeRef(
                      (const UInt8*)GetUTF8Path().c_str(), &ref, &isDirectory);
  
  if(!isDirectory)
    err = -1;
  
  FSCatalogInfo catalogInfo;
  if(!err)
    err = FSGetCatalogInfo(&ref, kFSCatInfoFinderInfo, &catalogInfo, nil, nil, nil);
  
  if(!err)
  {
    FolderInfo *finderInfo = (FolderInfo* )catalogInfo.finderInfo;
    /* set the package flag */
    finderInfo->finderFlags &= ~kHasBundle;
    err = FSSetCatalogInfo( &ref, kFSCatInfoFinderInfo, &catalogInfo );
  }
  return err == noErr;
}

bool PL_FileSpec::IsBundle() const
{
  Boolean isDirectory;
  FSRef	fileRef;
  LSItemInfoRecord	fileInfo;

  if(FSPathMakeRef(
    (const UInt8*)GetUTF8Path().c_str(), &fileRef, &isDirectory) != noErr)
    return false;
  if(!isDirectory)/* only directory can be a package */
    return false;
  if(LSCopyItemInfoForRef(
    &fileRef, kLSRequestBasicFlagsOnly, &fileInfo) != noErr)
    return false;
  /* as long as we requested only flags -- we will not release any strings */
  return (fileInfo.flags & kLSItemInfoIsPackage) != 0;
}

#endif

#if 0
#pragma mark -
#pragma mark *** Navigation ***
#endif

// Up one level
int PL_FileSpec::Up()
{
  UT_String::size_type	pos = m_path.rfind( kPathSeparatorChar );
#if defined(TARGET_WIN32 ) || defined(TARGET_WEBGL)
  if ( pos == 0 || pos == 1 || pos == UT_String::npos )
    return -1;
#else
  if ( pos == 0 or pos == UT_String::npos )
    return -1;
#endif

  m_path.erase( pos );
  m_info.Clear();
#ifdef TARGET_WIN32
  if(m_path.length() == 2 && isalpha(m_path[0]) && m_path[1] == ':')
    m_path += kPathSeparatorChar;
#endif
  return 0;
}

// 
int PL_FileSpec::Add( const UT_String &i_name )
{
  if ( i_name.empty() || i_name.length() > 255 || (i_name.find_first_of( "?*:/\\")  != UT_String::npos) )
    return -1;

  if ( i_name.find( kPathSeparatorChar ) != UT_String::npos /* || !IsFolder() */ )
  {
    return -1;
  }

  if ( m_path[m_path.length()-1] != kPathSeparatorChar )
    m_path.append( 1, kPathSeparatorChar );
  m_path.append( i_name );
  m_info.Clear();
  return 0;
}

#if 0
#pragma mark -
#pragma mark *** Actions ***
#endif

// delete file or folder (recursively)
int PL_FileSpec::Delete() const
{
  if ( TestAccess() != 0 )
    return 0;

  int err = 0;
  if ( IsFolder() )
  {
    vector<PL_FileSpec>	dirlist;
    PL_FileSpec::iterator	it = begin();
    while ( it != end() )
    {
      dirlist.push_back( *it );
      ++it;
    }

    for ( vector<PL_FileSpec>::iterator it = dirlist.begin(); it != dirlist.end() && err == 0; ++it )
      err = it->Delete();

    if ( err == 0 )
    {
#if ( defined(TARGET_MAC) || defined(TARGET_IOS) || defined(TARGET_WEBGL))
      if ( rmdir( GetUTF8Path().c_str() ) == -1 )
        err = errno;
#elif _fl_posix
      if ( rmdir( UT_W2S( m_path ).c_str() ) == -1 )
        err = errno;
#elif _fl_win32
      if ( RemoveDirectory( m_path.c_str() ) == 0 )
        err = GetLastError();
#endif
    }
  }
  else
  {
#if ( defined(TARGET_MAC) || defined(TARGET_IOS) || defined(TARGET_WEBGL) )
    if ( unlink( GetUTF8Path().c_str() ) == -1 )
      err = errno;
#elif _fl_posix
    if ( unlink( UT_W2S( m_path ).c_str() ) == -1 )
      err = errno;
#elif _fl_win32
    if ( DeleteFile( m_path.c_str() ) == 0 )
      err = GetLastError();
#endif
  }
  m_info.Clear();
  return err;
}

// Create a folder.
int PL_FileSpec::CreateFolder() const
{
  // Already a folder!
  if( IsFolder() || IsFile()  )
  {
    return 0;
  }

  // Recursively create parent folders if necessary
  PL_FileSpec parentFolder(*this);
  if (parentFolder.Up() == 0)
  {
    int result = parentFolder.CreateFolder();
    if (result != 0)
      return result;
  }

  m_info.Clear();
#ifdef TARGET_MAC
  int err = 0;
  mode_t aMode = S_IRWXU | S_IRWXG | S_IRWXO;
  if ( mkdir( GetUTF8Path().c_str(), aMode ) == -1 )
    err = errno;
  return err;
#elif _fl_posix
  int err = 0;
  mode_t aMode = S_IRWXU | S_IRWXG | S_IRWXO;
  if ( mkdir( UT_W2S(  m_path ).c_str(), aMode ) == -1 )
    err = errno;
  return err;
#elif _fl_win32
  if ( CreateDirectory( m_path.c_str(), 0 ) )
    return 0;
  else
  {
    return GetLastError();
  }
#endif
}

#if !( defined( TARGET_IOS ) || defined( TARGET_MAC ) )
int PL_FileSpec::FileCreate( const UT_String &fileName ) const
{
  std::string splitter ("/");
  std::string dest( m_path.begin(), m_path.end() );
  if( strcmp( &dest[dest.size()-1], splitter.c_str() ) != 0 )
    dest += splitter;

  dest.append( fileName.begin(), fileName.end() );

  int fd = -1;
  
  std::ofstream file;
  file.open( dest.c_str() );
  file << "";
  file.close();

#ifdef TARGET_WIN32
  fd = open( dest.c_str(), O_WRONLY | _O_CREAT, _S_IREAD | _S_IWRITE );
#else
  fd = open( dest.c_str(), S_IWOTH );
#endif
  return fd;
}
#endif

// rename file or folder.
int PL_FileSpec::Rename( const UT_String &i_name ) const
{
  if ( !IsValid() || i_name.empty() || (i_name.find_first_of( "?*:/\\" ) != UT_String::npos) )
    return -1;
  m_info.Clear();
  PL_FileSpec	tmpSpec( *this );
  int err = tmpSpec.Up();
  if ( err == 0 )
    err = tmpSpec.Add( i_name );
  if ( err == 0 /*&& TestAccess() == 0 */ )
    if ( rename( GetUTF8Path().c_str(), tmpSpec.GetUTF8Path().c_str() ) != 0 )
      err = errno;
  return err;
}


int PL_FileSpec::Copy( const PL_FileSpec &i_dest, bool overwrite ) const
{
  if(i_dest.TestAccess() == 0)
  {
    if(overwrite == false)
      return -1;  // File already exists ... don't do anything
    if(i_dest.Delete() != 0)
      return -1;  // Could not remove destination
  }

#if defined(TARGET_MAC) || defined(TARGET_IOS) || defined(TARGET_WEBGL)
  int err = 0;
  if ( link(GetUTF8Path().c_str(), i_dest.GetUTF8Path().c_str()) != 0 )
    err = errno;
  return err;
#elif defined(TARGET_LINUX)
  int err = 0;
  if ( link(UT_W2S( m_path ).c_str(), UT_W2S(i_dest.GetFilePath()).c_str()) != 0 )
    err = errno;
  return err;
#elif defined(TARGET_WIN32)
  int err = 0;
  if ( CopyFile( m_path.c_str(), i_dest.m_path.c_str(), true ) != 0 )
    err = GetLastError();
  return err;
#endif
  
  return -1;
}

//	move a file or folder, FileSpec don't follow the file
int PL_FileSpec::Move( const PL_FileSpec &i_dest ) const
{
  m_info.Clear();
  int	err = 0;
  if ( rename( GetUTF8Path().c_str(), i_dest.GetUTF8Path().c_str() ) != 0 )
    err = errno;
  return err;
}


#if 0
#pragma mark -
#pragma mark *** File access ***
#endif

FILE *PL_FileSpec::get_fs( const char *mode ) const
{
  m_info.Clear();
#if defined(TARGET_MAC) || defined(TARGET_ANDROID) || defined(TARGET_WEBGL)
  return fopen( GetUTF8Path().c_str(), mode );
#elif _fl_posix
  return fopen( UT_W2S( m_path ).c_str(), mode );
#elif _fl_win32
  if ( !IsValid() )	//	prevent an ASSERT on windows.
    return 0;
  UT_String	wmode( mode );
  return _wfopen( m_path.c_str(), wmode.c_str() );
#endif
}

#if 0
#pragma mark -
#pragma mark *** Folder iteration ***
#endif

PL_FileSpec::iterator PL_FileSpec::begin()
{
  iterator	iter( *this );
  return iter;
}

PL_FileSpec::const_iterator PL_FileSpec::begin() const
{
  const_iterator	iter( *this );
  return iter;
}

PL_FileSpec::iterator PL_FileSpec::end()
{
  iterator	iter;
  return iter;
}

PL_FileSpec::const_iterator PL_FileSpec::end() const
{
  const_iterator	iter;
  return iter;
}

#if 0
#pragma mark -
#pragma mark *** Core Foundation API ***
#endif

#ifdef TU_MACOS // Do we still need that ?
PL_FileSpec::PL_FileSpec( const CFURLRef i_url )
{
  IFileSpec( i_url );
}

int PL_FileSpec::IFileSpec( const CFURLRef i_url )
{
  m_info.Clear();
  int	err = -1;
  FSRef	fsref;
  if ( CFURLGetFSRef( i_url, &fsref ) )
    return IFileSpec( fsref );
  else
  {
    CFURLRef parent = CFURLCreateCopyDeletingLastPathComponent( kCFAllocatorDefault, i_url );
    bool	res = CFURLGetFSRef( parent, &fsref );
    CFRelease( parent );
    if ( res )
    {
      err = IFileSpec( fsref );
      if ( err == 0 )
      {
        CFStringRef	fname = CFURLCopyLastPathComponent( i_url );
        err = Add( fname );
        CFRelease( fname );
      }
    }
  }
  if ( err != 0 )
    IFileSpec();
  return err;
}

CFURLRef PL_FileSpec::GetCFURL() const
{
  CFStringRef pathRef = CFStringCreateWithCharactersNoCopy( 0, m_path.data(), m_path.length(), kCFAllocatorNull );
  CFURLRef  urfRef = CFURLCreateWithFileSystemPath( 0, pathRef, kCFURLPOSIXPathStyle, false );
  CFRelease( pathRef );
  return urfRef;
}

CFStringRef PL_FileSpec::GetCFName() const
{
  UT_String	fname = GetName();
  if ( not fname.empty() )
    return fname.ToCFString();
  else
    return nil;
}

int PL_FileSpec::Add( const CFStringRef i_name )
{
  UT_String	fname( i_name );
  return Add( fname );
}

int PL_FileSpec::Rename( const CFStringRef i_name )
{
  UT_String	fname( i_name );
  return Rename( fname );
}

#endif

#if 0
#pragma mark -
#pragma mark *** Folder iterator ***
#endif

PL_FileSpec_iterator::PL_FileSpec_iterator( const PL_FileSpec &folderSpec )
{
#ifdef TARGET_MAC
  m_path = folderSpec.GetFilePath();
  if ( m_path[m_path.length()-1] != kPathSeparatorChar )
    m_path.append( 1, kPathSeparatorChar );
  m_dirp = ::opendir( folderSpec.GetUTF8Path().c_str() );
  if ( m_dirp != NULL )
  {
    UT_String	filePath;
    PL_FileInfo info;
    if ( readdir_priv( filePath, info ) )
      m_current.IFileSpec( filePath, info );
    else
    {
      ::closedir( m_dirp );
      m_dirp = NULL;
      m_current.IFileSpec();
    }
  }
#elif _fl_posix
  m_path = folderSpec.GetFilePath();
  if ( m_path[m_path.length()-1] != kPathSeparatorChar )
    m_path.append( 1, kPathSeparatorChar );
  m_dirp = ::opendir( UT_W2S(folderSpec.GetFilePath()).c_str() );
  if ( m_dirp != NULL )
  {
    UT_String	filePath;
    PL_FileInfo info;
    if ( readdir_priv( filePath, info ) )
      m_current.IFileSpec( filePath, info );
    else
    {
      ::closedir( m_dirp );
      m_dirp = NULL;
      m_current.IFileSpec();
    }
  }
#elif _fl_win32
  m_path = folderSpec.GetFilePath();
  if ( m_path[m_path.length()-1] != kPathSeparatorChar )
    m_path.append( 1, kPathSeparatorChar );
  UT_String	filePath;
  PL_FileInfo info;
  m_current.IFileSpec();
#endif
}

PL_FileSpec_iterator::PL_FileSpec_iterator()
{
#if _fl_posix
  m_dirp = NULL;
#elif _fl_win32
#endif
}

PL_FileSpec_iterator::PL_FileSpec_iterator( const PL_FileSpec_iterator &i_other )
{
#if _fl_posix
  m_dirp = NULL;
#elif _fl_win32
#endif

  *this = i_other;
}

PL_FileSpec_iterator::~PL_FileSpec_iterator()
{
#if _fl_posix
  if ( m_dirp != NULL )
    closedir( m_dirp );
#elif _fl_win32
#endif
}


PL_FileSpec_iterator &PL_FileSpec_iterator::operator=( const PL_FileSpec_iterator &i_other )
{
  if ( this != &i_other )
  {
#if _fl_posix
    m_current = i_other.m_current;
    m_path = i_other.m_path;
//    m_dirp = DuplicateDirPtr_priv( i_other.m_dirp );
    m_dirp = i_other.m_dirp;
    i_other.m_dirp = NULL;     // Ownership transfer, like auto_ptr
#elif _fl_win32
    m_current = i_other.m_current;
    m_path = i_other.m_path;
    UT_String	target = m_current.GetFilePath();
//    DuplicateHandle_priv( target );
#endif
  }
  return *this;
}


bool PL_FileSpec_iterator::operator==( const PL_FileSpec_iterator &i_other ) const
{
  bool isequal;
#if _fl_posix

  if ( (m_dirp == NULL) && (i_other.m_dirp == NULL) )
    isequal = true;
  else if ( (m_dirp == NULL) || (i_other.m_dirp == NULL) )
    isequal = false;
  else
  {
    isequal = ( m_current.IsValid() && !i_other.m_current.IsValid() );
    isequal &= ( m_current == i_other.m_current );
  }
#elif _fl_win32
  isequal = ( m_current.IsValid() && !i_other.m_current.IsValid() );
  isequal &= ( m_current == i_other.m_current );
#endif
  return isequal;
}

bool PL_FileSpec_iterator::operator!=( const PL_FileSpec_iterator &i_other ) const
{
  return !(*this == i_other);
}

PL_FileSpec_iterator &PL_FileSpec_iterator::operator++()
{
  Advance();
  return *this;
}

const PL_FileSpec_iterator &PL_FileSpec_iterator::operator++() const	
{
  Advance();
  return *this;
}

void PL_FileSpec_iterator::Advance() const
{
#if _fl_posix
  if ( m_dirp != NULL )
  {
    UT_String	filePath;
    PL_FileInfo info;
    if ( readdir_priv( filePath, info ) )
      m_current.IFileSpec( filePath, info );
    else
    {
      closedir( m_dirp );
      m_dirp = NULL;
      m_current.IFileSpec();
    }
  }
#elif _fl_win32
#endif
}

#if _fl_posix
#if 0
#pragma mark -
#pragma mark *** Posix helpers ***
#endif
bool PL_FileSpec_iterator::readdir_priv( UT_String &o_filePath, PL_FileInfo &o_info ) const
{
  struct dirent	*direntry = readdir( m_dirp );
  o_filePath.erase();
  if ( direntry != NULL )
  {
    while ( strcmp( direntry->d_name, "." ) == 0 ||
      strcmp( direntry->d_name, ".." ) == 0 )
    {
      direntry = readdir( m_dirp );
      if ( direntry == NULL )
        break;
    }
    if ( direntry != NULL )
    {
      o_filePath.assign( m_path );
      UT_String us(  direntry->d_name );
      o_filePath.append( us);
    }
  }

  return (direntry != NULL);
}

#endif

#if 0
#pragma mark -
#pragma mark *** Win32 helpers ***
#endif


std::string PL_FileSpec::GetUTF8Path() const
{
#ifdef TU_MACOS //  Do we still need that ?
  string  res;
  CFURLRef	urlRef = GetCFURL();
  if ( urlRef != 0 )
  {
    UInt8   buffer[2048];
    if ( ::CFURLGetFileSystemRepresentation( urlRef, true, buffer, 2048 ) )
      res.assign( (const char *)buffer );
    ::CFRelease( urlRef );
  }
  return res;
#else
  return m_path.utf8();
#endif
}

