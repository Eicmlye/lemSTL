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

  void set_node(map_pointer new_node) {
    map_node_ = new_node;
    head_ = *new_node;
    tail_ = *new_node + (difference_type)buffer_size();

    return;
  }
 
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
      set_node(map_node_ + 1);
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
      set_node(map_node_ - 1);
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
      set_node(map_node_ + node_offset);
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
  bool operator<=(self const& other) const { // iterator index comparison;
    return *this < other || *this == other;
  }
  bool operator>(self const& other) const { // iterator index comparison;
    return !(*this <= other);
  }
  bool operator>=(self const& other) const { // iterator index comparison;
    return !(*this < other);
  }
};

// See declarations at https://en.cppreference.com/w/cpp/container/deque;
template <typename DataType, typename AllocType = ::lem::alloc, size_t BufSiz = 0>
class deque {
  /* EM NOTE */
  // The essence of SGI deque is indirect addressing.
  // The central manager is responsible for an array of pointers (map_)
  // to allocated memory blocks (called buffers) in which the data are actually stored.

 public:
  using allocator_type      = AllocType;

  using iterator            = __deque_iterator<DataType, DataType*, DataType&, BufSiz>;
  using reverse_iterator    = __deque_iterator<DataType, DataType*, DataType&, BufSiz>;

  using size_type           = size_t;
  using value_type          = DataType;
  using diff_type           = ptrdiff_t;
  using ptr_type            = DataType*;
  using ref_type            = DataType&;

 protected:
  using map_pointer         = ptr_type*;

  map_pointer map_; // header pointer to pointers to a buffer;
  size_type map_size_; // number of pointers in map_;
  iterator head_;
  iterator tail_;

  using map_allocator = ::lem::simple_alloc<ptr_type, allocator_type>;
  using data_allocator = ::lem::simple_alloc<value_type, allocator_type>;

  // build map structure;
  void create_deque_structure(size_type req_elem_num) {
    constexpr size_type kMinNodeNum = 8;
    size_type kBufSiz = iterator::buffer_size();

    // number of pointers needed in map_;
    // If req_elem_num % kBufSiz == 0, an extra node is added,
    // and this makes the iterator tail_
    // always point to an actual buffer node.
    size_type map_node_num = req_elem_num / kBufSiz + 1;

    // Add two extra nodes at the beginning and the end
    // for the convenience of expanding map_;
    map_size_ = ::lem::max(kMinNodeNum, map_node_num + 2);
    map_ = map_allocator::allocate(map_size_);

    // set data blocks to the middle of allocated map;
    map_pointer new_head = map_ + (map_size_ - map_node_num) / 2;
    map_pointer new_tail = new_head + map_node_num - 1;
    map_pointer new_cur = new_head;
    // build memory block for initial data;
    try {
      for (; new_cur <= new_tail; ++new_cur) {
        *new_cur = data_allocator::allocate(kBufSiz);
      }
    }
    catch (::std::exception const& e) {
      // commit or rollback semantics;
      for (; new_cur >= new_head; --new_cur) {
        data_allocator::deallocate(*new_cur, kBufSiz);
      }

      // throw out; 
      throw e;
    }

    head_.set_node(new_head);
    head_.cur_ = head_.head_;
    tail_.set_node(new_tail);
    tail_.cur_ = tail_.head_ + req_elem_num % kBufSiz;

    return;
  }

 public:
  /* ctor */
  // default ctor;
  // ##usage: list<...> lst;
  deque(void) :
    map_(nullptr),
    map_size_(0),
    head_(),
    tail_()
  {

  }

