#ifndef _MEM_DEBUGOVERRIDE_H_
#define _MEM_DEBUGOVERRIDE_H_

#include <stdlib.h>
#include <stdio.h>
#include <cstddef>
#include <new>

#include <iostream>

#define MEM_NO_EXCEPTIONS

/*!
 *  @namespace MEM_DebugOverride
 *  Debug memory override.
 *  Turn on by compiling with MEM_DEBUG_OVERRIDE.
 */
namespace MEM_DebugOverride
{
  void  *malloc(size_t size);
  void   free(void *ptr);

  template <typename T>
  class allocator
  {
  public:
    typedef T                   value_type;
    typedef value_type         *pointer;
    typedef const value_type   *const_pointer;
    typedef value_type         &reference;
    typedef const value_type   &const_reference;
    typedef size_t              size_type;
    typedef ptrdiff_t           difference_type;

    allocator() { }

    allocator(const allocator&) { }

    template<typename U>
      allocator(const allocator<U>&) { }

    ~allocator() { }

    template <typename U>
    struct rebind
    {
      typedef allocator<U> other;
    };

    pointer address(reference x) const
    {
      return &x;
    }

    const_pointer address(const_reference x) const
    {
      return &x;
    }

    pointer allocate(size_type n, const_pointer = 0)
    {
      void *ptr = MEM_DebugOverride::malloc(n * sizeof(T));
#ifndef MEM_NO_EXCEPTIONS
      if ( !ptr )
        throw std::bad_alloc();
#endif

      return static_cast<pointer>(ptr);
    }

    void deallocate(pointer ptr, size_type)
    {
      MEM_DebugOverride::free(ptr);
    }

    size_type max_size() const
    {
      return static_cast<size_type>(-1) / sizeof(value_type);
    }

    void construct(pointer ptr, const value_type &x)
    {
      new(ptr) value_type(x);
    }

    void destroy(pointer ptr)
    {
      (void)ptr;
      ptr->~value_type();
    }
  private:
    void operator= (const allocator&);
  };

  void   initialize();
  void   deinitialize();
}

#define MEM_ALLOC          MEM_DebugOverride::malloc
#define MEM_DEALLOC        MEM_DebugOverride::free

#ifndef MEM_NO_EXCEPTIONS
#define MEM_OVERRIDE_NEW \
  void *operator new (size_t size) \
  { \
    void *ptr = MEM_ALLOC(size); \
    if (ptr==0) \
      throw std::bad_alloc(); \
    return ptr; \
  }
#else
#define MEM_OVERRIDE_NEW \
  void *operator new (size_t size) \
  { \
    void *ptr = MEM_ALLOC(size); \
    return ptr; \
  }
#endif

#ifndef MEM_NO_EXCEPTIONS
#define MEM_OVERRIDE_NEW_ARRAY \
  void *operator new[] (size_t size) \
  { \
    void *ptr = MEM_ALLOC(size); \
    if (ptr==0) \
      throw std::bad_alloc(); \
    return ptr; \
  }
#else
#define MEM_OVERRIDE_NEW_ARRAY \
  void *operator new[] (size_t size) \
  { \
    void *ptr = MEM_ALLOC(size); \
    return ptr; \
  }
#endif

#define MEM_OVERRIDE_PLACEMENT_NEW \
  void *operator new (size_t /*size*/, void *ptr) \
  { \
    return ptr; \
  }

#define MEM_OVERRIDE_PLACEMENT_NEW_ARRAY \
  void *operator new[] (size_t /*size*/, void *ptr) \
  { \
    return ptr; \
  }

#define MEM_OVERRIDE_DELETE \
  void operator delete (void *ptr) \
  { \
    MEM_DEALLOC(ptr); \
  }

#define MEM_OVERRIDE_DELETE_ARRAY \
  void operator delete[] (void *ptr) \
  { \
    MEM_DEALLOC(ptr); \
  }

#define MEM_OVERRIDE_PLACEMENT_DELETE \
  void operator delete (void * /*ptr*/, void * /*voidptr2*/) \
  { \
  }

#define MEM_OVERRIDE_PLACEMENT_DELETE_ARRAY \
  void operator delete[] (void * /*ptr*/, void * /*voidptr2*/) \
  { \
  }

#define MEM_OVERRIDE \
  public: \
    MEM_OVERRIDE_NEW \
    MEM_OVERRIDE_NEW_ARRAY \
    MEM_OVERRIDE_PLACEMENT_NEW \
    MEM_OVERRIDE_PLACEMENT_NEW_ARRAY \
    MEM_OVERRIDE_DELETE \
    MEM_OVERRIDE_DELETE_ARRAY \
    MEM_OVERRIDE_PLACEMENT_DELETE \
    MEM_OVERRIDE_PLACEMENT_DELETE_ARRAY

#define MEM_ALLOCATOR MEM_DebugOverride::allocator

#endif /* _MEM_DEBUGOVERRIDE_H_ */
