
/*
 * Copyright Toon Boom Animation Inc. 
 * This file is part of the Toon Boom Animation SDK
 */


#ifndef CORE__H_SFILESPEC
#define	CORE__H_SFILESPEC


#include "UT_String.h"
#include "PL_FileInfo.h"
#include "PL_Configure.h"

#include "MEM_Override.h"

#if defined(TARGET_WIN32)
#define _fl_win32	1
#define _fl_posix	0
#else
#define _fl_win32	0
#define _fl_posix	1
#endif

//	sanity check
#if (_fl_posix&&_fl_win32)||(!(_fl_posix||_fl_win32))
#error "Choose one and only one file location management type"
#endif

// #include <fstream>
#include <iterator>
#include <time.h>
#include <stdio.h>

#if _fl_posix
#include <sys/types.h>
#include <dirent.h>
#endif

// #ifdef TU_MAC
// #include <ApplicationServices/ApplicationServices.h>
// #endif

#ifdef TARGET_IOS
#include <CoreFoundation/CoreFoundation.h>
#endif

class UT_RelPath;
class UT_String;

#if defined(TARGET_WIN32)
struct FSSpec;
#endif
class PL_FileSpec_iterator;

/*!
 * Class that encapsulate the access to the file system.
 * On windows, it will resolve the shortcuts
 */
class   PL_FileSpec
{
  MEM_OVERRIDE

  friend class PL_FileSpec_iterator;

public:

  ~PL_FileSpec();	

  /*
   * empty init.
   * the PL_FileSpec will be invalid, all operation on it will fail
   */
  PL_FileSpec();

  /*!
   * empty init.
   * the PL_FileSpec will be invalid, all operation on it will fail
   */  
  void IFileSpec();

  /*!
   * copy constructor.
   *filespec will point to the same location of source
   *or be invalid if source is invalid
   */
  PL_FileSpec( const PL_FileSpec &i_other );
  
  /*!
   * Same as operator=
   */
  int IFileSpec( const PL_FileSpec &i_other );

  /*!
   * Copies from i_other
   */
  PL_FileSpec	&operator=( const PL_FileSpec &i_other );

  /*!
   * Enum used for Special init.
   */
  enum SpecialFolderType
  {
    kTempFile = 1,			/*!<	filespec will point to a temporary (uncreated) file */
    kUserDocumentsFolder,	//	filespec will point to the user document folder (or home if that concept don't exist)
    kUserAppDataFolder
    // kSessionTempFolder
  };

  /*!
   * Init with a special folder location of file type
   */
  int		IFileSpec( SpecialFolderType i_code );

  // Init with a relative path.
  //	
  int		IFileSpec( const UT_String &i_relative, const PL_FileSpec &i_absolute );

  // Get a relative representation portable across platform.
  int		GetRelativePath( UT_String &o_relative, const PL_FileSpec &i_absolute ) const;

  explicit PL_FileSpec(const UT_String & i_path);
  explicit PL_FileSpec(const UT_RelPath& i_path);
  int IFileSpec( const UT_String &i_path );

  PL_FileSpec( const UT_String &i_path, const PL_FileInfo &info );
  int IFileSpec( const UT_String &i_path, const PL_FileInfo &info );

#if defined(TARGET_IOS)
  PL_FileSpec( const FSRef &i_ref );
  PL_FileSpec( const FSRef &i_parentRef, const UT_String &i_name );
  int IFileSpec( const FSRef &i_ref );
  int IFileSpec( const FSRef &i_parentRef, const UT_String &i_name );
  int GetFSRef( FSRef &o_ref ) const;
  int GetFSRef( FSRef &o_parentRef, UT_String &o_name ) const;
    
  int GetTypeAndCreator( OSType &o_type, OSType &o_creator ) const;
  int SetTypeAndCreator( OSType i_type, OSType i_creator );
  bool MakeBundle();
  bool UnmakeBundle();
  bool IsBundle() const;

  PL_FileSpec( const CFURLRef i_url );
  int IFileSpec( const CFURLRef i_url );
  CFURLRef GetCFURL() const;
  CFStringRef GetCFName() const;
  int Add( const CFStringRef i_name );
  int Rename( const CFStringRef i_name );
#endif

