// Allocation and deallocation of memory.
#ifndef LEMSTL_LEM_ALLOC_H_
#define LEMSTL_LEM_ALLOC_H_

#include <new.h>

namespace lem {
// _THROW_BAD_ALLOC settings; 
// EM NOTE: we never use ::operator new here, 
// because we are separating allocation with construction.
#if 0 // never use ::operator new;
# include <new>
# define _THROW_BAD_ALLOC throw std::bad_alloc;
#elif !defined(_THROW_BAD_ALLOC)
# include <iostream> // for std::cout and std::endl;
# include <cstdlib> // for exit() and macros;
# define _THROW_BAD_ALLOC std::cout << "AllocationFailure: out of memory. " << std::endl; exit(EXIT_FAILURE)
#endif /* _THROW_BAD_ALLOC */

/* malloc()-based allocator, usually slower than the free-list-based one */
/* This allocator is thread-safe in most cases, */
/* and is usually more efficient in space utilization. */
template <int inst>
class _malloc_alloc_template {

};

/* free-list-based allocator, usually faster than the malloc()-based one */
// EM NOTE: multi-thread process is NOT DISCUSSED here for simplicity.
template <bool threads, int inst>
class _default_alloc_template {

};

/* Check whether to use free list */
#ifdef _USE_FREE_LIST
  using alloc = _malloc_alloc_template<0>;
#else
  using alloc = _default_alloc_template<0, 0>;
#endif /* _USE_FREE_LIST */
/* end */

// STL standard interface;
// Every container should specifiy its own simple_alloc() member functions;
template <typename T, typename Alloc>
class simple_alloc {
  static T* allocate(size_t n = 1) {
    return (n == 0 ? static_cast<T*>(nullptr) : (T*)Alloc::allocate(n * sizeof(T)));
  }
  static void deallocate(T* p, size_t n = 0) {
    if (n != 0) { Alloc::deallocate(p, n * sizeof(T)); }

    return;
  }
};
} // lem
#endif
