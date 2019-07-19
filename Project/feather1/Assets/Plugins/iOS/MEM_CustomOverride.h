#ifndef _MEM_CUSTOMOVERRIDE_H_
#define _MEM_CUSTOMOVERRIDE_H_

#include <stdlib.h>
#include <cstddef>
#include <new>

/*!
 *  @namespace MEM_CustomOverride
 *  User specific memory override.
 *  Turn on by compiling with MEM_CUSTOM_OVERRIDE.
 */
namespace MEM_CustomOverride
{
  void *malloc(size_t size);
  void free(void *ptr);

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
      /* Implement me! */
      return 0;
    }

    void deallocate(pointer ptr, size_type)
    {
      /* Implement me! */
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
      ptr->~value_type();
    }
  private:
    void operator= (const allocator&);
  };

}

#define MEM_ALLOC          MEM_CustomOverride::malloc
#define MEM_DEALLOC        MEM_CustomOverride::free

#define MEM_OVERRIDE_NEW \
  void *operator new (size_t size) \
  { \
    /*  Implement me! */ \
    return 0; \
  }

#define MEM_OVERRIDE_NEW_ARRAY \
  void *operator new[] (size_t size) \
  { \
    /*  Implement me! */ \
    return 0; \
  }

#define MEM_OVERRIDE_PLACEMENT_NEW \
  void *operator new (size_t /*size*/, void *ptr) \
  { \
    /*  Implement me! */ \
    return 0; \
  }

#define MEM_OVERRIDE_PLACEMENT_NEW_ARRAY \
  void *operator new[] (size_t /*size*/, void *ptr) \
  { \
    /*  Implement me! */ \
    return 0; \
  }

#define MEM_OVERRIDE_DELETE \
  void operator delete (void *ptr) \
  { \
    /*  Implement me! */ \
  }

#define MEM_OVERRIDE_DELETE_ARRAY \
  void operator delete[] (void *ptr) \
  { \
    /*  Implement me! */ \
  }

#define MEM_OVERRIDE_PLACEMENT_DELETE \
  void operator delete (void *ptr, void *voidptr2) \
  { \
    /* Implement me! */ \
  }

#define MEM_OVERRIDE_PLACEMENT_DELETE_ARRAY \
  void operator delete[] (void *ptr, void *voidptr2) \
  { \
    /* Implement me! */ \
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
    MEM_OVERRIDE_PLACEMENT_DELETE_ARRAY \

#define MEM_ALLOCATOR MEM_CustomOverride::allocator

#endif /* _MEM_CUSTOMOVERRIDE_H_ */
