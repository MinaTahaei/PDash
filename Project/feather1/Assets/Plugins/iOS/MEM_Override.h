#ifndef _MEM_OVERRIDE_H_
#define _MEM_OVERRIDE_H_

#ifdef MEM_DEBUG_OVERRIDE

#include "MEM_DebugOverride.h"

#elif defined(MEM_CUSTOM_OVERRIDE)

#include "MEM_CustomOverride.h"

#else

/* default */

#define MEM_ALLOC              malloc
#define MEM_DEALLOC            free

#define MEM_OVERRIDE           /* nothing to do */

//  default stl allocator
#define MEM_ALLOCATOR          std::allocator

#endif

#endif /* _MEM_OVERRIDE_H_ */
