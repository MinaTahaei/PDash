#include "IM_PngIOHandler.h"
#include "PL_Configure.h"

#if defined(TARGET_DESKTOP) || defined(TARGET_ANDROID)

#include "IO_PersistentStore.h"

#include <png.h>
#include <pngconf.h>

#include <string.h>

namespace
{
  void read_fn_priv(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead)
  {
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    if (io_ptr == NULL)
      return;

    IO_PersistentStore &store = *(IO_PersistentStore*)io_ptr;
    store.loadBuffer( (unsigned char*)outBytes, (size_t)byteCountToRead );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - IM_PngIOHandler
#endif

IM_PngIOHandler::IM_PngIOHandler()
{
}

IM_PngIOHandler::~IM_PngIOHandler()
{
}

IM_ImagePtr_t IM_PngIOHandler::read(const IM_ImageIOContext &context, IO_PersistentStore &store) const
{
  png_structp png_ptr = 0;
  png_infop   info_ptr = 0;
  png_infop   end_info = 0;
  png_bytepp  row_ptr = 0;

  png_uint_32 width, height;
  png_byte depth;
  png_byte colType;

  png_ptr =
    png_create_read_struct( PNG_LIBPNG_VER_STRING,
                            (png_voidp)0,
                            (png_error_ptr)0,
                            (png_error_ptr)0 );
  if ( !png_ptr )
    return IM_ImagePtr_t(0);

  info_ptr = png_create_info_struct( png_ptr );
  if ( !info_ptr )
  {
    png_destroy_read_struct( &png_ptr, 0, 0 );
    return IM_ImagePtr_t(0);
  }

  end_info = png_create_info_struct( png_ptr );
  if ( !end_info )
  {
    png_destroy_read_struct( &png_ptr, &info_ptr, 0 );
    return IM_ImagePtr_t(0);
  }

  if ( setjmp( png_jmpbuf( png_ptr ) ) )
  {
    png_destroy_read_struct( &png_ptr, &info_ptr, &end_info );
    return IM_ImagePtr_t(0);
  }

  //  Read header and png data.
  //png_init_io( png_ptr, input );
  png_set_read_fn(png_ptr, &store, read_fn_priv);
  png_read_png( png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0 );

  //  Retrieve image info from data.
  width = png_get_image_width( png_ptr, info_ptr );
  height = png_get_image_height( png_ptr, info_ptr );
  depth = png_get_bit_depth( png_ptr, info_ptr );
  colType = png_get_color_type( png_ptr, info_ptr );

  row_ptr = png_get_rows( png_ptr, info_ptr );

  IM_ImagePtr_t pImage;

  switch(colType)
  {
  case PNG_COLOR_TYPE_GRAY:
    {
      pImage = new IM_Image(width, height, 3);

      for( unsigned y=0; y<height; ++y )
      {
        for( unsigned x=0; x<width; ++x )
        {
          IM_Image::Channel_t v = row_ptr[ y ][ x ];

          IM_Image::Channel_t *pixel = pImage->pixel(x, y);
          pixel[0] = v;
          pixel[1] = v;
          pixel[2] = v;
        }
      }
    }
    break;
  case PNG_COLOR_TYPE_GRAY_ALPHA:
    {
      pImage = new IM_Image(width, height, 4);

      if ( context._premultiplyAlpha )
      {
        for( unsigned y=0; y<height; ++y )
        {
          for( unsigned x=0; x<width; ++x )
          {
            IM_Image::Channel_t v = row_ptr[ y ][ (x<<1) ];
            IM_Image::Channel_t a = row_ptr[ y ][ (x<<1) + 1 ];

            IM_Image::Channel_t *pixel = pImage->pixel(x, y);
            pixel[0] = (IM_Image::Channel_t)(v * a / 255);
            pixel[1] = (IM_Image::Channel_t)(v * a / 255);
            pixel[2] = (IM_Image::Channel_t)(v * a / 255);
            pixel[3] = a;
          }
        }
      }
      else
      {
        for( unsigned y=0; y<height; ++y )
        {
          for( unsigned x=0; x<width; ++x )
          {
            IM_Image::Channel_t v = row_ptr[ y ][ (x<<1) ];
            IM_Image::Channel_t a = row_ptr[ y ][ (x<<1) + 1 ];

            IM_Image::Channel_t *pixel = pImage->pixel(x, y);
            pixel[0] = v;
            pixel[1] = v;
            pixel[2] = v;
            pixel[3] = a;
          }
        }
      }
    }
    break;
  case PNG_COLOR_TYPE_RGB:
    {
      pImage = new IM_Image(width, height, 3);

      for( unsigned y=0; y<height; ++y )
      {
        for( unsigned x=0; x<width; ++x )
        {
          IM_Image::Channel_t r = row_ptr[ y ][ (x*3) ];
          IM_Image::Channel_t g = row_ptr[ y ][ (x*3) + 1 ];
          IM_Image::Channel_t b = row_ptr[ y ][ (x*3) + 2 ];

          IM_Image::Channel_t *pixel = pImage->pixel(x, y);
          pixel[0] = r;
          pixel[1] = g;
          pixel[2] = b;
        }
      }
    }
    break;
  case PNG_COLOR_TYPE_RGB_ALPHA:
    {
      pImage = new IM_Image(width, height, 4);

      // Just dump it in structure
      if ( context._premultiplyAlpha )
      {
        for( unsigned y=0; y<height; ++y )
        {
          for( unsigned x=0; x<width; ++x )
          {
            IM_Image::Channel_t r = row_ptr[ y ][ (x<<2) ];
            IM_Image::Channel_t g = row_ptr[ y ][ (x<<2) + 1 ];
            IM_Image::Channel_t b = row_ptr[ y ][ (x<<2) + 2 ];
            IM_Image::Channel_t a = row_ptr[ y ][ (x<<2) + 3 ];

            IM_Image::Channel_t *pixel = pImage->pixel(x, y);
            pixel[0] = (IM_Image::Channel_t)(r * a / 255);
            pixel[1] = (IM_Image::Channel_t)(g * a / 255);
            pixel[2] = (IM_Image::Channel_t)(b * a / 255);
            pixel[3] = a;
          }
        }
      }
      else
      {
        size_t lineSize = 4 * width;
        for( unsigned y=0; y<height; ++y )
        {
          memcpy( pImage->pixel(0, y), row_ptr[ y ], lineSize );
        }
      }
    }
    break;
  }

  png_destroy_read_struct( &png_ptr, &info_ptr, &end_info );
  return pImage;
}

void IM_PngIOHandler::extensions(StringCol_t &extensions) const
{
  extensions.push_back( "png" );
  extensions.push_back( "PNG" );
}

REGISTER_IMAGE_HANDLER(IM_PngIOHandler);

#elif defined(TARGET_WEBGL)

#include "IO_PersistentStore.h"

#include "lodePng/lodepng.h"
#include "IO_Stream.h"

#define  LOGIOH(...)  fprintf(stderr, "[%s] ", "PNGhandler" );   fprintf(stderr, __VA_ARGS__)

IM_PngIOHandler::IM_PngIOHandler()
{
}

IM_PngIOHandler::~IM_PngIOHandler()
{
}

IM_ImagePtr_t IM_PngIOHandler::read(const IM_ImageIOContext &context, IO_PersistentStore &store) const
{
	std::vector<unsigned char> png;
	unsigned width, height;

	png.resize(size_t(store.stream()->size()));
	store.loadBuffer( (unsigned char*)&png[0], store.stream()->size() );
	
	std::vector<unsigned char> image;
	lodepng::decode(image,width,height,(unsigned char*)&png[0],png.size());
	
	IM_ImagePtr_t pImage;
	pImage = new IM_Image(width, height, 4);
	size_t lineSize = 4 * width;

	// Just dump it in structure
	if ( context._premultiplyAlpha )
	{
		for( unsigned y=0; y<height; ++y )
		{
			for( unsigned x=0; x<width; ++x )
			{
			IM_Image::Channel_t r = image[ y*lineSize + (x<<2) ];
			IM_Image::Channel_t g = image[ y*lineSize + (x<<2) + 1 ];
			IM_Image::Channel_t b = image[ y*lineSize + (x<<2) + 2 ];
			IM_Image::Channel_t a = image[ y*lineSize + (x<<2) + 3 ];

			IM_Image::Channel_t *pixel = pImage->pixel(x, y);
			pixel[0] = (IM_Image::Channel_t)(r * a / 255);
			pixel[1] = (IM_Image::Channel_t)(g * a / 255);
			pixel[2] = (IM_Image::Channel_t)(b * a / 255);
			pixel[3] = a;
			}
		}
	}else{
		for( unsigned y=0; y<height; ++y )
		{
			memcpy( pImage->pixel(0, y), &image[ y*lineSize ], lineSize );
		}	
	}

	png.clear();
	image.clear();

	return pImage;
}

void IM_PngIOHandler::extensions(StringCol_t &extensions) const
{
  extensions.push_back( "png" );
  extensions.push_back( "PNG" );
}

REGISTER_IMAGE_HANDLER(IM_PngIOHandler);

#endif /* TARGET_DESKTOP elif TARGET_WEBGL */ 

