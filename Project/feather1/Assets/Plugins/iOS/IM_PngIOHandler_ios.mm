#include "IM_PngIOHandler.h"
#include "PL_Configure.h"

#ifdef TARGET_IOS

#include "IO_PersistentStore.h"
#include "IO_Stream.h"

#import <UIKit/UIKit.h>

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
  const IO_Stream *stream = store.stream();
  if ( stream == NULL )
    return IM_ImagePtr_t(0);

  size_t bufferSize = store.stream()->size();
  unsigned char *buffer = (unsigned char*)malloc( bufferSize * sizeof(unsigned char) );
  store.loadBuffer( buffer, bufferSize );

  NSData *data = [[NSData alloc] initWithBytesNoCopy: buffer length: bufferSize];
  UIImage *image = [[UIImage alloc] initWithData: data];

  //NSString *fullPath = [NSString stringWithUTF8String:filename.c_str()];
  //UIImage *image = [[UIImage alloc] initWithContentsOfFile: fullPath];

  CGImageRef cgImage = image.CGImage;

  unsigned width = CGImageGetWidth(cgImage);
  unsigned height = CGImageGetHeight(cgImage);
  unsigned depth = CGImageGetBitsPerPixel(cgImage) / 8;

  CGImageAlphaInfo info = CGImageGetAlphaInfo(cgImage);
  CGColorSpaceRef colorSpace = CGImageGetColorSpace(cgImage);
  if ( !colorSpace )
    return IM_ImagePtr_t(0);

  if ( depth == 4u )
  {
    info = kCGImageAlphaPremultipliedLast;
  }
  else
  {
    info = kCGImageAlphaNoneSkipLast;
  }

  IM_ImagePtr_t pImage = new IM_Image( width, height, depth );

  colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef cgContext = CGBitmapContextCreate( pImage->data(),
                                                width,
                                                height,
                                                8,
                                                4 * width,
                                                colorSpace,
                                                info | kCGBitmapByteOrder32Big );

  CGContextClearRect(cgContext, CGRectMake(0, 0, width, height));
  CGContextDrawImage(cgContext, CGRectMake(0, 0, width, height), cgImage);

  CGContextRelease(cgContext);
  CFRelease(colorSpace);

  if ( !context._premultiplyAlpha )
  {
    //  Unmultiply alpha in image
    if ( depth == 4 )
    {
      for ( unsigned y = 0 ; y < height ; ++y )
      {
        for ( unsigned x = 0 ; x < width ; ++x )
        {
          IM_Image::Channel_t *pixel = pImage->pixel(x,y);
          IM_Image::Channel_t alpha = pixel[3];
          if ( alpha > 0 )
          {
            pixel[0] = (IM_Image::Channel_t)(pixel[0] * 255 / alpha);
            pixel[1] = (IM_Image::Channel_t)(pixel[1] * 255 / alpha);
            pixel[2] = (IM_Image::Channel_t)(pixel[2] * 255 / alpha);
          }
        }
      }
    }
  }

#if !__has_feature(objc_arc)
  [image release];
  [data release];
#endif

  return pImage;
}

void IM_PngIOHandler::extensions(StringCol_t &extensions) const
{
  extensions.push_back( "png" );
  extensions.push_back( "PNG" );
}

REGISTER_IMAGE_HANDLER(IM_PngIOHandler);

#endif /* TARGET_IOS */

