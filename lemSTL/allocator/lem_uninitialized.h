// Copying objects in uninitialized memory.
#ifndef LEMSTL_LEM_UNINITIALIZED_H_
#define LEMSTL_LEM_UNINITIALIZED_H_

#include "../lem_iterator" // for value_type();
#include "../lem_type_traits" // for __type_traits;
#include "../lem_algorithm/lem_algobase.h" // for fill(), fill_n(), and copy();
#include "../lem_memory/lem_construct.h" // for construct();

/* uninitialized_fill() */
// fill elem to uninitialized allocated memory;
template <typename ForwardIterator, typename DataType, typename ValueType>
inline void __uninitialized_fill_aux(ForwardIterator head, ForwardIterator tail, const DataType& elem, __true_tag) {
  fill(head, tail, elem);
}
template <typename ForwardIterator, typename DataType, typename ValueType>
inline void __uninitialized_fill_aux(ForwardIterator head, ForwardIterator tail, const DataType& elem, __false_tag) {
  ForwardIterator cur = head;
  for (; cur != tail; ++cur) {
    construct(&*cur, elem);

    // commit or rollback;
    /* exception handling */
  }
}
template <typename ForwardIterator, typename DataType, typename ValueType>
inline void __uninitialized_fill(ForwardIterator head, ForwardIterator tail, const DataType& elem, ValueType*) {
  using is_POD = typename __type_traits<ValueType>::is_POD_type;
  __uninitialized_fill_aux(head, tail, elem, is_POD());
}
template <typename ForwardIterator, typename DataType>
inline void uninitialized_fill(ForwardIterator head, ForwardIterator tail, const DataType& elem) {
  __uninitialized_fill(head, tail, elem, value_type(head));
}

#endif /* LEMSTL_LEM_UNINITIALIZED_H_ */