// Construction and destruction of objects.
#ifndef LEMSTL_LEM_CONSTRUCT_H_
#define LEMSTL_LEM_CONSTRUCT_H_

#include <new.h> // enable placement new;
#include <../lem_type_traits> // for __type_traits;

#include "../iterator/lem_iterator.h" // for iterator_traits;

namespace lem {
template <typename AllocedObject, typename InitValue>
inline void construct(AllocedObject* p, InitValue const& value) {
  new(p) AllocedObject(value); // placement new, call ctor AllocedObject(value);
}

/* Destroy a specific object */
template <typename Object>
inline void destroy(Object* p) {
  p->~Object(); // call dtor ~Object() manually, since placement new is used;
}
/* end */

/* Destroy objects in a specific range */
template <typename ForwardIterator>
inline void __destroy_aux(ForwardIterator head, ForwardIterator tail, __false_tag) {
  for (; head < tail; ++head) {
    destroy(&*head);
  }
}
template <typename ForwardIterator>
inline void __destroy_aux(ForwardIterator, ForwardIterator, __true_tag) {}
// EM NOTE: here we use __destroy_aux() instead of IF condition statements
// to make sure destroy() is inline and can be determined during compile time. 
// Less time is used runtime.
template <typename ForwardIterator, typename T>
inline void __destroy(ForwardIterator head, ForwardIterator tail, T*) {
  using dtor_type_tag = typename __type_traits<T>::has_trivial_dtor;
  __destroy_aux(head, tail, dtor_type_tag());
}
template <typename ForwardIterator>
inline void destroy(ForwardIterator head, ForwardIterator tail) {
  __destroy(head, tail, value_type(head));
}
/* end */
} // lem

#endif
