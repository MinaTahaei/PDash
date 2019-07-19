#ifndef _STD_CONTAINERS_H_
#define _STD_CONTAINERS_H_

#include "MEM_Override.h"

#include <vector>
#include <map>
#include <set>
#include <list>

#include <algorithm>

#define STD_Pair     std::pair
#define STD_MakePair std::make_pair

#define STD_Find     std::find
#define STD_FindIf   std::find_if

#define STD_Sort     std::sort

#define STD_Distance std::distance
#define STD_Advance  std::advance

#define STD_IterSwap std::iter_swap
#define STD_Swap     std::swap

template <typename T>
class STD_Vector : public std::vector< T, MEM_ALLOCATOR<T> >
{
};

template <typename K, typename T, typename Compare = std::less<K> >
class STD_Map : public std::map< K, T, Compare, MEM_ALLOCATOR< std::pair<const K,T> > >
{
};

template <typename K, typename T, typename Compare = std::less<K> >
class STD_MultiMap : public std::multimap< K, T, Compare, MEM_ALLOCATOR< std::pair<const K, T> > >
{
};

template <typename T, typename Compare = std::less<T> >
class STD_Set : public std::set< T, Compare, MEM_ALLOCATOR<T> >
{
};

template <typename T>
class STD_List : public std::list< T, MEM_ALLOCATOR<T> >
{
};

#endif /* _STD_CONTAINERS_H_ */
