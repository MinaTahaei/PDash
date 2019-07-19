
#include "RD_RenderId.h"
#include "PL_Atomic.h"

//  Allow space for reserved ids.
static PL_AtomicInt32 g_uniqueId(50);

int RD_RenderId::uniqueId()
{
  return ++g_uniqueId;
}
