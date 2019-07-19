#include "IO_PersistentStore.h"
#include "IO_Stream.h"
#include "STD_Types.h"

//  We don't have anything else than little endian in our implementations,
//  if needs be, reimplement functions for big endian.

//	Little endian to native format endian
#define Byte16LittleToNative(x) (x)
#define Byte32LittleToNative(x) (x)
#define Byte64LittleToNative(x) (x)

//	Native format endian to little endian
#define Byte16NativeToLittle(x) (x)
#define Byte32NativeToLittle(x) (x)
#define Byte64NativeToLittle(x) (x)

namespace
{
  template<typename T>
  void readData( IO_Stream *stream, T *data )
  {
    stream->read( data, sizeof(T) );
  }

  template <typename T>
  void writeData( IO_Stream *stream, T *data )
  {
    stream->write( data, sizeof(T) );
  }

  template <typename T>
  void writeData( IO_Stream *stream, T data )
  {
    stream->write( &data, sizeof(T) );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - IO_PersistentStore::Impl
#endif

class IO_PersistentStore::Impl
{
  MEM_OVERRIDE

  friend class IO_PersistentStore;

public:

  Impl()
  {
  }

  ~Impl()
  {
  }

private:

  IO_Stream *_stream;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - IO_PersistentStore
#endif

IO_PersistentStore::IO_PersistentStore( const IO_PersistentStore &store )
{
  _i = new Impl;
  _i->_stream = store._i->_stream;
}

IO_PersistentStore::IO_PersistentStore( IO_Stream &stream )
{
  _i = new Impl;
  _i->_stream = &stream;
}

IO_PersistentStore::~IO_PersistentStore()
{
  delete _i;
}

IO_PersistentStore& IO_PersistentStore::operator>>( bool& i )
{
  char	value;	//	char is always 1 byte (not bool)
  readData( _i->_stream, &value );
  i = (value != 0);
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator>>( char& i )
{
  readData( _i->_stream, &i );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator>>( unsigned char& i )
{
  readData( _i->_stream, &i );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator>>( short& i )
{
  readData( _i->_stream, &i );
  i = Byte16LittleToNative( i );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator>>( unsigned short& i )
{
  readData( _i->_stream, &i );
  i = Byte16LittleToNative( i );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator>>( int& i )
{
  readData( _i->_stream, &i );
  i = Byte32LittleToNative( i );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator>>( int64_t& i )
{
  readData( _i->_stream, &i );
  i = Byte64LittleToNative( i );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator>>( uint64_t& i )
{
  readData( _i->_stream, &i );
  i = Byte64LittleToNative( i );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator>>( unsigned int& i )
{
  readData( _i->_stream, &i );
  i = Byte32LittleToNative( i );
  return *this;
}


IO_PersistentStore& IO_PersistentStore::operator>>( float& i )
{
  readData( _i->_stream, &i );
  unsigned int *p = (unsigned int *)&i;
  *p = Byte32LittleToNative( *p );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator>>( double& i )
{
  readData( _i->_stream, &i );
  int64_t *p = (int64_t *)&i;
  *p = Byte64LittleToNative( *p );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator>>( STD_String &s )
{
  unsigned int size;
  readData( _i->_stream, &size );
  size = Byte32LittleToNative( size );
  if( size != 0 )
  {
    s.resize( size );
    loadBuffer( (unsigned char*)&*s.begin(), size );
  }
  return *this;
}

size_t IO_PersistentStore::loadBuffer( unsigned char* buffer, size_t size )
{
  return _i->_stream->read( buffer, size );
}

IO_PersistentStore& IO_PersistentStore::operator<<( const bool  i )
{
  char	value = i;	//	char is always 1 byte (not bool)
  writeData( _i->_stream, &value );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator<<( const char  i )
{
  writeData( _i->_stream, &i );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator<<( const unsigned char  i )
{
  writeData( _i->_stream, &i );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator<<( const short i )
{
  writeData( _i->_stream, Byte16NativeToLittle( i ) );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator<<( const unsigned short i )
{
  writeData( _i->_stream, Byte16NativeToLittle( i ) );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator<<( const int i )
{
  writeData( _i->_stream, Byte32NativeToLittle( i ) );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator<<( const unsigned int i )
{
  writeData( _i->_stream, Byte32NativeToLittle( i ) );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator<<( const int64_t i )
{
  writeData( _i->_stream, Byte64NativeToLittle( i ) );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator<<( const uint64_t i )
{
  writeData( _i->_stream, Byte64NativeToLittle( i ) );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator<<( const float i )
{
  unsigned int	*p = (unsigned int *) &i;
  writeData( _i->_stream, Byte32NativeToLittle( *p ) );
  return *this;
}

IO_PersistentStore& IO_PersistentStore::operator<<( const double i )
{
  int64_t	*p = (int64_t *) &i;
  writeData( _i->_stream, Byte64NativeToLittle( *p ) );
  return *this;
}


IO_PersistentStore& IO_PersistentStore::operator<<( const STD_String &s )
{
  size_t len = s.length();
  unsigned int	tmp = Byte32NativeToLittle( (unsigned int)len );
  writeData( _i->_stream, &tmp );
  storeBuffer( (unsigned char *)&*s.begin(), len );
  return *this;
}

void IO_PersistentStore::storeBuffer( const unsigned char* buffer, size_t size )
{
  _i->_stream->write( buffer, size );
}

const IO_Stream *IO_PersistentStore::stream() const
{
  return _i->_stream;
}
