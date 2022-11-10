#define _CRT_SECURE_NO_WARNINGS

#include <iostream>

// #define LEM_DEBUG
#include "lemTest/lem_test"

#ifdef LEM_TEST_
  #define TEST_VECTOR_
#endif

#ifdef TEST_VECTOR_
  #include "lemSTL/lem_vector"

  TEST(int_vector_iterator) {
    lem::vector<int> vec = { 0, 1, 2, 3, 4, 5 };

    EXPECT_EQ(vec.begin(), &vec[0]);
    EXPECT_EQ(vec.end(), &vec[6]);
  }

  TEST(int_vector_accessor) {
    lem::vector<int> vec = { 0, 1, 2, 3, 4, 5 };

    EXPECT_EQ(vec.at(0), 0);
    EXPECT_EQ(vec.at(3), 3);
    EXPECT_EQ(vec.at(5), 5);
    EXPECT_ERROR(vec.at(6), std::out_of_range);

    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[3], 3);
    EXPECT_EQ(vec[5], 5);
    EXPECT_EQ(vec.front(), 0);
    EXPECT_EQ(vec.back(), 5);
  }

  TEST(int_vector_capacity_and_modifiers) {
    lem::vector<int> vec = {};

    EXPECT_EQ(vec.empty(), true);
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 0);
  }
#endif


int main(void) {
  #ifdef LEM_TEST_
    RUN_ALL_TESTS();
  #endif

  return 0;
}