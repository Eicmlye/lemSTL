#ifndef LEM_TEST_H_
#define LEM_TEST_H_

#include <iostream>

#include "../lemSTL/lem_vector" // for lem::vector;

// See https://www.cnblogs.com/coderzh/archive/2009/04/12/1434155.html;

namespace lem {
/* class testcase */
class testcase {
 public:
  char const* case_name_;
  bool case_result_ = true;

  #ifdef LEM_DEBUG
    ~testcase(void) { std::cout << "\tLEM_DEBUG: Call dtor testcase. " << std::endl; }
  #endif

  testcase(char const* case_name) : case_name_(case_name) {}

  virtual void run_test(void) = 0;
};
/* end class testcase */

/* class testManager */
class testManager {
 public:
  bool result_; // whether all the testcases passed;
  size_t passed_;
  size_t failed_;
  testcase* currentTestcase_;
  
  testManager(void) :
    result_(true), 
    passed_(0), 
    failed_(0), 
    currentTestcase_(nullptr),
    testcases_({}) {

  }
  ~testManager(void) {
    // destroy all testcases, avoid memory leak;
    for (lem::vector<testcase*>::iterator iter = testcases_.begin(); iter != testcases_.end(); ++iter) {
      destroy(*iter);
    }
    destroy(&testcases_);
    #ifdef LEM_DEBUG
      std::cout << "\tLEM_DEBUG: Call dtor testManager. " << std::endl;
    #endif
  }

  static testManager* getManager(void);
  testcase* registerTestcase(testcase* testcase);
  void runTestList(void);

 protected:
  lem::vector<testcase*> testcases_;
};

testManager* testManager::getManager(void) {
  static testManager manager;

  return &manager;
}
testcase* testManager::registerTestcase(testcase* testcase) {
  testcases_.push_back(testcase);

  return testcase;
}
void testManager::runTestList(void) {
  result_ = true;
  for (lem::vector<testcase*>::iterator iter = testcases_.begin(); iter != testcases_.end(); ++iter) {
    currentTestcase_ = *iter;

    std::cout << "========" << std::endl;
    std::cout << "Run testcase " << currentTestcase_->case_name_ << "..." << std::endl << std::endl;
    currentTestcase_->run_test();
    std::cout << std::endl;
    std::cout << "End testcase " << currentTestcase_->case_name_ << ". " << std::endl;
    std::cout << "========" << std::endl;

    if (currentTestcase_->case_result_) {
      ++passed_;
    }
    else {
      ++failed_;
      result_ = false;
    }
  }

  printf("========\n%zd of %zd (%.2f%%) testcases passed. \n========\n",
         passed_, passed_ + failed_, 100 * (float)(passed_) / (passed_ + failed_));

  return;
}
/* end class testManager */

/* macros */
#define TESTCASE_NAME(testcase_name) testcase_name##_TEST

#define TEST(testcase_name) \
  class TESTCASE_NAME(testcase_name) : public ::lem::testcase {\
   public:\
    TESTCASE_NAME(testcase_name)(char const* case_name) : ::lem::testcase(case_name) {};\
    virtual void run_test(void);\
    \
   private:\
    /* EM QUESTION: why static? */\
    static ::lem::testcase* const testcase_;\
  };\
  \
  ::lem::testcase* const TESTCASE_NAME(testcase_name)::testcase_ =\
    ::lem::testManager::getManager()->registerTestcase(new TESTCASE_NAME(testcase_name)(#testcase_name));\
  void TESTCASE_NAME(testcase_name)::run_test(void) /* user add content for test */

#define RUN_ALL_TESTS() ::lem::testManager::getManager()->runTestList();
/* end macros */
} /* end lem */

#include "lem_expect.h"

#endif /* LEM_TEST_H_ */