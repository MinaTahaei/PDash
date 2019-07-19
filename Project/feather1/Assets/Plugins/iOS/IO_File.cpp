#include "IO_File.h"
#include "PL_FileSpec.h"

#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - IO_File::Impl
#endif
class IO_File::Impl
{
  MEM_OVERRIDE

  friend class IO_File;

public:

  Impl() :
    _file(0)
  {
  }

  ~Impl()
  {
  }

private:

  FILE        *_file;
  PL_FileSpec  _fileSpec;

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - IO_File::Impl
#endif

IO_File::IO_File()
{
  _i = new Impl;
}

IO_File::~IO_File()
{
  if (_i->_file)
    close();

  delete _i;
}


void IO_File::close( )
{
  fclose( _i->_file );
	_i->_file = 0;
}

bool IO_File::open( const PL_FileSpec &filename, bool bInput )
{
	const char *pOpenArg = bInput ? "rb" : "w+b";
	_i->_file = filename.get_fs( pOpenArg );
	if( _i->_file == 0 )
		return false;

	return true;
}


bool IO_File::openForInput( const PL_FileSpec &filename )
{
	if ( open( filename, true ) )
	{
		_i->_fileSpec = filename;
		return true;
	}
	else
		return false;
}


bool IO_File::createForOutput( const PL_FileSpec &filename )
{
	//PL_FileUtil::BackupFile(filename);
	if ( open( filename, false ) )
	{
		_i->_fileSpec = filename;
		return true;
	}
	else
		return false;
}


size_t IO_File::read(void* lpBuf, size_t nCount)
{
	size_t nRes = fread( lpBuf, 1, nCount, _i->_file );
  return nRes;

	// Expects either 0 (if reading the end of file )
	if( ( nRes != nCount ))
	{
    /*
		QString ws;
		if( feof( m_pFile ))
		{
			ws = qApp->translate("PersistentObj", "Reading past end of file");
		}
		else
		{
			ws = qApp->translate("PersistentObj", "Error reading from file"); 
		}
		throw DB_Exception( ws );
    */
	}
}

void IO_File::write(const void* lpBuf, size_t nCount)
{
	size_t nRes = fwrite( lpBuf, 1, nCount, _i->_file );
	if( nRes != nCount )
	{
    /*
		throw DB_Exception(qApp->translate("PersistentObj", "Unable to write to file" ));
    */
	}
}

void IO_File::reset()
{
	if( _i->_file )
	{
		rewind( _i->_file );
	}
}

bool IO_File::isEOF()
{
	return feof(_i->_file);
}

size_t IO_File::size() const
{
  return _i->_fileSpec.GetFileLength();
}

UT_String IO_File::streamName() const
{
	return _i->_fileSpec.GetName();
}

PL_FileSpec IO_File::streamSpec() const
{
	return _i->_fileSpec;
}

