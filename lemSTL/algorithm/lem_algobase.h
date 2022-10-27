#ifndef LEMSTL_LEM_ALGORITHM_H_
#define LEMSTL_LEM_ALGORITHM_H_

template <typename ForwardIterator, typename T>
void fill(ForwardIterator head, ForwardIterator tail, const T& value) {
  for (; head != tail; ++head) {
    *head = value;
  }

  return;
}

#endif /* LEMSTL_LEM_ALGORITHM_H_ */