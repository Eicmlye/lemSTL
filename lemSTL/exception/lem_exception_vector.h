#ifndef LEMSTL_LEM_EXCEPTION_VECTOR_H_
#define LEMSTL_LEM_EXCEPTION_VECTOR_H_

#include <exception>

class pop_empty_vector : public std::exception {
 public:
  using _Mybase = exception;

  explicit pop_empty_vector(const string& _Message) : _Mybase(_Message.c_str()) {}

  explicit pop_empty_vector(const char* _Message) : _Mybase(_Message) {}
};

#endif