  static UT_String UserDocumentDirectory();
  
  //	Get the complete file path (for information only on Mac OS ---- and IOS )
  UT_String	GetFilePath() const;
  std::string GetUTF8Path() const;

  //	Get the file name
  UT_String	GetName( bool i_withExtension = true ) const;

  //	extension management
  UT_String	&GetExtension( UT_String &o_ext ) const;
  UT_String	GetExtension() const;
  bool			HasExtension( const UT_String &i_ext ) const;
  void			SetExtension( const UT_String &i_ext );
  void			AddExtension( const UT_String &i_ext );

  //! Is this a file?
  bool	IsFile() const;

  //! Is this a symbolic link (only in Posix environment );
  bool IsSymbolicLink() const;

  //! Is this a folder?
  bool	IsFolder() const;

  //! Is this a share  (e.g. //machine/share)
  bool IsShare() const;

  //! Is this file or folder spec valid?
  bool	IsValid() const;


  //! Is this file or folder exist?
  int		TestAccess(bool bReadWrite = false) const;

  //! Return file length
  size_t GetFileLength() const;

  //!	Get the creation and modification time of the file
  int	GetFileTime( time_t &o_lastmodif ) const;

  int SetFileTime( const std::string &time );

  //!	down one level
  int		Add( const UT_String &i_name );

  //! Up one level
  int		Up();

  //! refer to same file.
  bool		operator == ( const PL_FileSpec &i_other ) const;
  inline bool	operator != ( const PL_FileSpec &i_other ) const	{	return !(*this == i_other);	}

  //! for stl compare
  bool operator<(const PL_FileSpec& i_file) const;

  //! delete file or folder
  //!	filespec is still valid and point to the deleted item
  int			Delete() const;

  // Create a folder.
  //	file spec don't change
  int			CreateFolder() const;

  // Create a file.
  // file spec don't change
  int    FileCreate( const UT_String &fileName ) const;

  // move file or folder
  //	file spec don't change
  int			Move( const PL_FileSpec &i_dest ) const;

  // rename an existing file
  //	filespec point to the new file or folder
  int			Rename( const UT_String &i_name ) const;

  // copy to file
  int     Copy( const PL_FileSpec &i_desc, bool overwrite = false ) const;


  FILE	*get_fs( const char *mode ) const;

  // folder iteration
  typedef PL_FileSpec_iterator			iterator;
  typedef const PL_FileSpec_iterator	const_iterator;

  iterator		begin();
  const_iterator	begin() const;
  iterator		end();
  const_iterator	end() const;


private:

  int	InitTempFile();
  int	InitSessionTempFolder();
  int	InitUserDocumentsFolder();
  int InitUserAppDataFolder();

  UT_String	m_path;
  mutable PL_FileInfo  m_info;
};

class   PL_FileSpec_iterator
{
  MEM_OVERRIDE

  friend class PL_FileSpec;

private:
  mutable PL_FileSpec	m_current;

  UT_String	m_path; // root path

#if _fl_posix
  mutable DIR	*m_dirp;
  bool	readdir_priv( UT_String &o_filePath, PL_FileInfo &o_info ) const;
#endif

  void Advance()const;

public:
  PL_FileSpec_iterator( const PL_FileSpec &folderSpec );
  PL_FileSpec_iterator();
  ~PL_FileSpec_iterator();

  PL_FileSpec_iterator( const PL_FileSpec_iterator &i_other );
  PL_FileSpec_iterator	&operator = ( const PL_FileSpec_iterator &i_other );

  bool							operator == ( const PL_FileSpec_iterator &i_other ) const;
  bool							operator != ( const PL_FileSpec_iterator &i_other ) const;
  inline const PL_FileSpec		&operator * () const{	return m_current;	}
  inline const PL_FileSpec		*operator -> () const{	return &m_current;	}
  PL_FileSpec_iterator			&operator ++ ();

  const PL_FileSpec_iterator			&operator ++ ()const;

};

#endif	// _H_SFILESPEC
