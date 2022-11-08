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

/* copy_backward() */
template <typename BidirectionalIterator1, typename BidirectionalIterator2>
BidirectionalIterator2 __copy_backward(BidirectionalIterator1 head, BidirectionalIterator1 tail, BidirectionalIterator2 result_tail, input_iterator_tag) {
  for (; head != tail; --tail, --result_tail) {
    *result_tail = *tail;
  }

  return result_tail;
}
template <typename RandomAccessIterator, typename BidirectionalIterator>
inline BidirectionalIterator __copy_backward(RandomAccessIterator head, RandomAccessIterator tail, BidirectionalIterator result_tail, random_access_iterator_tag) {
  return __copy_backward_class(head, tail, result_tail, get_difference_type(head));
}
template <typename RandomAccessIterator, typename BidirectionalIterator, typename DiffType>
BidirectionalIterator __copy_backward_class(RandomAccessIterator head, RandomAccessIterator tail, BidirectionalIterator result_tail, DiffType*) {
  for (DiffType n = tail - head; n > 0; --tail, --result_tail, --n) {
    *result_tail = *tail;
  }

  return result_tail;
}
// Use a functor to partial specialize a function;
template <typename BidirectionalIterator1, typename BidirectionalIterator2>
class __copy_backward_dispatch {
  public:
  BidirectionalIterator2 operator()(BidirectionalIterator1 head, BidirectionalIterator1 tail, BidirectionalIterator2 result_tail) {
    return __copy_backward(head, tail, result_tail, get_iterator_category(head));
  }
};
  /* deal with native pointers */
  template <typename T>
  inline T* __copy_backward_native(const T* head, const T* tail, T* result_tail, __true_tag) {
    memmove(result_tail - (tail - head), head, sizeof(T) * (tail - head));

    return result_tail - (tail - head);
  }
  template <typename T>
  inline T* __copy_backward_native(const T* head, const T* tail, T* result_tail, __false_tag) {
    return __copy_backward_class(head, tail, result_tail, get_difference_type(head));
  }
  template <typename T>
  class __copy_backward_dispatch<T*, T*> {
    public:
    T* operator()(T* head, T* tail, T* result_tail) {
      using triv_assgn = typename __type_traits<T>::has_trivial_assignment_oprtr;
      return __copy_backward_native(head, tail, result_tail, triv_assgn());
    }
  };
  template <typename T>
  class __copy_backward_dispatch<const T*, T*> {
    public:
    T* operator()(const T* head, const T* tail, T* result_tail) {
      using triv_assgn = typename __type_traits<T>::has_trivial_assignment_oprtr;
      return __copy_backward_native(head, tail, result_tail, triv_assgn());
    }
  };
  /* end native pointers */
template <typename BidirectionalIterator1, typename BidirectionalIterator2>
inline BidirectionalIterator2 copy_backward(BidirectionalIterator1 head, BidirectionalIterator1 tail, BidirectionalIterator2 result_tail) {
  return __copy_backward_dispatch< BidirectionalIterator1, BidirectionalIterator2>()(head, tail, result_tail);
}
// specialization for char pointers;
inline char* copy_backward(const char* head, const char* tail, char* result_tail) {
  return copy(head, tail, result_tail - (tail - head));
}
/* end copy_backward() */

/* max & min */
template <typename T>
inline T const& min(T const& a, T const& b) {
  return (a < b ? a : b);
}
template <typename T, typename Comp>
inline T const& min(T const& a, T const& b, Comp islt) {
  return (islt(a, b) ? a : b);
}
template <typename T>
inline T const& max(T const& a, T const& b) {
  return (a < b ? b : a);
}
template <typename T, typename Comp>
inline T const& max(T const& a, T const& b, Comp islt) {
  return (islt(a, b) ? b : a);
}
/* end max & min */
} /* end lem */

#endif /* LEMSTL_LEM_ALGORITHM_H_ */