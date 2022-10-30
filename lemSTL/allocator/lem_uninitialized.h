// Copying objects in uninitialized memory.
#ifndef LEMSTL_LEM_UNINITIALIZED_H_
#define LEMSTL_LEM_UNINITIALIZED_H_

#include "../lem_iterator" // for value_type();
#include "../lem_type_traits" // for __type_traits;
#include "../lem_algorithm/lem_algobase.h" // for fill(), fill_n(), and copy();
#include "../lem_memory/lem_construct.h" // for construct();
#include "../lem_exception" // for std::exception;

/* uninitialized_fill() */
// fill elem to uninitialized allocated memory;
template <typename ForwardIterator, typename DataType, typename ValueType>
inline void __uninitialized_fill_aux(ForwardIterator head, ForwardIterator tail, const DataType& elem, __true_tag) {
  fill(head, tail, elem);
}
template <typename ForwardIterator, typename DataType, typename ValueType>
void __uninitialized_fill_aux(ForwardIterator head, ForwardIterator tail, const DataType& elem, __false_tag) {
  ForwardIterator cur = head;
  try {
    for (; cur != tail; ++cur) {
      construct(&*cur, elem);
    }
  }
  catch (const std::exception&) {
    // commit or rollback semantics;
    destroy(head, cur);
    // Here it is impossible to get the allocator of the iterator,
    // so we cannot deallocate memory now.
  }
}
template <typename ForwardIterator, typename DataType, typename ValueType>
inline void __uninitialized_fill(ForwardIterator head, ForwardIterator tail, const DataType& elem, ValueType*) {
  using is_POD = typename __type_traits<ValueType>::is_POD_type;
  __uninitialized_fill_aux(head, tail, elem, is_POD());
}
template <typename ForwardIterator, typename DataType>
inline void uninitialized_fill(ForwardIterator head, ForwardIterator tail, const DataType& elem) {
  __uninitialized_fill(head, tail, elem, get_value_type(head));
}
/* end uninitialized_fill() */

/* uninitialized_fill_n() */
// Here we return the tail iterator for further use;
template <typename ForwardIterator, typename DataType, typename SizeType>
inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator head, SizeType n, const DataType& elem, __true_tag) {
  return fill_n(head, n, elem);
}
template <typename ForwardIterator, typename DataType, typename SizeType>
ForwardIterator __uninitialized_fill_n_aux(ForwardIterator head, SizeType n, const DataType& elem, __false_tag) {
  ForwardIterator cur = head;
  try {
    for (; n != 0; --n, ++cur) {
      construct(&*cur, elem);
    }
  }
  catch (const std::exception&) {
    // commit or rollback semantics;
    destroy(head, cur);
    // Here it is impossible to get the allocator of the iterator,
    // so we cannot deallocate memory now.
  }

  return cur;
}
template <typename ForwardIterator, typename DataType, typename SizeType, typename ValueType>
inline ForwardIterator __uninitialized_fill_n(ForwardIterator head, SizeType n, const DataType& elem, ValueType*) {
  using is_POD = typename __type_traits<ValueType>::is_POD_type;
  return __uninitialized_fill_n_aux(head, n, elem, is_POD());
}
template <typename ForwardIterator, typename DataType, typename SizeType>
inline ForwardIterator uninitialized_fill_n(ForwardIterator head, SizeType n, const DataType& elem) {
  return __uninitialized_fill_n(head, n, elem, get_value_type(head));
}
/* end uninitialized_fill_n() */

/* uninitialized_copy() */
template <typename InputIterator, typename ForwardIterator>
inline ForwardIterator __uninitialized_copy_aux(InputIterator head, InputIterator tail, ForwardIterator result, __true_tag) {
  return copy(head, tail, result);
}
template <typename InputIterator, typename ForwardIterator>
ForwardIterator __uninitialized_copy_aux(InputIterator head, InputIterator tail, ForwardIterator result, __false_tag) {
  try {
    ForwardIterator cur = result;
    for (; head != tail; ++head, ++cur) {
      construct(&*cur, *head);
    }
  }
  catch (const std::exception&) {
    // commit or rollback;
    destroy(result, cur);
    // Here it is impossible to get the allocator of the iterator,
    // so we cannot deallocate memory now.
  }

  return cur;
}
template <typename InputIterator, typename ForwardIterator, typename ValueType>
inline ForwardIterator __uninitialized_copy(InputIterator head, InputIterator tail, ForwardIterator result, const ValueType&) {
  using is_POD = typename __type_traits<ValueType>::is_POD_type;
  return __uninitialized_copy_aux(head, tail, result, is_POD());
}
template <typename InputIterator, typename ForwardIterator>
inline ForwardIterator uninitialized_copy(InputIterator head, InputIterator tail, ForwardIterator result) {
  return __uninitialized_copy(head, tail, result, get_value_type(head));
}
/* end uninitialized_copy() */

#endif /* LEMSTL_LEM_UNINITIALIZED_H_ */