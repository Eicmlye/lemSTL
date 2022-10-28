#ifndef LEMSTL_LEM_ALGORITHM_H_
#define LEMSTL_LEM_ALGORITHM_H_

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

#endif /* LEMSTL_LEM_ALGORITHM_H_ */