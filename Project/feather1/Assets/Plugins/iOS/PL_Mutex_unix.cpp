
#include "PL_Mutex.h"

#if !defined(SUPPORT_CXX11_STANDARD) && (defined(TARGET_LINUX) || defined(TARGET_ANDROID) || defined(TARGET_WEBGL))

#include <pthread.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - PL_Mutex::Impl
#endif
class PL_Mutex::Impl
{
  friend class PL_Mutex;

public:

  Impl()
  {
  }

  ~Impl()
  {
  }

  pthread_mutex_t _mutex;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#pragma mark - PL_Mutex
#endif

PL_Mutex::PL_Mutex()
{
  _i = new Impl;

  int ret = pthread_mutex_init(&_i->_mutex, NULL);

  if ( ret != 0 )
    fprintf( stderr, "Error in mutex initialization!\n" );
}

PL_Mutex::~PL_Mutex()
{
  pthread_mutex_destroy(&_i->_mutex);

  delete _i;
}

void PL_Mutex::lock()
{
  pthread_mutex_lock(&_i->_mutex);
}

bool PL_Mutex::try_lock()
{
  int ret = pthread_mutex_trylock(&_i->_mutex);
  return (ret == 0);
}

void PL_Mutex::unlock()
{
  pthread_mutex_unlock(&_i->_mutex);
}

#endif
