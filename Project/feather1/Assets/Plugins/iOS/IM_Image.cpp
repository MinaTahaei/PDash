#include "IM_Image.h"

IM_Image::IM_Image( unsigned width, unsigned height, unsigned depth ) :
  _width(width),
  _height(height),
  _depth(depth)
{
  _buffer = new Channel_t[ _width * _height * _depth ];
}

IM_Image::~IM_Image()
{
  delete [] _buffer;
}

IM_Image::Channel_t *IM_Image::data()
{
  return _buffer;
}

IM_Image::Channel_t *IM_Image::pixel( unsigned x, unsigned y )
{
  return &_buffer[ ((_width * y) + x) * _depth  ];
}

unsigned IM_Image::width() const
{
  return _width;
}

unsigned IM_Image::height() const
{
  return _height;
}

unsigned IM_Image::depth() const
{
  return _depth;
}
