#ifndef LEM_EXPECT_H_
#define LEM_EXPECT_H_

#include <iostream>

namespace lem {

#define EXPECT_EQ(actual, expect) \
  do {\
    if (expect != actual) {\
      ::lem::testManager::getManager()->currentTestcase_->case_result_ = false;\
      ::std::cout << "Failed at Line " << __LINE__ << " of File " << __FILE__ << ::std::endl;\
      ::std::cout << "\tExpect: " << expect << ", Actual: " << actual << ::std::endl;\
    }\
  } while (0)
#define EXPECT_NEQ(actual, expect) \
  do {\
    if (expect == actual) {\
      ::lem::testManager::getManager()->currentTestcase_->case_result_ = false;\
      ::std::cout << "Failed at Line " << __LINE__ << " of File " << __FILE__ << ::std::endl;\
      ::std::cout << "\tExpect: NOT " << expect << ", Actual: " << actual << ::std::endl;\
    }\
  } while (0)
#define EXPECT_ERROR(actual, error_expect) \
  do {\
    try {\
      actual;\
      throw "EXPECT_ERROR: no error";\
    }\
    catch (error_expect const&) {}\
    catch (::std::exception const& e) {\
      ::lem::testManager::getManager()->currentTestcase_->case_result_ = false;\
      ::std::cout << "Failed at Line " << __LINE__ << " of File " << __FILE__ << ::std::endl;\
      ::std::cout << "\tExpect: throw " << #error_expect << ", Actual: " << e.what() << ::std::endl;\
    }\
    catch (...) {\
      ::lem::testManager::getManager()->currentTestcase_->case_result_ = false; \
      ::std::cout << "Failed at Line " << __LINE__ << " of File " << __FILE__ << ::std::endl; \
      ::std::cout << "\tExpect: throw " << #error_expect << ", Actual: No error" << ::std::endl; \
    }\
  } while (0)

#define EXPECT_EQ_INT_VECTOR(actual, ...) \
  do {\
    ::std::initializer_list<int> vec_exp(__VA_ARGS__);\
    \
    if (actual.size() != vec_exp.size()) {\
      ::lem::testManager::getManager()->currentTestcase_->case_result_ = false;\
      ::std::cout << "Failed at Line " << __LINE__ << " of File " << __FILE__ << ::std::endl;\
      ::std::cout << "\tExpect: vector size " << vec_exp.size() << ", Actual: " << actual.size() << ::std::endl;\
    }\
    \
    ::std::initializer_list<int>::iterator exp_iter = vec_exp.begin();\
    ::lem::vector<int>::iterator iter = actual.begin();\
    for (; iter != actual.end(); ++iter, ++exp_iter) {\
      if ((*iter) != (*exp_iter)) {\
        ::lem::testManager::getManager()->currentTestcase_->case_result_ = false;\
        ::std::cout << "Failed at Line " << __LINE__ << " of File " << __FILE__ << ::std::endl;\
        ::std::cout << "\tElement [" << iter - actual.begin();\
        ::std::cout << "] Expect: " << *exp_iter << ", Actual: " << *iter << ::std::endl;\
        break;\
      }\
    }\
  } while (0)

} /* end lem */
#endif