#ifndef _IM_IMAGE_H_
#define _IM_IMAGE_H_

#include "UT_ShareBase.h"
#include "UT_SharedPtr.h"

class IM_Image;
typedef UT_SharedPtr<IM_Image> IM_ImagePtr_t;

/*!
 *  @class IM_Image
 *  image container for 32 bits rgba image
 */
class IM_Image : public UT_ShareBase
{
public:

  typedef unsigned char Channel_t;

public:

  IM_Image( unsigned width, unsigned height, unsigned depth );
  ~IM_Image();

  Channel_t *data();
  Channel_t *pixel( unsigned x, unsigned y );

  unsigned width() const;
  unsigned height() const;
  unsigned depth() const;

private:

  unsigned   _width, _height, _depth;
  Channel_t *_buffer;
};

#endif /* _IM_IMAGE_H_ */
