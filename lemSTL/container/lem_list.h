// Linked list containers (coubly-linked);
#ifndef LEMSTL_LEM_LIST_H_
#define LEMSTL_LEM_LIST_H_

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
// The destroy() work has NOT been done by uninitialized-xxx().
// If exception occurs, you should destroy all the objects in a single round.
//
// You should be clear that the above block will only catch exceptions in construct(),
// i.e. in the ctor(). And you should throw the exception out to the user function
// to let the user know this construction failure. 

namespace lem {
// See declarations at https://en.cppreference.com/w/cpp/container/list;
template <typename DataType>
struct __list_node {
  DataType data_;
  __list_node<DataType>* pred_ = nullptr;
  __list_node<DataType>* next_ = nullptr;
};
} /* end lem */

template <typename DataType, typename PointerType, typename ReferenceType>
struct __list_iterator {
  using iterator_category = ::lem::bidirectional_iterator_tag;
  using value_type = DataType;
  using difference_type = ptrdiff_t;
  using pointer_type = PointerType;
  using reference_type = ReferenceType;

  using self = __list_iterator<value_type, pointer_type, reference_type>;
  using iterator = __list_iterator<value_type, value_type*, value_type&>;
  using node_pointer = ::lem::__list_node<value_type>*;

  node_pointer node_;

  // ctor;
  __list_iterator(void) : node_(nullptr) {}
  __list_iterator(node_pointer node) : node_(node) {}
  __list_iterator(iterator const& iter) : node_(iter.node_) {}

  // equality;
  bool operator==(self const& iter) {
    return node_ == iter.node_;
  }
  bool operator!=(self const& iter) {
    return node_ != iter.node_;
  }

  // dereference;
  reference_type operator*(void) {
    return node_->data_;
  }

  // member access;
  // iter-> is equivalent to (*iter)., which allows you add member directly after the arrow;
  // operator-> must return an object with operator-> defined (native pointers are allowed); 
  // See https://stackoverflow.com/questions/8777845 for this special semantics;
  pointer_type operator->(void) const {
    // standard definition;
    return &(operator*());
  }

  // incre and decre;
  /* EM NOTE: list is defined as a cyclic linkedlist, so in/de-cre will never overflow */
  // prefix incre;
  self& operator++(void) {
    node_ = node_->next_;
    return *this;
  }
  // postfix incre;
  self operator++(int) {
    self cache = *this;
    ++(*this);
    return cache;
  }
  // prefix decre;
  self& operator--(void) {
    node_ = node_->pred_;
    return *this;
  }
  // postfix decre;
  self operator--(int) {
    self cache = *this;
    --(*this);
    return cache;
  }
};

/* SGI STL implemented list as a cyclic doubly-linked list with empty header node */
template <typename DataType, typename AllocType = ::lem::alloc>
class list {
 public:
  using allocator_type      = AllocType;

  // iterator traits;
  using iterator            = __list_iterator<DataType, DataType*, DataType&>;
  using reverse_iterator    = __list_iterator<DataType, DataType*, DataType&>;

  // data type;
  using size_type           = size_t;
  using value_type          = DataType;
  using difference_type     = ptrdiff_t;
  using pointer_type        = DataType*;
  using reference_type      = DataType&;
  using node_type           = ::lem::__list_node<value_type>;
  using node_pointer           = ::lem::__list_node<value_type>*;

 protected:
  using list_node_allocator = ::lem::simple_alloc<node_type, allocator_type>;

  // data;
  node_pointer head_; // pointer to empty header node;

  /* ctor */
  // default ctor;
  list(void) {
    head_ = list_node_allocator::allocate();
    head_->next_ = head_;
    head_->pred_ = head_;
  }

  /* end ctor */
};
#endif