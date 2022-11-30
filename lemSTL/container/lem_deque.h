#ifndef LEMSTL_LEM_DEQUE_H_
#define LEMSTL_LEM_DEQUE_H_

#ifdef LEM_DEBUG
#include <iostream> // for cout, endl;
#include <cstdlib> // for exit();

using std::cout;
using std::endl;
#endif

#include <cstddef> // for std::ptrdiff_t;
#include <initializer_list> // for std::initializer_list

#include "../lem_memory"
#include "../lem_iterator" // for distance();
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
// catch (std::exception const& e) {
//  /* commit or rollback semantics */
//  destroy(...); // unnecessary only if there is only ONE construct() used, and no unini_xxx() is used;
//  // data_allocator::deallocate(...); // only if you allocated new memory before try-catch;
//  throw e;
// }
// 
// The destroy() work has NOT been done by uninitialized_xxx().
// If exception occurs, you should destroy all the objects in a single round.
//
// You should be clear that the above block will only catch exceptions in construct(),
// i.e. in the ctor(). And you should throw the exception out to the user function
// to let the user know this construction failure. 

namespace lem {
static size_t __deque_buf_size(size_t bufsize, size_t datasize) {
  constexpr size_t kDefaultBufSize = 512;

  return (bufsize != 0) ? bufsize : (datasize <= kDefaultBufSize ? kDefaultBufSize / datasize : 1);
}

template <typename DataType, typename PointerType, typename ReferenceType, size_t BufSiz>
struct __deque_iterator {
  using iterator_category   = ::lem::random_access_iterator_tag;
  using value_type          = DataType;
  using difference_type     = ptrdiff_t;
  using pointer_type        = PointerType;
  using reference_type      = ReferenceType;

  using size_type           = size_t;

  using self                = __deque_iterator<DataType, PointerType, ReferenceType, BufSiz>;
  using iterator            = __deque_iterator<DataType, DataType*, DataType&, BufSiz>;
  using const_iterator      = __deque_iterator<DataType, DataType const*, DataType const&, BufSiz>;
  using map_pointer         = DataType**;

  map_pointer map_node_;
  value_type* cur_;
  value_type* head_; // the address that map_node_ points to;
  value_type* tail_; // head_ + (difference_type)buffer_size();

  static size_t buffer_size(void) {
    return __deque_buf_size(BufSiz, sizeof(value_type));
  }

  // dereference;
  reference_type operator*(void) const {
    return *cur_;
  }

  // member access;
  // iter-> is equivalent to (*iter)., which allows you to add member directly after the arrow;
  // operator-> must return an object with operator-> defined (native pointers are allowed); 
  // See https://stackoverflow.com/questions/8777845 for this special semantics;
  pointer_type operator->(void) const {
    return &(operator*());
  }

  // difference;
  difference_type operator-(self const& other) const {
    // When *this > other:
    // ---------------------------- ---------     --------- ----------------
    // |other.cur_ ... other.tail_| |buffer1| ... |bufferk| |head_ ... cur_|
    // ---------------------------- ---------     --------- ----------------
    //             ^                                                ^
    //             |                                                |
    //       other.map_node_                                    map_node_
    // 
    // *this - other = (other.tail_ - other.cur_) + (cur_ - head_)
    //                 + (map_node_ - other.map_node_ - 1) * (difference_type)buffer_size()
    // =====================
    // When *this < other:
    // ---------------- ---------     --------- ----------------------------
    // |cur_ ... tail_| |buffer1| ... |bufferk| |other.head_ ... other.cur_|
    // ---------------- ---------     --------- ----------------------------
    //             ^                                      ^
    //             |                                      |
    //         map_node_                           other.map_node_
    // 
    // *this - other = - (tail_ - cur_) - (other.cur_ - other.head_)
    //                 - (other.map_node_ - map_node_ - 1) * (difference_type)buffer_size()
    //               = (cur_ - head_) - (difference_type)buffer_size()
    //                 + (other.tail_ - other.cur_) - (difference_type)buffer_size()
    //                 + (map_node_ - other.map_node_ - 1) * (difference_type)buffer_size()
    //               = (other.tail_ - other.cur_) + (cur_ - head_)
    //                 + (map_node_ - other.map_node_ - 1) * (difference_type)buffer_size()

    return (other.tail_ - other.cur_) + (cur_ - head_) + (map_node_ - other.map_node_ - 1) * (difference_type)buffer_size();
  }

