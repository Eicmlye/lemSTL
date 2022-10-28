// Sequential containers;
#ifndef LEMSTL_LEM_VECTOR_H_
#define LEMSTL_LEM_VECTOR_H_

#include <cstddef> // for ::std::ptrdiff_t;

#include "../lem_memory"
#include "../lem_iterator"

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

  //          data
  // <--------------------->
  //                  capacity
  // <--------------------------------------->
  // | 1 | 2 | 3 | ... | n |   |   | ... |   |xxxxxx
  //   |                     |                 |
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
    // initialize memory;
    uninitialized_fill_n(mem_head_, n, value);

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
    destroy(mem_head_, data_tail_); // uninitialized memory need not to be destroyed;
    if (mem_head_ != nullptr) {
      data_allocator::deallocate(mem_head_, mem_tail_ - mem_head_);
    }
  }
  /* end dtor */

  /* iterators */
  iterator begin(void) const {
    return mem_head_;
  }
  iterator end(void) const {
    return data_tail_;
  }
  /* end iterators */

  /* state tags */
  bool empty(void) const {
    return begin() == end();
  }
  size_type size(void) const {
    return (size_type)(end() - begin());
  }
  size_type capacity(void) const {
    return (size_type)(mem_tail_ - begin());
  }
  /* end state tags */

  /* accessors */
  reference_type at(size_type pos) {
    if (pos > size()) {
      throw std::out_of_range();
    }

    return *(begin() + pos);
  }
  reference_type operator[](size_type pos) {
    return *(begin() + pos);
  }
  reference_type front(void) {
    return *(begin());
  }
  reference_type back(void) {
    return *(end() - 1);
  }
  /* end accessors */

  /* member functions */
  void push_back(const value_type& value) {

  }
  void pop_back(void) {

  }
  /* end member functions */
};
} // lem

#endif