  // ctor;
  // ##usage: list<...> lst({...});
  deque(::std::initializer_list<value_type> init_list) {
    create_deque_structure(init_list.size());

    // set data;
    size_type kBufSiz = iterator::buffer_size();
    // See https://stackoverflow.com/questions/71104242
    // and https://stackoverflow.com/questions/610245
    // for why typename is necessary here.
    // Here value_type is a template argument,
    // and therefore this type name is a dependent name.
    /* potential error (when typename is omitted):
    error C3878: syntax error: unexpected token 'identifier' following 'expression' */
    typename ::std::initializer_list<value_type>::iterator iter = init_list.begin();
    map_pointer cur = head_.map_node_;
    try {
      for (; cur < tail_.map_node_; ::lem::advance(iter, kBufSiz), ++cur) {
        ::lem::uninitialized_copy(iter, iter + kBufSiz, *cur);
      }
      ::lem::uninitialized_copy(iter, init_list.end(), *cur);
    }
    catch (::std::exception const& e) {
      // commit or rollback semantics;
      for (cur = head_.map_node_; cur < tail_.map_node_; ++cur) {
        ::lem::destroy(*cur, *cur + iterator::buffer_size());
      }
      ::lem::destroy(tail_.head_, tail_.cur_); // the last map node;

      // throw out;
      throw e;
    }
  }
  explicit deque(size_type n, value_type const& value) :
    map_(nullptr),
    map_size_(0),
    head_(),
    tail_()
  {
    size_type kBufSiz = iterator::buffer_size();
    create_deque_structure(n);

    // set data;
    map_pointer cur = head_.map_node_;
    try {
      for (; cur < tail_.map_node_; ++cur) {
        ::lem::uninitialized_fill(*cur, *cur + kBufSiz, value);
      }
      ::lem::uninitialized_fill(tail_.head_, tail_.cur_, value); // the last map node;
    }
    catch (::std::exception const& e) {
      // commit or rollback semantics;
      for (cur = head_.map_node_; cur < tail_.map_node_; ++cur) {
        ::lem::destroy(*cur, *cur + iterator::buffer_size());
      }
      ::lem::destroy(tail_.head_, tail_.cur_); // the last map node;

      // throw out;
      throw e;
    }
  }

  // copy ctor;
  // Copy ctor is called when an object is initialized to an existed object of the same type;
  // Notice that copy ctor is only used for initialization.
  // ##usage: deque<...> dq = ARGUMENT; 
  //  ARGUMENT is used to construct a temporary object K via ctor deque(ARGUMENT),
  //  and copy ctor is used to copy K to dq.
  // ##usage: deque<...> dq(SOME_DEQUE);
  // ##usage: deque<...> dq = SOME_DEQUE;
  //  Here SOME_DEQUE will not be considered as ARGUMENT for copy ctor to construct a temp object,
  //  instead dq is directly constructed by copy ctor which is exactly like vct(SOME_DEQUE).
  /* deque(deque<DataType, AllocType> const&) {} */ // deep copy for specific DataType;

  // assignment ctor;
  // deep copy for specific DataType;
  // Notice that assignment ctor is used to change data of initialized objects.
  // ##usage: dq = SOME_DEQUE;
  /* deque<DataType, AllocType> operator=(deque<DataType, AllocType>) {} */

  /* end ctor */

  /* iterators */
  iterator begin(void) const noexcept {
    return head_;
  }
  iterator end(void) const noexcept {
    return tail_;
  }
  /* end itertors */

  /* accessors */
  ref_type at(size_type ind) {
    if ((tail_ - head_) * iterator::buffer_size() + tail_.tail_ - tail_.head_ <= diff_type(ind)) {
      throw ::std::out_of_range("Invalid deque subscript. ");
    }

    return head_[diff_type(ind)];
  }
  ref_type operator[](size_type ind) noexcept {
    return head_[diff_type(ind)];
  }
  ref_type front(void) noexcept {
    return *head_;
  }
  ref_type back(void) noexcept {
    return *(tail_ - 1);
  }
  /* end accessors */

  /* capacity functions */
  bool empty(void) const noexcept {
    return head_ == tail_;
  }
  size_type size(void) const noexcept {
    return tail_ - head_;
  }
  /* end capacity functions */
};

/* deque __type_traits */
template <typename DataType, typename AllocType>
struct __type_traits<deque<DataType, AllocType>> {
  using has_trivial_default_ctor = ::lem::__false_tag;
  using has_trivial_copy_ctor = ::lem::__false_tag;
  using has_trivial_assignment_oprtr = ::lem::__false_tag;
  using has_trivial_dtor = ::lem::__false_tag;
  using is_POD_type = ::lem::__false_tag;
};
/* end __type_traits */
} /* end lem */

#endif