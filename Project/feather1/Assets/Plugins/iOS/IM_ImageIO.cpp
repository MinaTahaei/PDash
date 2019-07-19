#include "IM_ImageIO.h"

#include "IO_File.h"
#include "IO_PersistentStore.h"
#include "PL_FileSpec.h"
#include "STD_Types.h"
#include "STD_Containers.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - IM_ImageIOContext
#endif
IM_ImageIOContext::IM_ImageIOContext( bool premultiplyAlpha ) :
  _premultiplyAlpha(premultiplyAlpha)
{
}

IM_ImageIOContext::~IM_ImageIOContext()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - IM_ImageIOHandler
#endif
IM_ImageIOHandler::IM_ImageIOHandler()
{
}

IM_ImageIOHandler::~IM_ImageIOHandler()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - IM_ImageIO::Impl
#endif
class IM_ImageIO::Impl
{
  friend class IM_ImageIO;

public:

  Impl()
  {
  }

  ~Impl()
  {
  }

private:

  typedef STD_Vector<IM_ImageIOHandler*> HandlerCol_t;
  HandlerCol_t _handlers;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - IM_ImageIO
#endif

IM_ImageIO *IM_ImageIO::instance()
{
  static IM_ImageIO *g_instance = 0;

  if (g_instance == 0)
  {
    g_instance = new IM_ImageIO;
  }

  return g_instance;
}

IM_ImageIO::IM_ImageIO()
{
  _i = new Impl;
}

IM_ImageIO::~IM_ImageIO()
{
  for ( Impl::HandlerCol_t::const_iterator i = _i->_handlers.begin() , iEnd = _i->_handlers.end() ; i!=iEnd ; ++i )
  {
    delete *i;
  }

  delete _i;
}

IM_ImagePtr_t IM_ImageIO::read(const IM_ImageIOContext &context, const STD_String &filename) const
{
  size_t idx = filename.find_last_of('.');
  if ( idx == STD_String::npos )
    return IM_ImagePtr_t(0);

  STD_String ext = filename.substr(idx+1);

  for ( Impl::HandlerCol_t::const_iterator i = _i->_handlers.begin() , iEnd = _i->_handlers.end() ; i!=iEnd ; ++i )
  {
    IM_ImageIOHandler *handler = *i;

    IM_ImageIOHandler::StringCol_t extensions;
    handler->extensions(extensions);

    for ( IM_ImageIOHandler::StringCol_t::const_iterator i = extensions.begin(), iEnd = extensions.end() ; i!=iEnd ; ++i )
    {
      if ( ext.compare( *i ) == 0 )
      {
        IM_ImagePtr_t ret;

        PL_FileSpec fileSpec( filename.c_str() );
        IO_File file;

        if ( file.openForInput( fileSpec ) )
        {
          IO_PersistentStore store(file);
          ret = handler->read(context, store);

          file.close();
        }

        return ret;
      }
    }
  }

  return IM_ImagePtr_t(0);
}

void IM_ImageIO::addHandler(IM_ImageIOHandler *handler)
{
  _i->_handlers.push_back(handler);
}

