// Sequential containers;
#ifndef LEMSTL_LEM_VECTOR_H_
#define LEMSTL_LEM_VECTOR_H_

#include <cstddef> // for ptrdiff_t;

#include "lem_memory"
#include "../lem_iterator"

namespace lem {
// See declarations at https://en.cppreference.com/w/cpp/container/vector;
template <typename DataType, typename AllocType = alloc>
class vector {
 public:
  // Member types;
  // iterator traits;
  using iterator = DataType*; 
  using reverse_iterator = DataType*;
  /* using iterator_category = random_access_iterator_tag; */ // native pointer iterator;
  using value_type = DataType;
  using difference_type = ptrdiff_t;
  using pointer_type = DataType*;
  using reference_type = DataType&;
}£»
} // lem

#endif
