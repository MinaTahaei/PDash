
#ifndef _IM_IMAGE_IO_H_
#define _IM_IMAGE_IO_H_

#include "IM_Image.h"

#include "STD_Types.h"
#include "STD_Containers.h"

class IO_PersistentStore;

#define REGISTER_IMAGE_HANDLER(HandlerType) \
  static struct Register##HandlerTypeStruct \
  { \
    Register##HandlerTypeStruct() \
    { \
      IM_ImageIO::instance()->addHandler(new HandlerType); \
    } \
  } register##HandlerType;

/*!
 *  @class IM_ImageIOContext
 *  Context for image io.
 */
class IM_ImageIOContext
{
public:

  IM_ImageIOContext( bool premultiplyAlpha );
  ~IM_ImageIOContext();

public:

  bool _premultiplyAlpha;

};


/*!
 *  @class IM_ImageIOHandler
 *  Abstract IO handler for image
 */
class IM_ImageIOHandler
{
public:

  typedef STD_Vector<STD_String> StringCol_t;

public:
  IM_ImageIOHandler();
  virtual ~IM_ImageIOHandler();

  virtual IM_ImagePtr_t read(const IM_ImageIOContext &context, IO_PersistentStore &store) const =0;
  virtual void          extensions(StringCol_t &extensions) const =0;

};

/*!
 *  @class IM_ImageIO
 *  IO Manager for image
 */
class IM_ImageIO
{
  friend class IM_ImageIOHandler;
public:

  static IM_ImageIO *instance();
  virtual ~IM_ImageIO();

public:

  IM_ImagePtr_t read(const IM_ImageIOContext &context, const STD_String &filename) const;

  void          addHandler(IM_ImageIOHandler *handler);

private:

  IM_ImageIO();

private:

  class Impl;
  Impl *_i;
};

#endif /* _IM_IMAGE_IO_H_ */
