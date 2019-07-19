
#ifndef _IM_PNG_IO_HANDLER_H_
#define _IM_PNG_IO_HANDLER_H_

#include "IM_ImageIO.h"

class IO_PersistentStore;

/*!
 *  @class IM_PngIOHandler
 *  PNG IO Handler implementation.
 */
class IM_PngIOHandler : public IM_ImageIOHandler
{
public:

  IM_PngIOHandler();
  virtual ~IM_PngIOHandler();

  virtual IM_ImagePtr_t read(const IM_ImageIOContext &context, IO_PersistentStore &store) const;
  virtual void          extensions(StringCol_t &extensions) const;

private:

  class Impl;
  Impl *_i;

};

#endif /* _IM_PNG_IO_HANDLER_H_ */
