// Sequential containers;
#ifndef LEMSTL_LEM_VECTOR_H_
#define LEMSTL_LEM_VECTOR_H_

#ifdef LEM_DEBUG
#include <iostream> // for cout, endl;
#include <cstdlib> // for exit();

using std::cout;
using std::endl;
#endif

#include <cstddef> // for ::std::ptrdiff_t;

#include "../lem_memory"
#include "../lem_iterator"
#include "../lem_exception"
#include "../lem_type_traits" // for __type_traits;

/* NOTICE ON try-catch BLOCKS FOR CONTAINERS */
// EM NOTE: be caution to the logic of try-catch blocks:
// always use try-catch if you use uninitialized_xxx(),
// and only use in the following pattern:
//
// try {
//  uninitialized_xxx(...);
//  construct(...);
//  ... // There may be several unini_xxx() and construct();
//  uninitialized_xxx(...);
//  construct(...);
// }
// catch (const std::exception& e) {
//  /* commit or rollback semantics */
//  destroy(...); // unnecessary only if there is only ONE construct() used, and no unini_xxx() is used;
//  // data_allocator::deallocate(...); // only if you allocated new memory before try-catch;
//  throw e;
// }
// 
// The destroy() work has NOT been done by uninitialized-xxx().
// If exception occurs, you should destroy all the objects in a single round.
//
// You should be clear that the above block will only catch exceptions in construct(),
// i.e. in the ctor(). And you should throw the exception out to the user function
// to let the user know this construction failure. 

namespace lem {
// See declarations at https://en.cppreference.com/w/cpp/container/vector;
template <typename DataType, typename AllocType = alloc>
class vector {
 public:
  // Member types;
  // allocator traits;
  using allocator_type    = AllocType;
  // iterator traits;
  using iterator          = DataType*; 
  using reverse_iterator  = DataType*;
  /* using iterator_category = random_access_iterator_tag; */ // native pointer iterator;
  using value_type        = DataType;
  using difference_type   = ptrdiff_t;
  using pointer_type      = DataType*;
  using reference_type    = DataType&;

  // Data type;
  using size_type         = size_t;

 protected:
  // memory allocation;
  using data_allocator    = simple_alloc<value_type, allocator_type>;

  // EM NOTE:
  //            data
  //   <--------------------->
  //                    capacity
  //   <--------------------------------------->
  //   | 1 | 2 | 3 | ... | n |   |   | ... |   |xxxxxx
  //   ^                     ^                 ^
  //   |                     |                 |
  // begin()                end()              |
  // mem_head             data_tail         mem_tail
  iterator mem_head_;
  iterator data_tail_;
  iterator mem_tail_;

 public:
  /* ctor */
  // default ctor;
  vector(void) : mem_head_(nullptr), data_tail_(nullptr), mem_tail_(nullptr) {};

  // ctor;
  explicit vector(size_type n, const value_type& value = value_type()) {
    // allocate memory;
    mem_head_ = data_allocator::allocate(n);
    try {
      // initialize memory;
      uninitialized_fill_n(mem_head_, n, value);
    }
    catch (const std::exception& e) {
      // commit or rollback semantics;
      destroy(mem_head_, mem_head_ + n);
      data_allocator::deallocate(mem_head_, n);
      #ifdef LEM_DEBUG
        cout << "\tLEM_DEBUG: " << e.what() << endl;
      #endif
      throw e;
    }

    // set memory tags;
    data_tail_ = mem_head_ + n;
    mem_tail_ = data_tail_;
  }

  // copy ctor;
  // Copy ctor is called when an object is initialized to an existed object of the same type;
  /* vector(const vector<DataType, AllocType>&) {} */ // deep copy for specific DataType;

  // assignment ctor;
  // deep copy for specific DataType;
  /* vector<DataType, AllocType> operator=(vector<DataType, AllocType>) {} */

  /* end ctor */

  /* dtor */
  ~vector(void) {
    #ifdef LEM_DEBUG
      cout << "\tLEM_DEBUG: Call ~vector(). " << endl;
    #endif
    destroy(mem_head_, data_tail_); // uninitialized memory need not to be destroyed;
    if (mem_head_ != nullptr) {
      data_allocator::deallocate(mem_head_, mem_tail_ - mem_head_);
    }
  }
  /* end dtor */

