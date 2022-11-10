#ifndef LEM_EXPECT_H_
#define LEM_EXPECT_H_

#include <iostream>

namespace lem {
#define EXPECT_EQ(actual, expect) \
  if (expect != actual) {\
    lem::testManager::getManager()->currentTestcase_->case_result_ = false;\
    std::cout << "Failed at Line " << __LINE__ << " of File " << __FILE__ << std::endl;\
    std::cout << "\tExpect: " << expect << ", Actual: " << actual << std::endl;\
  }
#define EXPECT_NEQ(actual, expect) \
  if (expect == actual) {\
    lem::testManager::getManager()->currentTestcase_->case_result_ = false;\
    std::cout << "Failed at Line " << __LINE__ << " of File " << __FILE__ << std::endl;\
    std::cout << "\tExpect: NOT " << expect << ", Actual: " << actual << std::endl;\
  }
#define EXPECT_ERROR(actual, error_expect) \
  try {\
    actual;\
    throw "EXPECT_ERROR: no error";\
  }\
  catch (error_expect const&) {}\
  catch (std::exception const& e) {\
    lem::testManager::getManager()->currentTestcase_->case_result_ = false;\
    std::cout << "Failed at Line " << __LINE__ << " of File " << __FILE__ << std::endl;\
    std::cout << "\tExpect: throw " << #error_expect << ", Actual: " << e.what() << std::endl;\
  }\
  catch (...) {\
    lem::testManager::getManager()->currentTestcase_->case_result_ = false; \
    std::cout << "Failed at Line " << __LINE__ << " of File " << __FILE__ << std::endl; \
    std::cout << "\tExpect: throw " << #error_expect << ", Actual: No error" << std::endl; \
  }

#define EXPECT_EQ_INT_VECTOR(actual, vector_expect)

} /* end lem */
#endif