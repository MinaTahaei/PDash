
#include "PL_FileInfo.h"
#include "UT_String.h"
// #include <UT_Path.h>

#include <iostream>
#include <string>
#include <sys/stat.h>

//#define _fl_win32	0
//#define _fl_posix	1

#if defined(TARGET_WEBGL)
	#define _fl_win32	1
	#define _fl_posix	0
#else
	#if defined(TARGET_WIN32)
		#define _fl_win32	1
		#define _fl_posix	0
		#undef UNICODE
		#define S_IRUSR S_IREAD
		#define S_IWUSR S_IWRITE
	#else
	  #define _fl_win32	0
	  #define _fl_posix	1
	#endif
#endif






// 0 is the default value for unititalised test. 0xffffffff is used to indicate that we failed to 'stat' the file 
// attribute.
#define INIT_MODE 0  
#define ERROR_MODE 0xffffffff

bool PL_FileInfo::init(const char* pathName)
{
  struct stat s;
  if( stat( pathName, &s ) == -1 )
  {
    m_mode = ERROR_MODE;
    return false;
  }
  m_mode = s.st_mode;
  m_modifTime = s.st_mtime;
  m_fileLength = (unsigned int)s.st_size;
  return true;
}

PL_FileInfo::PL_FileInfo()
: m_mode(0), m_modifTime(0), m_fileLength(0), m_hidden(0)
{
}


PL_FileInfo::PL_FileInfo( const UT_String & pathName )
: m_mode(0), m_modifTime(0), m_fileLength(0), m_hidden(0)
{
  init(pathName.utf8().c_str());
}




  // Constructor to be used when iterating over files in a folder.  This avoid stating the file
  // a second time.
PL_FileInfo::PL_FileInfo(time_t modif, unsigned mode, unsigned fileLength) :
  m_mode      (mode),
  m_modifTime (modif),
  m_fileLength(fileLength),
  m_hidden    (0)
{
}


void PL_FileInfo::Clear()
{
  m_mode = 0;
}
bool PL_FileInfo::IsInit() const
{
  return m_mode != 0;
}

bool PL_FileInfo::IsError() const
{
  return m_mode == ERROR_MODE;
}


bool PL_FileInfo::IsFile() const
{
  return ( m_mode != ERROR_MODE && m_mode & S_IFREG );
}

bool PL_FileInfo::IsFolder() const
{
  return ( m_mode != ERROR_MODE &&  m_mode & S_IFDIR );
}

bool PL_FileInfo::IsExists() const
{
  return ( m_mode != ERROR_MODE && m_mode & ( S_IFDIR | S_IFREG ));
}

bool PL_FileInfo::IsSymbolicLink() const
{
#if _fl_win32
  return ( m_mode != ERROR_MODE &&  m_mode & S_IFLNK );
#elif _fl_posix
  return ( m_mode != ERROR_MODE && S_ISLNK( m_mode ) );
#endif

}

bool PL_FileInfo::CanRead() const
{
  return ( m_mode != ERROR_MODE &&  m_mode & S_IRUSR );
}

bool PL_FileInfo::CanWrite() const
{
  return ( m_mode != ERROR_MODE && m_mode & S_IWUSR );
}

time_t PL_FileInfo::GetModifTime() const
{
  if( m_mode != ERROR_MODE )
    return m_modifTime;
  else
    return 0; 
}

unsigned PL_FileInfo::GetFileLength() const
{
  if( m_mode != ERROR_MODE )
    return m_fileLength;
  else
    return 0; 
}

