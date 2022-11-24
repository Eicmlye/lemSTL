#ifndef LEMSTL_LEM_EXCEPTION_LIST_H_
#define LEMSTL_LEM_EXCEPTION_LIST_H_

#include <string> // for std::string
#include <exception> // for class exception;

namespace lem {
class danger_header : public ::std::exception {
  public:
  using _Mybase = ::std::exception;

  explicit danger_header(const ::std::string& _Message) : _Mybase(_Message.c_str()) {}

  explicit danger_header(const char* _Message) : _Mybase(_Message) {}
};
class del_header : public ::lem::danger_header {
  public:
  using _Mybase = ::lem::danger_header;

  del_header(void) : _Mybase("Attempting to delete header node of list. ") {}

  explicit del_header(const ::std::string& _Message) : _Mybase(_Message.c_str()) {}

  explicit del_header(const char* _Message) : _Mybase(_Message) {}
};
class mov_header : public ::lem::danger_header {
  public:
  using _Mybase = ::lem::danger_header;

  mov_header(void) : _Mybase("Attempting to move header node of list. ") {}

  explicit mov_header(const ::std::string& _Message) : _Mybase(_Message.c_str()) {}

  explicit mov_header(const char* _Message) : _Mybase(_Message) {}
};

class self_splice : public ::std::exception {
  public:
  using _Mybase = ::std::exception;

  self_splice(void) : _Mybase("List self-splicing. ") {}

  explicit self_splice(const ::std::string& _Message) : _Mybase(_Message.c_str()) {}

  explicit self_splice(const char* _Message) : _Mybase(_Message) {}
};
} /* end lem */

#endif