 protected:
  void __set_node(map_pointer new_node) {
    map_node_ = new_node;
    head_ = *new_node;
    tail_ = new_node + (difference_type)buffer_size();

    return;
  }
 public:
  /* EM NOTE */
  // The iterator itself does not care about
  // whether it has gone out of range of the object.
  // The range check and memory expansion
  // is left to the corresponding object to fulfill.

  // incre;
  self& operator++(void) {
    ++cur_;

    if (cur_ == tail_) { // if block overflow;
      // set iterator to next block;
      __set_node(map_node_ + 1);
      cur_ = head_;
    }

    return *this;
  }
  self operator++(int) {
    self cache = *this;
    ++(*this);

    return cache;
  }
  // decre;
  self& operator--(void) {
    if (cur_ == head_) {
      __set_node(map_node_ - 1);
      cur_ = tail_;
    }

    --cur_;

    return *this;
  }
  self operator--(int) {
    self cache = *this;
    --(*this);

    return cache;
  }

  // random access;
  self& operator+=(difference_type n) {
    difference_type offset = n + (cur_ - head_);

    if (offset >= 0 && offset < difference_type(buffer_size())) {
      // The iterator stays in the same block after movement.
      cur_ += n;
    }
    else { // The iterator moves to a new block;
      // Notice: one should try to avoid using negative numbers as dividend,
      // because different languages may use different rules for rounding.
      difference_type node_offset = offset > 0 ?\
        offset / difference_type(buffer_size()) :\
        -((-offset) / difference_type(buffer_size()) + 1);

      // move map_node_;
      __set_node(map_node_ + node_offset);
      // move cur_;
      cur_ = head_ + (-offset) % difference_type(buffer_size());
    }

    return *this;
  }
  self& operator-=(difference_type n) {
    return operator+=(-n);
  }
  self operator+(difference_type n) const {
    self cache = *this;

    return cache += n;
  }
  self operator-(difference_type n) const {
    self cache = *this;

    return cache -= n;
  }

  reference_type operator[](difference_type n) const {
    return *(*this + n);
  }

  // comparison operator;
  bool operator==(self const& other) const { // value equivalence;
    return cur_ == other.cur_;
  }
  bool operator!=(self const& other) const { // value equivalence;
    return cur_ != other.cur_;
  }
  bool operator<(self const& other) const { // iterator index comparison;
    return (map_node_ == other.map_node_) ? (cur_ < other.cur_) : (map_node_ < other.map_node_);
  }
};

// See declarations at https://en.cppreference.com/w/cpp/container/deque;
template <typename DataType, typename AllocType = ::lem::alloc, size_t BufSiz = 0>
class deque {
  /* EM NOTE */
  // The essence of SGI deque is indirect addressing.
  // The central manager is responsible for an array of pointers
  // to allocated memory blocks (called buffers) in which the data are actually stored.

 public:
  using allocater_type      = AllocType;

  using iterator            = __deque_iterator<DataType, DataType*, DataType&, BufSiz>;
  using reverse_iterator    = __deque_iterator<DataType, DataType*, DataType&, BufSiz>;

  using size_type           = size_t;
  using value_type          = DataType;
  using diff_type           = ptrdiff_t;
  using ptr_type            = DataType*;
  using ref_type            = DataType&;

 protected:
  using map_pointer         = ptr_type*;

  map_pointer map_; // head pointer to pointers to a buffer;
  size_type map_size_;
};
} /* end lem */

#endif