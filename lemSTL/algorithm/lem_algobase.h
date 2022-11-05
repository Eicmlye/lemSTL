#ifndef LEMSTL_LEM_ALGOBASE_H_
#define LEMSTL_LEM_ALGOBASE_H_

#include <cstring> // for memmove();

#include "../lem_iterator" // for iterator tags;
#include "../lem_type_traits" // for __type_traits;

namespace lem {
template <typename ForwardIterator, typename T>
void fill(ForwardIterator head, ForwardIterator tail, const T& value) {
  for (; head != tail; ++head) {
    *head = value;
  }

  return;
}

template <typename OutputIterator, typename SizeType, typename T>
OutputIterator fill_n(OutputIterator head, SizeType n, const T& value) {
  for (; n != 0; --n, ++head) {
    *head = value;
  }

  return head;
}

/* EM NOTE: copy() */
// copy()
// |
// +---->__copy_dispatch()
//        |
//        +----native pointer---->__copy_native()
//        |                       |
//        |                       +----trivial assignment oprtr---->memmove()
//        |                       |
//        |                       +----else------------------
//        |                                                 |
//        +----else---->__copy()                            |
//                      |                                   v
//                      +----random access iterator---->__copy_class()
//                      |
//                      +----else---->__copy()

template <typename InputIterator, typename OutputIterator>
OutputIterator __copy(InputIterator head, InputIterator tail, OutputIterator result, input_iterator_tag) {
  for (; head != tail; ++head, ++result) {
    *result = *head;
  }

  return result;
}
template <typename RandomAccessIterator, typename OutputIterator>
inline OutputIterator __copy(RandomAccessIterator head, RandomAccessIterator tail, OutputIterator result, random_access_iterator_tag) {
  return __copy_class(head, tail, result, get_difference_type(head));
}
template <typename RandomAccessIterator, typename OutputIterator, typename DiffType>
OutputIterator __copy_class(RandomAccessIterator head, RandomAccessIterator tail, OutputIterator result, DiffType*) {
  for (DiffType n = tail - head; n > 0; ++head, ++result, --n) {
    *result = *head;
  }

  return result;
}
// Use a functor to partial specialize a function;
template <typename InputIterator, typename OutputIterator>
class __copy_dispatch {
 public:
  OutputIterator operator()(InputIterator head, InputIterator tail, OutputIterator result) {
    return __copy(head, tail, result, get_iterator_category(head));
  }
};
  /* deal with native pointers */
  template <typename T>
  inline T* __copy_native(const T* head, const T* tail, T* result, __true_tag) {
    memmove(result, head, sizeof(T) * (tail - head));

    return result + (tail - head);
  }
  template <typename T>
  inline T* __copy_native(const T* head, const T* tail, T* result, __false_tag) {
    return __copy_class(head, tail, result, get_difference_type(head));
  }
  template <typename T>
  class __copy_dispatch<T*, T*> {
   public:
    T* operator()(T* head, T* tail, T* result) {
      using triv_assgn = typename __type_traits<T>::has_trivial_assignment_oprtr;
      return __copy_native(head, tail, result, triv_assgn());
    }
  };
  template <typename T>
  class __copy_dispatch<const T*, T*> {
   public:
    T* operator()(const T* head, const T* tail, T* result) {
      using triv_assgn = typename __type_traits<T>::has_trivial_assignment_oprtr;
      return __copy_native(head, tail, result, triv_assgn());
    }
  };
  /* end native pointers */
template <typename InputIterator, typename OutputIterator>
inline OutputIterator copy(InputIterator head, InputIterator tail, OutputIterator result) {
  // EM NOTE: two ()'s here, the first one to create a __copy_dispatch object,
  // and the second to use operator() of the object.
  return __copy_dispatch<InputIterator, OutputIterator>()(head, tail, result);
}
// specialization for char pointers;
inline char* copy(const char* head, const char* tail, char* result) {
  memmove(result, head, tail - head);

  return result + (tail - head);
}
/* end copy() */
} /* end lem */

#endif /* LEMSTL_LEM_ALGORITHM_H_ */