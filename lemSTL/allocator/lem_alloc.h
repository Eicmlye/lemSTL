// Allocation and deallocation of memory.
#ifndef LEMSTL_LEM_ALLOC_H_
#define LEMSTL_LEM_ALLOC_H_

#include <cstdlib> // for std::malloc() and std::free(); for exit() and macros;
#include <cassert> // for macro assert();

#include "../lem_exception" // for lem::alloc_zero_free_list;

namespace lem {
/* _THROW_BAD_ALLOC settings */
// EM NOTE: we never use ::operator new here, 
// because we are separating allocation with construction.
#if 0 // never use ::operator new;
# include <new>
# define _THROW_BAD_ALLOC throw std::bad_alloc();
#elif !defined(_THROW_BAD_ALLOC)
# include <iostream> // for std::cout and std::endl;
# define _THROW_BAD_ALLOC ::std::cout << "AllocationFailure: out of memory. " << ::std::endl; exit(EXIT_FAILURE)
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

// EM NOTE: Free-list memory process:
//         __malloc_alloc_template::allocate()
// System heap memory      ---->         memory pool
//                                            |
//                            mempool_alloc() | refill()
//                                            \/
//     user pointer       <----        free-list system
//        __default_alloc_template::allocate()

// EM NOTE: Free-list system:
// free-list[0]   -->   free-list[1]   ->   ...
//      |                   |
// FreeListNode         FreeListNode
//      |                   |
// FreeListNode         FreeListNode
//      |                   |
//     ...                 ...

// EM NOTE:
// free-list structure: // sizeof(char) = 1(byte)
// free_list[...] is stored in stack memory, not in heap memory.
// FreeListNode* free-list[i]                       //       -----     sizeof(addr), &node1
//      |                                           //       |     ----------------------------
// FreeListNode node1 { next_ = &node3; }           //       ---->|      8*i bytes, &node3     |-----
//      |                                           //             ----------------------------     |
// FreeListNode node2 { data_ = &data2; }           // userP2 --> |      8*i bytes, data2      |    |
//      |                                           //             ----------------------------     |
// FreeListNode node3 { next_ = &node5; }           //       -----|      8*i bytes, &node5     |<----
//      |                                           //       |     ----------------------------
// FreeListNode node4 { next_ = nullptr; }          //       |    |      8*i bytes, empty      |<----
//      |                                           //       |     ----------------------------     |
// FreeListNode node5 { next_ = &node4; }           //       ---->|      8*i bytes, &node4     |-----
//      |                                           //             ----------------------------
// FreeListNode node6 { data_ = &data6; }           // userP6 --> |      8*i bytes, data6      |
//                                                  //             ----------------------------

// EM NOTE: insertation to free-lists is made at the head of the list.

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
  using FreeListNode = union FreeListNode {
    union FreeListNode* next_; // When node is empty, this address next_ is stored in data memory.
    char data_[1]; // This must be in array form to save memory.
  };
  using FreeList = FreeListNode*;

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
  // choose min-sized proper free-list for required memory size;
  static size_t free_list_get_ind(size_t req_bytes_nonzero) {
    if (req_bytes_nonzero == 0) {
      throw lem::alloc_zero_free_list();
    }

    return (req_bytes_nonzero - 1) / __kAlign;
  }

  /* Memory arrangment */
    /* Memory pool */
    // mempool_alloc() tries to return a chunk 
    // of memory (sized free_list_node_size * num_node) to refill(),
    // and refill() will use this chunk to return a node and extend free-lists.
    // If remaining memory is less than required chunk size, certain strategy is used to:
    // 1. return the remaining memory blocks;
    // 2. allocate memory from system heap;
    // 3. rearrange spare memory from other free-lists;
    // 4. call malloc_alloc_template to solve oom cases;
    // num_node may decrease when allocation fails;
  static char* mempool_alloc(size_t free_list_node_size, size_t& num_node);

