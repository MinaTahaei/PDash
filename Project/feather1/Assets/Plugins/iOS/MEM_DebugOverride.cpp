
#include "MEM_DebugOverride.h"

#include <map>

namespace MEM_DebugOverride
{
  //  Static map to cumulate memory allocation.
  //  *** Not thread safe.
  typedef std::map< void*, size_t > AllocationMap_t;
  static AllocationMap_t *g_allocation_map = NULL;

  void *malloc(size_t size)
  {
    void *ptr = ::malloc(size);

    if ( g_allocation_map != NULL )
    {
      g_allocation_map->insert( std::make_pair(ptr, size) );
    }

    return ptr;
  }

  void free(void *ptr)
  {
    if ( (g_allocation_map != NULL) && (ptr != NULL) )
    {
      AllocationMap_t::iterator iAlloc = g_allocation_map->find( ptr );
      if ( iAlloc == g_allocation_map->end() )
      {
        fprintf( stderr, "double deallocation at %p!\n", ptr );
        abort();
      }
      else
      {
        g_allocation_map->erase( iAlloc );
      }
    }

    ::free(ptr);
  }

  void initialize()
  {
    g_allocation_map = new AllocationMap_t;
  }

  void deinitialize()
  {
    printf( "Objects allocated: %lu.\n", g_allocation_map->size() );

    size_t allocated_memory = 0;
    for ( AllocationMap_t::const_iterator i= g_allocation_map->begin(), iEnd = g_allocation_map->end() ; i!=iEnd ; ++i )
    {
      printf( "  (%p): %lu bytes.\n", i->first, i->second );
      allocated_memory += i->second;
    }

    printf( "Total Memory allocated: %lu bytes.\n", allocated_memory );

    delete g_allocation_map;
    g_allocation_map = 0;
  }
}


