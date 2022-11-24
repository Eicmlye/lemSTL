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
template <typename DataType>
struct __list_node {
  DataType data_;
  __list_node<DataType>* pred_ = nullptr;
  __list_node<DataType>* next_ = nullptr;
};

template <typename DataType, typename PointerType, typename ReferenceType>
struct __list_iterator {
  using iterator_category     = ::lem::bidirectional_iterator_tag;
  using value_type            = DataType;
  using difference_type       = ptrdiff_t;
  using pointer_type          = PointerType;
  using reference_type        = ReferenceType;

  using self                  = __list_iterator<value_type, pointer_type, reference_type>;
  using iterator              = __list_iterator<value_type, value_type*, value_type&>;
  using node_pointer          = __list_node<value_type>*;

  // data;
  node_pointer node_;

  // ctor;
  __list_iterator(void) : node_(nullptr) {}
  __list_iterator(node_pointer node) : node_(node) {}
  
  __list_iterator(iterator const& iter) : node_(iter.node_) {
    #if defined LEM_DEBUG && 0
      ::std::cout << "Call list iterator copy ctor. " << ::std::endl;
    #endif
  }

  // equality;
  bool operator==(self const& iter) const {
    return node_ == iter.node_;
  }
  bool operator!=(self const& iter) const {
    return node_ != iter.node_;
  }

