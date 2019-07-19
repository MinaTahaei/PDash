#ifndef _IO_PERSISTENT_STORE_H_
#define _IO_PERSISTENT_STORE_H_

#include <stdlib.h>
#include <stdint.h>
#include <string>

#include "MEM_Override.h"
#include "STD_Types.h"

class IO_Stream;

/*!
 *  @class IO_PersistentStore
 *  Interface for writing/reading data to stream.
 */
class IO_PersistentStore
{
  MEM_OVERRIDE

public:

  explicit IO_PersistentStore( const IO_PersistentStore& store );
  explicit IO_PersistentStore( IO_Stream &stream );

  ~IO_PersistentStore();

  // Read data from a binary database file.
  IO_PersistentStore& operator>>( bool& i );
  IO_PersistentStore& operator>>( char& i );
  IO_PersistentStore& operator>>( unsigned char& i);
  IO_PersistentStore& operator>>( short& i );
  IO_PersistentStore& operator>>( unsigned short &i );
  IO_PersistentStore& operator>>( int& i );
  IO_PersistentStore& operator>>( unsigned int& i );
  IO_PersistentStore& operator>>( int64_t& i );
  IO_PersistentStore& operator>>( uint64_t& i );
  IO_PersistentStore& operator>>( float& i );
  IO_PersistentStore& operator>>( double& i );
  IO_PersistentStore& operator>>( STD_String &s );

  size_t loadBuffer( unsigned char* buffer, size_t size );

  // Write data from to a binary database file.
  IO_PersistentStore& operator<<( const bool i );
  IO_PersistentStore& operator<<( const char i );
  IO_PersistentStore& operator<<( const unsigned char i);
  IO_PersistentStore& operator<<( const short i );
  IO_PersistentStore& operator<<( const unsigned short i );
  IO_PersistentStore& operator<<( const int i );
  IO_PersistentStore& operator<<( const unsigned int i );
  IO_PersistentStore& operator<<( const int64_t i );
  IO_PersistentStore& operator<<( const uint64_t i );
  IO_PersistentStore& operator<<( const float i );
  IO_PersistentStore& operator<<( const double i );
  IO_PersistentStore& operator<<( const STD_String &s );

  void storeBuffer( const unsigned char* buffer, size_t size );

  const IO_Stream *stream() const;

private:

  class Impl;
  Impl *_i;

};

#endif /* _IO_PERSISTENT_STORE_H_ */
