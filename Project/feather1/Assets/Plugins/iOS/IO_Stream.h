#ifndef _IO_STREAM_H_
#define _IO_STREAM_H_

#include <stdlib.h>

#include "MEM_Override.h"

class UT_String;

/*!
 *  @class IO_Stream
 *  Base streaming interface.
 */
class IO_Stream
{
  MEM_OVERRIDE

public:
  virtual ~IO_Stream();

  virtual size_t read(void* lpBuf, size_t nCount) = 0;
  virtual void write(const void* lpBuf, size_t nCount) = 0;

  virtual UT_String streamName() const = 0;

  virtual bool isEOF() = 0;

  virtual void reset() = 0;

  virtual size_t size() const = 0;
};

#endif /* _IO_STREAM_H_ */
