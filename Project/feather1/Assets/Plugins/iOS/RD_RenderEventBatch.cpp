
#include "RD_RenderEventBatch.h"
#include "RD_RenderEvent.h"
#include "RD_RenderScript.h"
#include "RD_Renderer.h"

#include "PL_Mutex.h"
#include "STD_Containers.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RenderEventBatch::Impl
#endif
class RD_RenderEventBatch::Impl
{
  friend class RD_RenderEventBatch;

public:
  Impl() :
    _frameCount(-1)
  {
  }

  ~Impl()
  {
  }

private:

  int              _frameCount;

  typedef STD_MultiMap< int, RD_RenderEvent* > RenderEventCol_t;
  RenderEventCol_t _events;

  PL_Mutex         _mutex;

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - RD_RenderEventBatch::Impl
#endif

RD_RenderEventBatch::RD_RenderEventBatch()
{
  _i = new Impl;
}

RD_RenderEventBatch::~RD_RenderEventBatch()
{
  flushEvents();

  delete _i;
}

void RD_RenderEventBatch::addEvent( int frameCount, int depth, RD_RenderEvent *renderEvent )
{
  _i->_mutex.lock();

  //  We changed frame, clear batch and start anew.
  if ( frameCount != _i->_frameCount )
  {
    flushEvents();
    _i->_frameCount = frameCount;
  }

  _i->_events.insert( STD_MakePair(depth, renderEvent) );

  _i->_mutex.unlock();
}

void RD_RenderEventBatch::clear()
{
  _i->_mutex.lock();

  flushEvents();
  _i->_frameCount = -1;

  _i->_mutex.unlock();
}

void RD_RenderEventBatch::render( RD_Renderer *renderer, int cullingMask, const Math::Matrix4x4 &projectionMatrix, const Math::Matrix4x4 &viewMatrix ) const
{
  _i->_mutex.lock();

  for ( Impl::RenderEventCol_t::const_iterator i = _i->_events.begin(), iEnd = _i->_events.end() ; i!=iEnd ; ++i )
  {
    RD_RenderEvent *event = i->second;
    if ( event->layerMask() & cullingMask )
    {
      RD_RenderScriptPtr_t pScript = RD_RenderScriptManager::instance()->object( event->scriptId() );
      if ( pScript.isValid() )
      {
        //renderer->setMatrices( projectionMatrix, viewMatrix * event->modelMatrix() );
        pScript->render( renderer, projectionMatrix, viewMatrix * event->modelMatrix() );
      }
    }
  }

  _i->_mutex.unlock();
}

void RD_RenderEventBatch::flushEvents()
{
  for ( Impl::RenderEventCol_t::iterator i = _i->_events.begin(), iEnd = _i->_events.end() ; i!=iEnd ; ++i )
  {
    delete i->second;
  }

  _i->_events.clear();
}
