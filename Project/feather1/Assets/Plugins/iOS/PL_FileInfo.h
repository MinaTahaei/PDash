
/*
 * Copyright Toon Boom Animation Inc. 
 * This file is part of the Toon Boom Animation SDK
 */


#ifndef CORE_PL_FILEINFO_H
#define CORE_PL_FILEINFO_H

#include <time.h> 

#if defined(TARGET_WIN32)
#define S_IFLNK  0200000
#endif

#include "MEM_Override.h"
#include "PL_Configure.h"

class UT_Path;
class UT_String;

/*!
 * Provides a portable way of handling file information. This class
 * has a couple of benefits over QFileInfo. It is faster, provide extended settings
 * and can be partially constructured when iterating a directory.
 *
 * The actual file name is not kept in this structure. It is assume that this 
 * class will be used conjointly with the PL_FileSpec or UT_Path.
 */
class   PL_FileInfo
{
  MEM_OVERRIDE

public:
  PL_FileInfo(); // not a file, not a folder.
  PL_FileInfo( const UT_String & pathName );
  PL_FileInfo( const char* &pathName );


  // Constructor to be used when iterating over files in a folder.  This avoid stating the file
  // a second time.
  PL_FileInfo(time_t modif, unsigned mode, unsigned fileLength);


  void Clear();

  bool IsInit() const;    //!< true when PL_FileInfo was initialised with a path.
  bool IsError() const;

  bool IsFile() const;    //!< true when this path name is a valid file.
  bool IsFolder() const;  //!< true when this pathname is a valid folder/directory
  bool IsExists() const;  //!< true when pathname exists (is either a file or a folder).

  bool IsSymbolicLink() const; //!< true if this file is a symblink. Always false on Windows.

  bool CanRead() const;   //!< true when can read in this folder/file.
  bool CanWrite() const;  //!< true when can write to this folder/file.

  time_t GetModifTime() const; //!< return the modification time of the file.
  unsigned GetFileLength() const; //!< return the file length.

private:
  unsigned m_mode;    /*!< copied from struct stat.st_mode. Contains all flags. 
                         0->not initialised, 0xffffffff-> failed access the file. */
  time_t m_modifTime;  //!< modification time of the file.
  unsigned m_fileLength; 

  //! Indicate if file is hidden. This is 3 state bool (0=> not init, 1=>not hidden, 2=>hidden)
  mutable unsigned char m_hidden;

  bool init(const char*pathName);
};

#endif

