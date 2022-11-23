#ifndef LEMSTL_LEM_EXCEPTION_LIST_H_
#define LEMSTL_LEM_EXCEPTION_LIST_H_

#include <string> // for std::string
#include <exception> // for class exception;

namespace lem {
class del_header : public ::std::exception {
  public:
  using _Mybase = ::std::exception;

  del_header(void) : _Mybase("Attempting to delete header node of list. ") {}

  explicit del_header(const ::std::string& _Message) : _Mybase(_Message.c_str()) {}

  explicit del_header(const char* _Message) : _Mybase(_Message) {}
};
} /* end lem */

#endif