  /* iterators */
  iterator begin(void) const noexcept {
    return mem_head_;
  }
  iterator end(void) const noexcept {
    return data_tail_;
  }
  /* end iterators */

  /* state tags */
  bool empty(void) const noexcept {
    return begin() == end();
  }
  size_type size(void) const noexcept {
    return (size_type)(end() - begin());
  }
  size_type capacity(void) const noexcept {
    return (size_type)(mem_tail_ - begin());
  }
  /* end state tags */

  /* accessors */
  reference_type at(size_type ind) const {
    if (ind >= size()) {
      throw std::out_of_range("Invalid vector subscript. ");
    }

    return *(begin() + ind);
  }
  reference_type operator[](size_type ind) const noexcept {
    return *(begin() + ind);
  }
  reference_type front(void) noexcept {
    return *(begin());
  }
  reference_type back(void) noexcept {
    return *(end() - 1);
  }
  /* end accessors */

  /* modifiers */
  void push_back(const value_type& value) {
    if (data_tail_ != mem_tail_) { // memory available;
      construct(end(), value);
      ++data_tail_;

      return;
    }

    // Now capacity full;
    // reallocate memory;
    size_type prev_size = size();
    size_type new_size = (prev_size == 0 ? 1 : 2 * prev_size);
    iterator new_mem_head = data_allocator::allocate(new_size);
    iterator new_data_tail = new_mem_head;
    try {
      // copy data;
      new_data_tail = uninitialized_copy(mem_head_, mem_tail_, new_mem_head);
      // insert new data;
      construct(new_data_tail, value);
      ++new_data_tail;
    }
    catch (const std::exception& e) {
      // commit or rollback semantics;
      destroy(new_mem_head, new_data_tail);
      data_allocator::deallocate(new_mem_head, new_size);
      // throw out;
      throw e;
    }

    // delete prev vector;
    destroy(begin(), end());
    data_allocator::deallocate(mem_head_, prev_size);

    // update memory tags;
    mem_head_ = new_mem_head;
    mem_tail_ = new_mem_head + new_size;
    data_tail_ = new_data_tail;
  }
  void pop_back(void) {
    if (empty()) {
      throw lem::pop_empty_vector();
    }

    // Now non-empty vector;
    --data_tail_;
    destroy(data_tail_);

    return;
  }
  void resize(size_type n) {


    return;
  }
  // erase() returns the iterator following the last removed element;
  iterator erase(iterator iter) {
    // erasing an empty range is an no-op;
    if (iter == end()) {
      return iter;
    }

    // if not erasing ending element;
    if (iter + 1 != end()) { // shift elements after iter;
      copy(iter + 1, end(), iter);
    }

    // update memory tags;
    --data_tail_;
    // destroy duplicant;
    destroy(data_tail_);

    return iter;
  }
  iterator erase(iterator head, iterator tail) {
    // erasing an empty range is an no-op;
    if (head > tail) { // This will produce duplicate elements after copy();
      #ifdef LEM_DEBUG
        throw lem::illegal_iterval();
      #else
        return tail;
      #endif
    }

    // shift elements after tail;
    iterator new_data_tail = copy(tail, end(), head);
    // destroy duplicants;
    destroy(new_data_tail, end());
    // update memory tags;
    data_tail_ = new_data_tail;

    return head;
  }
  void clear(void) {
    erase(begin(), end());
  }
  /* end modifiers */

  /* capacity functions */
  void reserve(size_type req) {


    return;
  }
  void shrink_to_fit(void) {


    return;
  }
  /* end capacity */
};

/* vector __type_traits */
template <typename DataType, typename AllocType>
struct __type_traits<vector<DataType, AllocType>> {
  using has_trivial_default_ctor = __false_tag;
  using has_trivial_copy_ctor = __false_tag;
  using has_trivial_assignment_oprtr = __false_tag;
  using has_trivial_dtor = __false_tag;
  using is_POD_type = __false_tag;
};
/* end __type_traits */
} /* end lem */

#endif
