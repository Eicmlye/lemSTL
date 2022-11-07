#ifndef LEMSTL_LEM_EXCEPTION_VECTOR_H_
#define LEMSTL_LEM_EXCEPTION_VECTOR_H_

#include <string> // for std::string
#include <exception> // for class exception;

namespace lem {
class pop_empty_vector : public ::std::exception {
 public:
  using _Mybase = ::std::exception;

  pop_empty_vector(void) : _Mybase("Attempting to pop an empty vector. ") {}

  explicit pop_empty_vector(const ::std::string& _Message) : _Mybase(_Message.c_str()) {}

  explicit pop_empty_vector(const char* _Message) : _Mybase(_Message) {}
};
class illegal_iterval : public ::std::exception {
 public:
  using _Mybase = ::std::exception;

  illegal_iterval(void) : _Mybase("Left bound is greater than right bound. ") {}

  explicit illegal_iterval(const ::std::string& _Message) : _Mybase(_Message.c_str()) {}

  explicit illegal_iterval(const char* _Message) : _Mybase(_Message) {}
};
} /* end lem */

#endif