  // dereference;
  reference_type operator*(void) const {
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
// See declarations at https://en.cppreference.com/w/cpp/container/list;
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
  using node_pointer        = ::lem::__list_node<value_type>*;

 protected:
  using list_node_allocator = ::lem::simple_alloc<node_type, allocator_type>;

  // data;
  /* EM NOTE: data structure */
  //      -----------------------------------------------
  //      |                                             |
  //      v                                             |
  // ---------     ---------     -------     ---------  |
  // | head_ | --> | elem1 | --> | ... | --> | elemk | --
  // ---------     ---------     -------     ---------
  //     ^             ^
  //     |             |
  //    end()       begin()
  node_pointer head_; // pointer to empty header node;

 public:
  /* ctor */
  // default ctor;
  // ##usage: list<...> lst;
  list(void) {
    #ifdef LEM_DEBUG
      ::std::cout << "Call list default ctor. " << ::std::endl;
    #endif

    head_ = list_node_allocator::allocate();
    head_->next_ = head_;
    head_->pred_ = head_;
  }

  // ctor;
  // ##usage: list<...> lst({...});
  list(::std::initializer_list<value_type> init_list) {
    #ifdef LEM_DEBUG
      ::std::cout << "Call list init-list ctor. " << ::std::endl;
    #endif

    // set header node;
    head_ = list_node_allocator::allocate();
    head_->next_ = head_;
    head_->pred_ = head_;

    try {
      // build data;
      // See https://stackoverflow.com/questions/71104242
      // and https://stackoverflow.com/questions/610245
      // for why typename is necessary here.
      // Here value_type is a template argument,
      // and therefore this type name is a dependent name.
      /* potential error (when typename is omitted): 
      error C3878: syntax error: unexpected token 'identifier' following 'expression' */
      typename ::std::initializer_list<value_type>::iterator iter = init_list.begin();

      for (; iter != init_list.end(); ++iter) {
        // allocate memory;
        node_pointer newNode = list_node_allocator::allocate();

        // construct data;
        ::lem::construct(&(newNode->data_), *iter);

        // link to list;
        newNode->next_ = head_;
        newNode->pred_ = head_->pred_;
        head_->pred_->next_ = newNode;
        head_->pred_ = newNode;
      }
    }
    catch (::std::exception const& e) {
      // commit or rollback semantics;
      // destroy data;
      ::lem::destroy(iterator(head_->next_), iterator(head_));
      for (; head_->pred_ != head_;) {
        node_pointer cur = head_->pred_;

        cur->pred_->next_ = head_;
        head_->pred_ = cur->pred_;

        // deallocate memory;
        list_node_allocator::deallocate(cur);
      }
      // throw out;
      throw e;
    }
  }

  // copy ctor;
  // Copy ctor is called when an object is initialized to an existed object of the same type;
  // Notice that copy ctor is only used for initialization.
  // ##usage: list<...> lst = ARGUMENT; 
  //  ARGUMENT is used to construct a temporary object K via ctor list(ARGUMENT),
  //  and copy ctor is used to copy K to lst.
  // ##usage: list<...> lst(SOME_LIST);
  // ##usage: list<...> lst = SOME_LIST;
  //  Here SOME_LIST will not be considered as ARGUMENT for copy ctor to construct a temp object,
  //  instead lst is directly constructed by copy ctor which is exactly like vct(SOME_LIST).
  /* list(list<DataType, AllocType> const&) {} */ // deep copy for specific DataType;

  // assignment ctor;
  // deep copy for specific DataType;
  // Notice that assignment ctor is used to change data of initialized objects.
  // ##usage: lst = SOME_LIST;
  /* list<DataType, AllocType> operator=(list<DataType, AllocType>) {} */

  /* end ctor */

  /* dtor */
  ~list(void) {
    #ifdef LEM_DEBUG
      ::std::cout << "Call list dtor. " << ::std::endl;
    #endif

    // destroy data;
    ::lem::destroy(iterator(head_->next_), iterator(head_));
    // free memory;
    while (head_->pred_ != head_) {
      node_pointer cur = head_->pred_;

      cur->pred_->next_ = head_;
      head_->pred_ = cur->pred_;

      // deallocate memory;
      list_node_allocator::deallocate(cur);
    }

    // deallocate header;
    list_node_allocator::deallocate(head_);
  }
  /* end dtor */

  /* iterators */
  iterator begin(void) noexcept {
    return iterator(head_->next_);
  }
  iterator end(void) noexcept {
    return iterator(head_);
  }
  /* edn iterators */

  /* accessors */
  reference_type front(void) noexcept {
    return *begin();
  }
  reference_type back(void) noexcept {
    return *(--end());
  }
  /* end accessors */

  /* capacity */
  bool empty(void) noexcept {
    return (begin() == end());
  }
  size_type size(void) noexcept {
    return ::lem::distance(begin(), end());
  }
  /* end capacity */

  /* modifiers */
  iterator insert(iterator iter, const value_type& value) {
    // build new node;
    node_pointer newNode = list_node_allocator::allocate();
    ::lem::construct(&(newNode->data_), value);

    // insert to list;
    newNode->pred_ = iter.node_->pred_;
    newNode->next_ = iter.node_;
    iter.node_->pred_->next_ = newNode;
    iter.node_->pred_ = newNode;

    return newNode; // build iterator via __list_node;
  }
  void push_back(const value_type& value) {
    insert(end(), value);

    return;
  }
  void push_front(const value_type& value) {
    insert(begin(), value);

    return;
  }

  iterator erase(iterator iter) {
    // protect list header;
    if (iter == end()) {
      throw ::lem::del_header();
    }

    // pop iter;
    node_pointer pred = iter.node_->pred_;
    node_pointer next = iter.node_->next_;

    pred->next_ = next; 
    next->pred_ = pred;

    // destroy iter;
    ::lem::destroy(&(iter.node_->data_));
    // free memory;
    list_node_allocator::deallocate(iter.node_);

    return next;
  }
  void pop_front(void) {
    erase(begin());

    return;
  }
  void pop_back(void) {
    erase(--end());

    return;
  }
  size_type remove(const value_type& value) {
    size_type count = 0;
    iterator iter = begin();

    while (iter != end()) {
      if (*iter == value) {
        iter = erase(iter);
        ++count;
      }
      else {
        ++iter;
      }
    }

    return count;
  }

  void clear(void) {
    // destroy data;
    ::lem::destroy(begin(), end());

    // free memory;
    while (head_->pred_ != head_) {
      node_pointer cur = head_->pred_;

      cur->pred_->next_ = head_;
      head_->pred_ = cur->pred_;

      // deallocate memory;
      list_node_allocator::deallocate(cur);
    }

    return;
  }

  size_type unique(void) {
    if (empty()) {
      return 0;
    }

    size_type count = 0;
    iterator curFirst = begin();
    iterator mov = ++begin();

    while (mov != end()) {
      if (*mov == *curFirst) {
        mov = erase(mov);
        ++count;
      }
      else {
        curFirst = mov;
        ++mov;
      }
    }

    return count;
  }

 protected:
  // transfer [head, tail) to the front of iter.
  // EM NOTE: if iter.node_ == head.node_ == tail.node_->pred_, the behaviour is undefined.
  void transfer(iterator const iter, iterator const head, iterator const tail) {
    //                                        iter
    //                                         |
    //                                         v
    // ----      -------------------      ------------      ----
    // ...| <--> |iter.node_->pred_| <--> |iter.node_| <--> |...
    // ----      -------------------      ------------      ----
    // 
    // ----      -------------------      ------------      -----      -------------------      ------------
    // ...| <--> |head.node_->pred_| <--> |head.node_| <--> |...| <--> |tail.node_->pred_| <--> |tail.node_|
    // ----      -------------------      ------------      -----      -------------------      ------------
    //                                         ^                                                      ^
    //                                         |                                                      |
    //                                        head                                                   tail
    
    // deal with iterator predecessors;
    iter.node_->pred_->next_ = head.node_;
    tail.node_->pred_->next_ = iter.node_;
    head.node_->pred_->next_ = tail.node_;
    //                                        iter
    //                                         |
    //                                         v
    // ----      -------------------      ------------      ----
    // ...| <--> |iter.node_->pred_| <--  |iter.node_| <--> |...
    // ----      -------------------      ------------      ----
    //                            |          ^
    //                            ---------  |
    //                                    |  --------------------------------------------
    //                                    v                                             |
    // ----      -------------------      ------------      -----      -------------------      ------------
    // ...| <--> |head.node_->pred_| <--  |head.node_| <--> |...| <--> |tail.node_->pred_| <--  |tail.node_|
    // ----      -------------------      ------------      -----      -------------------      ------------
    //                             |           ^                                                ^     ^
    //                             |           |                                                |     |
    //                             |          head                                              |    tail
    //                             --------------------------------------------------------------
    
    node_pointer cache = iter.node_->pred_;
    //                  cache                 iter
    //                    |                    |
    //                    v                    v
    // ----      -------------------      ------------      ----
    // ...| <--> |iter.node_->pred_| <--  |iter.node_| <--> |...
    // ----      -------------------      ------------      ----
    //                            |          ^
    //                            ---------  |
    //                                    |  --------------------------------------------
    //                                    v                                             |
    // ----      -------------------      ------------      -----      -------------------      ------------
    // ...| <--> |head.node_->pred_| <--  |head.node_| <--> |...| <--> |tail.node_->pred_| <--  |tail.node_|
    // ----      -------------------      ------------      -----      -------------------      ------------
    //                             |           ^                                                ^     ^
    //                             |           |                                                |     |
    //                             |          head                                              |    tail
    //                             --------------------------------------------------------------

    // deal with iterators;
    iter.node_->pred_ = tail.node_->pred_;
    tail.node_->pred_ = head.node_->pred_;
    head.node_->pred_ = cache;
    //                  cache                 iter
    //                    |                    |
    //                    v                    v
    // ----      -------------------      ------------      ----
    // ...| <--> |iter.node_->pred_|      |iter.node_| <--> |...
    // ----      -------------------      ------------      ----
    //                            ^          ^
    //                            |          |
    //                            ---------  --------------------------------------------
    //                                    |                                             |
    //                                    v                                             v
    // ----      -------------------      ------------      -----      -------------------      ------------
    // ...| <--> |head.node_->pred_|      |head.node_| <--> |...| <--> |tail.node_->pred_|      |tail.node_|
    // ----      -------------------      ------------      -----      -------------------      ------------
    //                             ^           ^                                                ^     ^
    //                             |           |                                                |     |
    //                             |          head                                              |    tail
    //                             --------------------------------------------------------------

    return;
  }
 public:
  void splice(iterator const iter, list<DataType, AllocType>& lst) {
    if (this == &lst) {
      throw ::lem::self_splice();
    }
    if (lst.empty()) {
      return;
    }

    transfer(iter, lst.begin(), lst.end());

    return;
  }
  void splice(iterator const iter, list<DataType, AllocType>& lst, iterator const head) {
    #ifdef LEM_WARNING
      do {
        iterator mov = lst.begin();
        bool isIterOfLst = false;
        for (; mov != lst.end(); ++mov) {
          if (head == mov) {
            isIterOfLst = true;
            break;
          }
        }

        if (!isIterOfLst) {
          throw ::std::out_of_range("Iterator does not point to any element of given list. ");
        }
        if (head == lst.end()) {
          throw ::lem::mov_header();
        }
      } while (0);
    #endif

    iterator tail = head;
    ++tail;

    if (iter == head || iter == tail) {
      return;
      /* when iter == head, transfer(iter, head, tail) will make iter.node_->next_ == iter.node_ */
    }
    if (lst.empty()) {
      return;
    }

    transfer(iter, head, tail);

    return;
  }
  void splice(iterator const iter, list<DataType, AllocType>& lst, iterator const head, iterator const tail) {
    #ifdef LEM_WARNING
      if (this == &lst) {
        iterator mov = head;
        while (mov != tail) {
          if (mov == lst.head_) {
            throw ::lem::mov_header();
          }
          if (mov == iter) {
            throw ::lem::self_splice("Interval self-splicing. ");
          }
        }
      }
    #endif

    if (head == tail) {
      return;
    }

    transfer(iter, head, tail);

    return;
  }
  /* end modifiers */
};

/* list __type_traits */
template <typename DataType, typename AllocType>
struct __type_traits<list<DataType, AllocType>> {
  using has_trivial_default_ctor = ::lem::__false_tag;
  using has_trivial_copy_ctor = ::lem::__false_tag;
  using has_trivial_assignment_oprtr = ::lem::__false_tag;
  using has_trivial_dtor = ::lem::__false_tag;
  using is_POD_type = ::lem::__false_tag;
};
/* end __type_traits */
} /* end lem */
#endif