// Iterators and itrator traits;
#ifndef LEMSTL_LEM_ITERATOR_H_
#define LEMSTL_LEM_ITERATOR_H_

#include <cstddef> // for ::std::ptrdiff_t;

namespace lem {
// Iterator category tags;
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : input_iterator_tag {};
struct bidirectional_iterator_tag : forward_iterator_tag {};
struct random_access_iterator_tag : bidirectional_iterator_tag {};

// lem::iterator type;
// Every container should specify its own iterator;
// EM NOTE: c++ struct expect public scope for undeclared members,
// while class expect private by default.
template <typename Category,
          typename T,
          typename DifferenceType = ptrdiff_t,
          typename PointerType = T*,
          typename ReferenceType = T&
          >
struct iterator {
  using iterator_category = Category;
  using value_type = T;
  using difference_type = DifferenceType;
  using pointer_type = PointerType;
  using reference_type = ReferenceType;
};

// lem::iterator_traits;
template <typename Iter>
struct iterator_traits {
  using iterator_category = typename Iter::iterator_category;
  using value_type = typename Iter::value_type;
  using difference_type = typename Iter::difference_type;
  using pointer_type = typename Iter::pointer_type;
  using reference_type = typename Iter::reference_type;
};
// partial specialization for native pointers;
template <typename T>
struct iterator_traits<T*> {
  using iterator_category = random_access_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer_type = T*;
  using reference_type = T&;
};
// partial specialization for native const pointers;
template <typename T>
struct iterator_traits<T const*> {
  using iterator_category = random_access_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer_type = T const*;
  using reference_type = T const&;
};

/* Functions to fetch nullptr to iterator_traits objects */
template <typename Iter>
inline typename iterator_traits<Iter>::value_type*
get_value_type(Iter const&) {
  return static_cast<typename iterator_traits<Iter>::value_type*>(nullptr);
}
template <typename Iter>
inline typename iterator_traits<Iter>::difference_type*
get_difference_type(Iter const&) {
  return static_cast<typename iterator_traits<Iter>::difference_type*>(nullptr);
}
template <typename Iter>
inline typename iterator_traits<Iter>::iterator_category
get_iterator_category(Iter const&) {
  // iterator category struct is simple enough.
  // The getter need not return pointer and instead returns an object.
  return typename iterator_traits<Iter>::iterator_category();
}
/* end functions to fetch nullptr */

/* distance */
template <typename InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
__distance(InputIterator head, InputIterator tail, ::lem::input_iterator_tag) {
  typename iterator_traits<InputIterator>::difference_type dist = 0;
  while (head != tail) {
    ++dist;
    ++head;
  }

  return dist;
}
template <typename RandomAccessIterator>
inline typename iterator_traits<RandomAccessIterator>::difference_type
__distance(RandomAccessIterator head, RandomAccessIterator tail, ::lem::random_access_iterator_tag) {
  return tail - head;
}
template <typename InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
distance(InputIterator head, InputIterator tail) {
  return ::lem::__distance(head, tail, ::lem::get_iterator_category(head));
}
/* end distance */

/* advance */
template <typename InputIterator, typename Distance>
inline void __advance(InputIterator& iter, Distance dist, ::lem::input_iterator_tag) {
  for (; dist != 0; --dist) {
    ++iter;
  }
}
template <typename BidirectionalIterator, typename Distance>
inline void __advance(BidirectionalIterator& iter, Distance dist, ::lem::bidirectional_iterator_tag) {
  if (dist >= 0) {
    for (; dist != 0; --dist) {
      ++iter;
    }
  }
  else {
    for (; dist != 0; ++dist) {
      --iter;
    }
  }
}
template <typename RandomAccessIterator, typename Distance>
inline void __advance(RandomAccessIterator& iter, Distance dist, ::lem::random_access_iterator_tag) {
  iter += dist;
}
template <typename InputIterator, typename Distance>
inline void advance(InputIterator& iter, Distance dist = 1) {
  __advance(iter, dist, ::lem::get_iterator_category(iter));
}
/* end advance */
} /* end lem */

#endif