  // variables for memory pool;
  static char* mempool_head;
  static char* mempool_tail;
  static size_t alloced_from_heap;
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
typename __default_alloc_template<threads, inst>::FreeList volatile
__default_alloc_template<threads, inst>::free_list[__kNumFreeList] =
{ nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr,
  nullptr, nullptr, nullptr, nullptr
};
template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::mempool_head = nullptr;
template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::mempool_tail = nullptr;
template <bool threads, int inst>
size_t __default_alloc_template<threads, inst>::alloced_from_heap = 0;

template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::mempool_alloc(size_t free_list_node_size,
                                                             size_t& num_node
) {
  char* result = nullptr;

  // Compute remaining memory in memory pool;
  size_t req_bytes = free_list_node_size * num_node;
  size_t remain_bytes = mempool_tail - mempool_head;

  // if remaining memory is adequate;
  if (remain_bytes >= req_bytes) {
    // allocate according to the request;
    result = mempool_head;
    mempool_head += req_bytes;

    return result;
  }

  // Now not adequate;
  // if remaining memory is not less than 1 node size;
  if (remain_bytes >= free_list_node_size) {
    // update allocated number of nodes;
    num_node = remain_bytes / free_list_node_size;

    // allocate as many nodes as possible;
    result = mempool_head;
    mempool_head += free_list_node_size * num_node;

    return result;
  }

  // Now remaining memory is less than 1 node size;
  // EM NOTE: expand memory pool with the similar strategy to array.
  // `alloced_from_heap >> 4` == `alloced_from_heap / 16`,
  // which computes the average memory that every free-list has been allocated.
  size_t mempool_req_bytes = 2 * req_bytes + round_up(alloced_from_heap >> 4);

  // Try to give out remaining memory in the pool;
  if (remain_bytes != 0) {
    // find max-sized proper free-list;
    // EM NOTE: we find the max-sized one 
    // to make the allocated memory as continuous as possible.
    // Notice that the memory in the pool is n*8 bytes, so are the allocated ones.
    // Hence remain_bytes will always fit exactly into one free-list.
    FreeList volatile* plist = free_list + free_list_get_ind(remain_bytes);

    // insert remain_bytes to the free-list;
    ((FreeListNode*)mempool_head)->next_ = *plist; // *plist == free_list[] == &(some_FreeListNode);
    (*plist)->next_ = (FreeListNode*)mempool_head;
  }

  // Try to get more memory from system heap memory;
  mempool_head = (char*)malloc(mempool_req_bytes);
  // if system heap is also empty;
  if (mempool_head == nullptr) {
    // Check if there is spare block in larger free-lists;
    FreeList volatile* mov = nullptr;
    for (size_t size = free_list_node_size + __kAlign; size <= __kMaxBytes; size += __kAlign) {
      mov = free_list + free_list_get_ind(size);

      // if there is spare block;
      if (*mov != nullptr) {
        // recycle this memory block to memory pool;
        // EM NOTE: you should not, and are usually unable to
        // recycle all the spare blocks, because after the program
        // runs for a while, the spare blocks are out of order
        // (though they may be continuous overall, and this is because
        // insertation and deallocate is always made at the head of lists).
        mempool_head = (char*)(*mov);
        mempool_tail = mempool_head + size;
        *mov = (*mov)->next_;

        // modify num_node and allocate;
        return mempool_alloc(free_list_node_size, num_node);
      }
    }

    // Now larger empty blocks do not exit;
    mempool_tail = nullptr;
    // try to call __malloc_alloc_template;
    mempool_head = (char*)malloc_alloc::allocate(mempool_req_bytes);
    /* oom_handler will deal with the situation *//* end oom */
  }

  // Now allocation succeeded;
  alloced_from_heap += mempool_req_bytes;
  mempool_tail = mempool_head + mempool_req_bytes;

  // modify num_node and allocate;
  return mempool_alloc(free_list_node_size, num_node);
}

template <bool threads, int inst>
void* __default_alloc_template<threads, inst>::refill(size_t free_list_node_size) {
  assert(free_list_node_size % 8 == 0); // must round_up() first;

  // try to get 20 nodes by default;
  size_t num_node = 20;
  char* mem_block = mempool_alloc(free_list_node_size, num_node);

  // Now allocated successfully
  // (if failed, mempool_alloc() will call exit());
  if (num_node == 1) {
    return (void*)mem_block;
  }

  // Now allocated more than one node;
  // allocate 1 node to user;
  FreeListNode* result = (FreeListNode*)mem_block;
  // link the rest to free-list;
  // EM NOTE: notice that since we are calling refill(),
  // now (*plist)->next_ == nullptr.
  FreeList volatile* plist = free_list + free_list_get_ind(free_list_node_size);
  *plist = (FreeListNode*)mem_block + free_list_node_size;
  FreeListNode* cur = (FreeListNode*)(mem_block + free_list_node_size);
  FreeListNode* pred = nullptr;
  for (size_t index = 1; index < num_node - 1; ++index) {
    pred = cur;
    cur = (FreeListNode*)(mem_block + (index + 1) * free_list_node_size);
    pred->next_ = cur;
  }
  // Now cur is the last new node;
  cur->next_ = nullptr;

  return (void*)result;
}

template <bool threads, int inst>
void* __default_alloc_template<threads, inst>::allocate(size_t n) {
  if (n > __kMaxBytes) {
    return malloc_alloc::allocate(n);
  }

  // Now n <= __kMaxBytes, use free-list system;
  FreeList volatile* plist = free_list + free_list_get_ind(n);
  // if proper node does not exist;
  if (*plist == nullptr) {
    return refill(round_up(n));
  }

  // Now proper node exists;
  FreeListNode* result = *plist;
  *plist = (*plist)->next_;

  return result;
}
template <bool threads, int inst>
void __default_alloc_template<threads, inst>::deallocate(void* p, size_t n) {
  if (n > __kMaxBytes) {
    malloc_alloc::deallocate(p, n);

    return;
  }

  // Now n <= __kMaxBytes;
  // find proper free-list;
  FreeList volatile* plist = free_list + free_list_get_ind(n);
  FreeListNode* recycled = (FreeListNode*)p;
  recycled->next_ = *plist;
  *plist = recycled;

  return;
}
template <bool threads, int inst>
void* __default_alloc_template<threads, inst>::reallocate(void* p, size_t prev_size, size_t new_size) {
  /* This function has no interface in simple_alloc, so we will not give an implementation. */
}
/* end free-list-based allocator */


/* Check whether to use free list */
#ifdef _FREE_LIST_OFF
using alloc = __malloc_alloc_template<0>;
#else
using alloc = __default_alloc_template<false, 0>;
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
    if (n != 0) {
      Alloc::deallocate(p, n * sizeof(T));
    }

    return;
  }
};
} // lem
#endif
