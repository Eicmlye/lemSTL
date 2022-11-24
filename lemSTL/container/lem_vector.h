// Sequential containers;
#ifndef LEMSTL_LEM_VECTOR_H_
#define LEMSTL_LEM_VECTOR_H_

#ifdef LEM_DEBUG
  #include <iostream> // for cout, endl;
  #include <cstdlib> // for exit();

  using std::cout;
  using std::endl;
#endif

#include <cstddef> // for std::ptrdiff_t;
#include <initializer_list> // for std::initializer_list

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

  // Data type;
  using size_type         = size_t;
  using value_type        = DataType;
  using difference_type   = ptrdiff_t;
  using pointer_type      = DataType*;
  using reference_type    = DataType&;

 protected:
  // memory allocation;
  using data_allocator    = ::lem::simple_alloc<value_type, allocator_type>;

  // EM NOTE:
  //            size
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
  // ##usage: vector<...> vct;
  vector(void) : mem_head_(nullptr), data_tail_(nullptr), mem_tail_(nullptr) {}

  // ctor;
  // ##usage: vector<...> vct({...});
  vector(::std::initializer_list<DataType> init_list) {
    // allocate memory;
    mem_head_ = data_allocator::allocate(init_list.size());
    try {
      ::lem::uninitialized_copy(init_list.begin(), init_list.end(), mem_head_);
    }
    catch (::std::exception const& e) {
      // commit or rollback semantics;
      ::lem::destroy(mem_head_, mem_head_ + init_list.size());
      data_allocator::deallocate(mem_head_, init_list.size());
      // throw out;
      throw e;
    }

    // set memory tags;
    data_tail_ = mem_head_ + init_list.size();
    mem_tail_ = data_tail_;
  }
  explicit vector(size_type n, value_type const& value = value_type()) {
    // allocate memory;
    mem_head_ = data_allocator::allocate(n);
    try {
      // initialize memory;
      ::lem::uninitialized_fill_n(mem_head_, n, value);
    }
    catch (std::exception const& e) {
      // commit or rollback semantics;
      ::lem::destroy(mem_head_, mem_head_ + n);
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
  // Notice that copy ctor is only used for initialization.
  // ##usage: vector<...> vct = ARGUMENT; 
  //  ARGUMENT is used to construct a temporary object K via ctor vector(ARGUMENT),
  //  and copy ctor is used to copy K to vct.
  // ##usage: vector<...> vct(SOME_VECTOR);
  // ##usage: vector<...> vct = SOME_VECTOR;
  //  Here SOME_VECTOR will not be considered as ARGUMENT for copy ctor to construct a temp object,
  //  instead vct is directly constructed by copy ctor which is exactly like vct(SOME_VECTOR).
  /* vector(vector<DataType, AllocType> const&) {} */ // deep copy for specific DataType;

  // assignment ctor;
  // deep copy for specific DataType;
  // Notice that assignment ctor is used to change data of initialized objects.
  // ##usage: vct = SOME_VECTOR;
  /* vector<DataType, AllocType> operator=(vector<DataType, AllocType>) {} */

  /* end ctor */

  /* dtor */
  ~vector(void) {
    #ifdef LEM_DEBUG
      cout << "\tLEM_DEBUG: Call ~vector(). " << endl;
    #endif
    ::lem::destroy(mem_head_, data_tail_); // uninitialized memory need not to be destroyed;
    if (mem_head_ != nullptr) {
      data_allocator::deallocate(mem_head_, mem_tail_ - mem_head_);
    }
  }
  /* end dtor */

  /* iterators */
  iterator begin(void) noexcept {
    return mem_head_;
  }
  iterator end(void) noexcept {
    return data_tail_;
  }
  /* end iterators */

  /* accessors */
  reference_type at(size_type ind) {
    if (ind >= size()) {
      throw std::out_of_range("Invalid vector subscript. ");
    }

    return *(begin() + ind);
  }
  reference_type operator[](size_type ind) noexcept {
    return *(begin() + ind);
  }
  reference_type front(void)  noexcept {
    return *(begin());
  }
  reference_type back(void) noexcept {
    return *(end() - 1);
  }
  /* end accessors */

  /* capacity functions */
  bool empty(void) noexcept {
    return begin() == end();
  }
  size_type size(void) noexcept {
    return (size_type)(end() - begin());
//    return (size_type)::lem::distance(begin(), end());
  }
  size_type capacity(void) noexcept {
    return (size_type)(mem_tail_ - begin());
  }
  // If req <= capacity, reserve() does nothing,
  // otherwise the vector is reallocated and iterators will therefore be invalid.
  void reserve(size_type req) {
    if (req <= capacity()) {
      return;
    }

    #ifdef LEM_DEBUG
      cout << "\tLEM_DEBUG: Call reserve(). " << endl;
    #endif
    // Now req > capacity();
    iterator new_mem_head = data_allocator::allocate(req);
    iterator new_data_tail = new_mem_head;
    try {
      // move data to newly allocated memory;
      new_data_tail = ::lem::uninitialized_copy(mem_head_, data_tail_, new_mem_head);
    }
    catch (std::exception const& e) {
      // if any construct() failed, new_data_tail would not be assigned to new value;
      ::lem::destroy(new_mem_head, new_mem_head + req);
      data_allocator::deallocate(new_mem_head, req);
      // throw out;
      throw e;
    }

    // delete prev vector;
    ::lem::destroy(begin(), end());
    data_allocator::deallocate(mem_head_, mem_tail_ - mem_head_);

    // update memory tags;
    mem_head_ = new_mem_head;
    data_tail_ = new_data_tail;
    mem_tail_ = mem_head_ + req;

    return;
  }
  void shrink_to_fit(void) {
    if (size() == capacity()) {
      return;
    }

    data_allocator::deallocate(data_tail_, mem_tail_ - data_tail_);
    mem_tail_ = data_tail_;

    return;
  }
  /* end capacity */

  /* modifiers */
  iterator insert(iterator pos_iter, value_type const& value, size_type n = 1) {
    if (n == 0) {
      return pos_iter;
    }

    // if capacity is large enough;
    if (n <= (size_type)(mem_tail_ - data_tail_)) {
      size_type num_after_pos = data_tail_ - pos_iter;
      iterator prev_data_tail = data_tail_;
      // EM NOTE: we should not leave uninitialized memory
      // between begin() and end() during insertation.
      if (num_after_pos >= n) {
        // deal with data in uninitialized memory;
        ::lem::uninitialized_copy(data_tail_ - n, data_tail_, data_tail_);
        data_tail_ += n;
        // move the rest part of existed data;
        ::lem::copy_backward(pos_iter, prev_data_tail - n, prev_data_tail);
        // fill new data;
        // EM QUESTION: is there any difference using fill() and fill_n() here?
        ::lem::fill_n(pos_iter, n, value);
      }
      else {
        // deal with data in uninitialized memory;
        ::lem::uninitialized_fill_n(data_tail_, n - num_after_pos, value);
        data_tail_ += n - num_after_pos;
        // move existed data;
        ::lem::uninitialized_copy(pos_iter, prev_data_tail, data_tail_);
        data_tail_ += num_after_pos;
        // fill the rest part of new data;
        // EM QUESTION: is there any difference using fill() and fill_n() here?
        ::lem::fill_n(pos_iter, num_after_pos, value);
      }
    }
    // Now capacity not enough;
    else {
      size_type prev_size = size();
      size_type new_size = prev_size + max(prev_size, n);
      iterator new_mem_head = data_allocator::allocate(prev_size + n);
      iterator new_data_tail = new_mem_head;

      try {
        new_data_tail = ::lem::uninitialized_copy(mem_head_, pos_iter, new_mem_head);
        new_data_tail = ::lem::uninitialized_fill_n(new_data_tail, n, value);
        new_data_tail = ::lem::uninitialized_copy(pos_iter, data_tail_, new_data_tail);
      }
      catch (::std::exception const& e) {
        // commit or rollback semantics;
        ::lem::destroy(new_mem_head, new_mem_head + new_size);
        data_allocator::deallocate(new_mem_head, new_size);
        // throw out;
        throw e;
      }

      // delete prev vector;
      ::lem::destroy(mem_head_, data_tail_);
      data_allocator::deallocate(mem_head_, prev_size);

      // update memory tags;
      mem_head_ = new_mem_head;
      data_tail_ = new_data_tail;
      mem_tail_ = mem_head_ + new_size;
    }

    return pos_iter;
  }
  void push_back(const value_type& value) {
    if (data_tail_ != mem_tail_) { // memory available;
      ::lem::construct(end(), value);
      ++data_tail_;

      return;
    }

    // Now capacity full;
    // reallocate memory;
    /* EM NOTE */
    // here we should not use reserve(),
    // because if construct() fails,
    // the reallocation done by reserve() cannot be rolled back,
    // and iterators will be invalid, which breaks the principle of c/r.
    size_type prev_size = size();
    size_type new_size = (prev_size == 0 ? 1 : 2 * prev_size);
    iterator new_mem_head = data_allocator::allocate(new_size);
    iterator new_data_tail = new_mem_head;
    try {
      // copy data;
      new_data_tail = ::lem::uninitialized_copy(mem_head_, mem_tail_, new_mem_head);
      // insert new data;
      ::lem::construct(new_data_tail, value);
      ++new_data_tail;
    }
    catch (std::exception const& e) {
      // commit or rollback semantics;
      ::lem::destroy(new_mem_head, new_data_tail);
      data_allocator::deallocate(new_mem_head, new_size);
      // throw out;
      throw e;
    }

    // delete prev vector;
    ::lem::destroy(begin(), end());
    data_allocator::deallocate(mem_head_, prev_size);

    // update memory tags;
    mem_head_ = new_mem_head;
    mem_tail_ = new_mem_head + new_size;
    data_tail_ = new_data_tail;

    return;
  }
  void pop_back(void) {
    if (empty()) {
      throw lem::pop_empty_vector();
    }

    // Now non-empty vector;
    --data_tail_;
    ::lem::destroy(data_tail_);

    return;
  }
  // resize() only changes capacity when new size is greater than current capacity;
  void resize(size_type n, value_type const& value = value_type()) {
    if (n == size()) {
      return;
    }

    if (n < size()) {
      ::lem::destroy(mem_head_ + n, data_tail_);
      data_tail_ = mem_head_ + n;

      return;
    }

    // Now n > size();
    // check whether to extend capacity;
    if (n <= capacity()) {
      try {
        ::lem::uninitialized_fill_n(data_tail_, n - size(), value);
      }
      catch (std::exception const& e) {
        // commit or rollback semantics;
        ::lem::destroy(data_tail_, data_tail_ + n - size());
        // throw out;
        throw e;
      }

      // update memory tags;
      data_tail_ = mem_head_ + n;

      return;
    }

    // Now n > capacity();
    // EM NOTE: here we should not use reserve(),
    // because if uninitialized_fill_n() fails, 
    // the reallocation done by reserve() cannot be rolled back.
    iterator new_mem_head = data_allocator::allocate(n);
    iterator new_data_tail = new_mem_head;

    try {
      // copy data;
      new_data_tail = ::lem::uninitialized_copy(mem_head_, data_tail_, new_mem_head);
      // insert value;
      new_data_tail = ::lem::uninitialized_fill_n(new_data_tail, n - size(), value);
    }
    catch (std::exception const& e) {
      // commit or rollback semantics;
      ::lem::destroy(new_mem_head, new_data_tail);
      data_allocator::deallocate(new_mem_head, n);
      // throw out;
      throw e;
    }

    // delete prev vector;
    ::lem::destroy(begin(), end());
    data_allocator::deallocate(mem_head_, mem_tail_ - mem_head_);

    // update memory tags;
    mem_head_ = new_mem_head;
    data_tail_ = new_data_tail;
    mem_tail_ = mem_head_ + n;

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
    ::lem::destroy(data_tail_);

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
    iterator new_data_tail = ::lem::copy(tail, end(), head);
    // destroy duplicants;
    ::lem::destroy(new_data_tail, end());
    // update memory tags;
    data_tail_ = new_data_tail;

    return head;
  }
  void clear(void) {
    erase(begin(), end());
  }
  /* end modifiers */
};

/* vector __type_traits */
template <typename DataType, typename AllocType>
struct __type_traits<vector<DataType, AllocType>> {
  using has_trivial_default_ctor = ::lem::__false_tag;
  using has_trivial_copy_ctor = ::lem::__false_tag;
  using has_trivial_assignment_oprtr = ::lem::__false_tag;
  using has_trivial_dtor = ::lem::__false_tag;
  using is_POD_type = ::lem::__false_tag;
};
/* end __type_traits */
} /* end lem */

#endif
