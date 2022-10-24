// Allocation and deallocation of memory.
#ifndef LEMSTL_LEM_ALLOC_H_
#define LEMSTL_LEM_ALLOC_H_

#include <new.h>

namespace lem {
/* _THROW_BAD_ALLOC settings */
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
// This allocator is thread-safe in most cases,
// and is usually more efficient in space utilization.
template <int inst>
class __malloc_alloc_template {
 private:
  /* functions for Out-Of-Memory cases */
  static void* oom_malloc(size_t n);
  static void* oom_realloc(void* p, size_t n);
  // the solution to oom;
  // For handler design, see <Effective C++>, 2e, Item 7, or 3e, Item 49.
  // For the necessity of handler function, see https://www.cnblogs.com/lang5230/p/5556611.html;
  // In short, __default_alloc_template will only return free-list memory
  // after the WHOLE program terminates. Large blocks of continuous memory 
  // may be unvailable to __malloc_alloc_template with too many free-list nodes.
  // Handlers will try to return continuous empty free-list nodes to system heap.
  static void (*__malloc_alloc_oom_handler)();
  /* end oom functions */

 public:
  static void* allocate(size_t n) {
    void* result = malloc(n);
    if (result == nullptr) {
      result = oom_malloc(n);
    }

    return result;
  }
  // EM NOTE: the argument n is not used here, only for interface consistency.
  static void deallocate(void* p, size_t /* n */) {
    free(p);

    return;
  }
  static void* reallocate(void* p, size_t /* prev_size */, size_t new_size) {
    void* result = realloc(p, new_size);
    if (result == nullptr) {
      result = oom_realloc(p, new_size);
    }

    return result;
  }

  // to customize oom handler;
  static void (*set_malloc_handler(void (*new_malloc_handler)()))(){
    void (*prev_malloc_handler)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = new_malloc_handler;

    return prev_malloc_handler;
  }
};

// For member variable initialization, see https://blog.csdn.net/mal327/article/details/6894769;
// For member variable initialization in class template, see https://blog.csdn.net/zjq2008wd/article/details/38417859;
template <int inst>
void (*__malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = nullptr;

template <int inst>
void* __malloc_alloc_template<inst>::oom_malloc(size_t n) {
  void (*malloc_handler)() = nullptr;
  void* result = nullptr;

  for (;;) { // Try free & alloc;
    malloc_handler = __malloc_alloc_oom_handler;
    if (malloc_handler == nullptr) { // if there is no solution to oom;
      _THROW_BAD_ALLOC; // exit;
    }
    // Now handle oom cases;
    malloc_handler();
    result = malloc(n); // try allocate again;

    if (result != nullptr) {
      return result;
    }
  }
}

template <int inst>
void* __malloc_alloc_template<inst>::oom_realloc(void* p, size_t n) {
  void (*malloc_handler)() = nullptr;
  void* result = nullptr;

  for (;;) { // Try free & realloc;
    malloc_handler = __malloc_alloc_oom_handler;
    if (malloc_handler == nullptr) { // if there is no solution to oom;
      _THROW_BAD_ALLOC; // exit;
    }
    // Now handle oom cases;
    malloc_handler();
    result = realloc(p, n); // try allocate again;

    if (result != nullptr) {
      return result;
    }
  }
}

using malloc_alloc = __malloc_alloc_template<0>;
/* end malloc()-based allocator */


/* free-list-based allocator, usually faster than the malloc()-based one */
// free-list constants;
constexpr size_t __kAlign = 8; // block size increment;
constexpr size_t __kMaxBytes = 128; // max size of a free-list;
constexpr size_t __kNumFreeList = __kMaxBytes / __kAlign; // number of kinds of free-lists;

// EM NOTE: multi-thread process is NOT DISCUSSED here for simplicity.
// To provide a thread-safe environment, inline is not used and volatile is used often.
template <bool threads, int inst>
class __default_alloc_template {
 private:
  // align `req_bytes` to __kAlign;
  // This function is correct only if __kAlign is some power of 2;
  static size_t round_up(size_t req_bytes) {
    return (req_bytes + __kAlign - 1) & ~(__kAlign - 1);
  }

  // free-list node;
  typedef union FreeListNode {
    union FreeListNode* next_;
    char data_[1];
  } FreeListNode, *FreeList;

  // EM NOTE: free-list header list;
  //
  // free-list[0]   --   free-list[1]  --  ...
  //     |                 |
  // list_8sized[0]    list_16sized[0]
  //     |                 |
  // list_8sized[1]    list_16sized[1]
  //     |                 |
  //    ...               ...
  //
  // For keyword volatile, see https://www.runoob.com/w3cnote/c-volatile-keyword.html;
  static FreeList volatile free_list[__kNumFreeList];
  // choose min-sized suitable free-list for required memory size;
  // EM NOTE: This is algebraicly equivalent to
  // (req_bytes - 1) / __kAlign,
  // but this one avoids size_t overflow for req_bytes == 0;
  static size_t free_list_get_ind(size_t req_bytes) {
    return (req_bytes + __kAlign - 1) / __kAlign - 1;
  }

  /* Memory arrangment */
    /* Memory pool */
    // memory_pool_alloc() tries to return a chunk 
    // of memory (sized free_list_node_size * num_node) to refill(),
    // and refill() will use this chunk to return a node and extend free-lists.
    // If remaining memory is less than required chunk size, certain strategy is used to:
    // 1. return the remaining memory blocks;
    // 2. allocate memory from system heap;
    // 3. rearrange spare memory from other free-lists;
    // 4. call malloc_alloc_template to solve oom cases;
    // num_node may decrease when allocation fails;
    static char* memory_pool_alloc(size_t free_list_node_size, size_t& num_node);

    // variables for memory pool;
    static char* memory_pool_head; 
    static char* memory_pool_tail;
    static size_t memory_pool_size;
    /* end memory pool*/

  // When a certain sized (say sized n) free-list has no spare node,
  // refill() tries to return an n-sized memory block from memory pool,
  // and if there is extra memory in the pool,
  // an extra number of empty nodes will link to the n-sized-node free-list.
  // EM NOTE: This behaviour is similar to memory allocation of dynamic array, 
  // where we realloc twice the memory it had when previous memory is used up.
  static void* refill(size_t free_list_node_size);
  /* end memory arrangment */

 public:
  static void* allocate(size_t n);
  static void deallocate(void* p, size_t n);
  static void* reallocate(void* p, size_t prev_size, size_t new_size);
};

template <bool threads, int inst>
static __default_alloc_template<threads, inst>::FreeList volatile
__default_alloc_template<threads, inst>::free_list[__kNumFreeList] =
{ nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr
};
template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::memory_pool_head = 0;
template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::memory_pool_tail = 0;
template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::memory_pool_size = 0;

template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::memory_pool_alloc(size_t free_list_node_size, size_t& num_node) {
  // Compute remaining memory in memory pool;
}
/* end free-list-based allocator */


/* Check whether to use free list */
///*
#define _FREE_LIST_OFF
//*/
#ifdef _FREE_LIST_OFF
  using alloc = __malloc_alloc_template<0>;
#else
  using alloc = __default_alloc_template<0, 0>;
#endif /* _FREE_LIST_OFF */
/* end check */

// STL standard interface;
// Every container should specifiy its own simple_alloc() member functions;
template <typename T, typename Alloc>
class simple_alloc {
 public:
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
