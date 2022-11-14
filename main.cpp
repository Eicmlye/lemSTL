#define _CRT_SECURE_NO_WARNINGS

#include <iostream>

// #define LEM_DEBUG
#include "lemSTL/lem_test"

#ifdef LEM_TEST_
  #define TEST_VECTOR_
  #define TEST_LIST_
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
  TEST(int_vector_capacity) {
    lem::vector<int> vec = {};

    EXPECT_EQ(vec.empty(), true);
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 0);

    lem::vector<int> vec2 = { 1 };

    EXPECT_EQ(vec2.empty(), false);
    EXPECT_EQ(vec2.size(), 1);
    EXPECT_EQ(vec2.capacity(), 1);
  }
  TEST(int_vector_push_and_pop) {
    lem::vector<int> vec = {};
    vec.push_back(9);

    // first push_back test;
    EXPECT_EQ(vec.empty(), false);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec.capacity(), 1);

    // unrealloc push_back test;
    vec.push_back(7);
    vec.push_back(5);
    vec.push_back(3);
    vec.push_back(1);

    EXPECT_EQ_INT_VECTOR(vec, { 9, 7, 5, 3, 1 });
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec.capacity(), 8);

    // unrealloc insert test;
    vec.insert(vec.begin() + 2, 0, 2);

    EXPECT_EQ_INT_VECTOR(vec, { 9, 7, 0, 0, 5, 3, 1 });
    EXPECT_EQ(vec.size(), 7);
    EXPECT_EQ(vec.capacity(), 8);

    // realloc insert test;
    vec.insert(vec.begin() + 2, 10, 2);

    EXPECT_EQ_INT_VECTOR(vec, { 9, 7, 10, 10, 0, 0, 5, 3, 1 });
    EXPECT_EQ(vec.size(), 9);
    EXPECT_EQ(vec.capacity(), 14);

    // pop_back test;
    vec.pop_back();

    EXPECT_EQ_INT_VECTOR(vec, { 9, 7, 10, 10, 0, 0, 5, 3 });
    EXPECT_EQ(vec.size(), 8);
    EXPECT_EQ(vec.capacity(), 14);
  }
  TEST(int_vector_shrink_to_fit) {
    lem::vector<int> vec = { 9, 7, 5, 3 };

    vec.push_back(1);
    vec.shrink_to_fit();

    EXPECT_EQ_INT_VECTOR(vec, { 9, 7, 5, 3, 1 });
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec.capacity(), 5);
  }
  TEST(int_vector_reserve) {
    lem::vector<int> vec = {};

    // empty reserve test;
    vec.reserve(5);

    EXPECT_EQ_INT_VECTOR(vec, {});
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 5);

    vec.insert(vec.begin(), 1, 3);
    // unrealloc reserve test;
    vec.reserve(3);

    EXPECT_EQ_INT_VECTOR(vec, { 1, 1, 1 });
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec.capacity(), 5);

    // realloc reserve test;
    vec.reserve(10);

    EXPECT_EQ_INT_VECTOR(vec, { 1, 1, 1 });
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec.capacity(), 10);
  }
  TEST(int_vector_resize) {
    lem::vector<int> vec = {};

    // empty resize test;
    vec.resize(5);

    EXPECT_EQ_INT_VECTOR(vec, { 0, 0, 0, 0, 0 });
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec.capacity(), 5);

    // unrealloc reserve test;
    vec.resize(3);

    EXPECT_EQ_INT_VECTOR(vec, { 0, 0, 0 });
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec.capacity(), 5);

    // realloc reserve test;
    vec.resize(10);

    EXPECT_EQ_INT_VECTOR(vec, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
    EXPECT_EQ(vec.size(), 10);
    EXPECT_EQ(vec.capacity(), 10);
  }
#endif
#ifdef  TEST_LIST_
  #include "lemSTL/lem_list"


#endif



int main(void) {
  #ifdef LEM_TEST_
    RUN_ALL_TESTS();
  #endif

  return 0;
}