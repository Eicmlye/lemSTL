#ifndef LEMSTL_LEM_EXCEPTION_EXTRA_H_
#define LEMSTL_LEM_EXCEPTION_EXTRA_H_

#include <string> // for std::string
#include <exception> // for class exception;

namespace lem {
class impossible_ord : public ::std::exception {
 public:
  using _Mybase = ::std::exception;

  size_t data_ = 0;

  explicit impossible_ord(const size_t num) : _Mybase("Impossible ordinal number shows up. ") { data_ = num; }
};
} /* end lem */

#endif