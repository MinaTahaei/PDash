#ifndef _IO_FILE_H_
#define _IO_FILE_H_

#include "IO_Stream.h"
#include "MEM_Override.h"

class PL_FileSpec;

/*!
 *  @class IO_File
 *  Stream onto a file.
 */
class IO_File : public IO_Stream
{
  MEM_OVERRIDE

public:

  IO_File();
  virtual ~IO_File();

  // File interface
  void                 close();

  // Replace filename by the 'encapsulated' filename.
  bool                 openForInput( const PL_FileSpec &filename );
  bool                 createForOutput( const PL_FileSpec &filename );

  // IO_Stream interface
  virtual size_t       read(void* lpBuf, size_t nCount);
  virtual void         write(const void* lpBuf, size_t nCount);
  virtual void         reset();
  virtual bool	       isEOF();
  virtual size_t       size() const;

  virtual UT_String    streamName() const;
  PL_FileSpec          streamSpec() const;

protected:

  // Utility function used to open for input or output
  bool                 open(const PL_FileSpec &filename, bool bInput);

private:

  //  non-implemented constructors.
  IO_File( const IO_File& );
  IO_File &operator= ( const IO_File& );

private:

  class Impl;
  Impl *_i;
};

#endif // _IO_FILE_H_
