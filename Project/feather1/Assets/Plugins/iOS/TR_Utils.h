#ifndef _TR_UTILS_H_
#define _TR_UTILS_H_

#include "TR_Types.h"

#include "STD_Containers.h"
#include "STD_Types.h"

#include <string>
#include <set>

/*!
 *  @namespace TR_Utils
 *  Helper namespace for tree data structure.
 */
namespace TR_Utils
{
  typedef STD_Set< STD_String > NameCol_t;

  bool nameToChannel( const STD_String &name, TR_Types::CurveChannel_t &channel );
  bool channelToNames( TR_Types::CurveChannel_t channel, NameCol_t &names );
}

#endif /* _TR_UTILS_H_ */
