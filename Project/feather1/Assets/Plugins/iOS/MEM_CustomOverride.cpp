
#include "MEM_CustomOverride.h"

// Implement your custom memory allocators first before compiling

namespace MEM_CustomOverride
{
  void *malloc(size_t size)
  {
    /*  Implement me! */
    return 0;
  }

  void free(void *ptr)
  {
    /*  Implement me! */
  }
}
