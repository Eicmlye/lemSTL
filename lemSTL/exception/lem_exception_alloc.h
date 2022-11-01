#ifndef LEMSTL_LEM_EXCEPTION_ALLOC_H_
#define LEMSTL_LEM_EXCEPTION_ALLOC_H_

#include <string> // for std::string
#include <exception> // for class exception;

namespace lem {
class alloc_zero_free_list : public ::std::exception {
 public:
  using _Mybase = ::std::exception;

  alloc_zero_free_list(void) : _Mybase("Requested zero-byte free-list. ") {}

  explicit alloc_zero_free_list(const ::std::string& _Message) : _Mybase(_Message.c_str()) {}

  explicit alloc_zero_free_list(const char* _Message) : _Mybase(_Message) {}
};
}

#endif