#ifndef LEMSTL_LEM_EXCEPTION_ALLOC_H_
#define LEMSTL_LEM_EXCEPTION_ALLOC_H_

#include <exception> // for class exception;

class alloc_zero_free_list : public std::exception {
 public:
  using _Mybase = exception;

  explicit alloc_zero_free_list(const string& _Message) : _Mybase(_Message.c_str()) {}

  explicit alloc_zero_free_list(const char* _Message) : _Mybase(_Message) {}
};

#endif