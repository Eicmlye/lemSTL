#define _CRT_SECURE_NO_WARNINGS

#include <iostream>

//#define LEM_DEBUG
//#define LEM_WARNING
#include "lemSTL/lem_test"

#ifdef LEM_TEST_
//  #define TEST_VECTOR_
//  #define TEST_LIST_
  #define TEST_DEQUE_
#else
  #include "lemSTL/lem_vector"
  #include "lemSTL/lem_list"
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
#ifdef TEST_LIST_
  #include "lemSTL/lem_list"

  TEST(int_list_ctor) {
    lem::list<int> lst = { 1, 2, 3 };

    EXPECT_EQ_INT_LIST(lst, { 1, 2, 3 });
    EXPECT_NEQ_INT_LIST(lst, { 1, 2, 3, 4 });
    EXPECT_NEQ_INT_LIST(lst, { 1, 2, 4 });
  }
  TEST(int_list_iterator) {
    lem::list<int> lst = { 1, 2, 3, 4, 5 };
    lem::list<int>::iterator cur = lst.begin();

    EXPECT_EQ(*cur, 1);
    ++cur; ++cur;
    EXPECT_NEQ(*cur, 2);
    EXPECT_EQ(*cur, 3);
  }
  TEST(int_list_accessor) {
    lem::list<int> lst = { 1, 2, 3, 4, 5 };

    EXPECT_EQ(lst.front(), 1);
    EXPECT_EQ(lst.back(), 5);
  }
  TEST(int_list_capacity) {
    lem::list<int> lst = {};

    EXPECT_EQ(lst.size(), 0);
    EXPECT_EQ(lst.empty(), true);

    lem::list<int> lst2 = { 1, 3, 5, 7 };

    EXPECT_EQ(lst2.size(), 4);
    EXPECT_EQ(lst2.empty(), false);
  }
  TEST(int_list_push_and_pop) {
    lem::list<int> lst = { 1, 3, 4, 5, 7 };
    lem::list<int>::iterator iter = lst.begin(); // iter points to element 1;

    lst.push_front(0);
    EXPECT_EQ_INT_LIST(lst, { 0, 1, 3, 4, 5, 7 });
    lst.push_back(8);
    EXPECT_EQ_INT_LIST(lst, { 0, 1, 3, 4, 5, 7, 8 });
    ++iter;
    lst.insert(iter, 2);
    EXPECT_EQ_INT_LIST(lst, { 0, 1, 2, 3, 4, 5, 7, 8 });

    lst.pop_front();
    EXPECT_EQ_INT_LIST(lst, { 1, 2, 3, 4, 5, 7, 8 });
    lst.pop_back();
    EXPECT_EQ_INT_LIST(lst, { 1, 2, 3, 4, 5, 7 });
    lst.erase(iter);
    EXPECT_EQ_INT_LIST(lst, { 1, 2, 4, 5, 7 });
    EXPECT_ERROR(lst.erase(lst.end()), lem::del_header);
  }
  TEST(int_list_clear) {
    lem::list<int> lst = { 1, 3, 5, 7 };

    lst.clear();
    EXPECT_EQ(lst.size(), 0);
    EXPECT_EQ(lst.empty(), true);

    lst.push_back(1);
    EXPECT_EQ_INT_LIST(lst, { 1 });
  }
  TEST(int_list_remove) {
    lem::list<int> lst = { 1, 3, 5, 7, 9, 1 };

    EXPECT_EQ(lst.remove(1), 2);
    EXPECT_EQ_INT_LIST(lst, { 3, 5, 7, 9 });

    EXPECT_EQ(lst.remove(9), 1);
    EXPECT_EQ_INT_LIST(lst, { 3, 5, 7 });

    EXPECT_EQ(lst.remove(5), 1);
    EXPECT_EQ_INT_LIST(lst, { 3, 7 });
  }
  TEST(int_list_unique) {
    // no-op;
    lem::list<int> lst = { 1, 2, 3, 4, 5 };
    EXPECT_EQ(lst.unique(), 0);
    EXPECT_EQ_INT_LIST(lst, { 1, 2, 3, 4, 5 });

    // 1 duplicant, head/mid/tail;
    lem::list<int> lst1 = { 1, 1, 2, 3, 4, 5 };
    EXPECT_EQ(lst1.unique(), 1);
    EXPECT_EQ_INT_LIST(lst1, { 1, 2, 3, 4, 5 });

    lem::list<int> lst2 = { 1, 2, 3, 4, 5, 5 };
    EXPECT_EQ(lst2.unique(), 1);
    EXPECT_EQ_INT_LIST(lst2, { 1, 2, 3, 4, 5 });

    lem::list<int> lst3 = { 1, 2, 3, 3, 4, 5 };
    EXPECT_EQ(lst3.unique(), 1);
    EXPECT_EQ_INT_LIST(lst3, { 1, 2, 3, 4, 5 });

    // mixed;
    lem::list<int> lst4 = { 1, 1, 2, 3, 3, 4, 5, 5, 5 };
    EXPECT_EQ(lst4.unique(), 4);
    EXPECT_EQ_INT_LIST(lst4, { 1, 2, 3, 4, 5 });
  }
  TEST(int_list_splice_1) {
    lem::list<int> lst = { 1, 2, 3 };
    lem::list<int> child1 = { 9, 8, 7 };
    lem::list<int> child2 = { 6, 5, 4 };
    lem::list<int> child3 = { 13, 12, 11 };
    lem::list<int>::iterator cur = lst.begin();

    lst.splice(lst.begin(), child1);
    EXPECT_EQ_INT_LIST(lst, { 9, 8, 7, 1, 2, 3 });
    // now child1 is empty;
    lst.splice(lst.begin(), child1);
    EXPECT_EQ_INT_LIST(lst, { 9, 8, 7, 1, 2, 3 });
    lst.splice(lst.end(), child2);
    EXPECT_EQ_INT_LIST(lst, { 9, 8, 7, 1, 2, 3, 6, 5, 4 });
    lem::advance(cur, 2);
    lst.splice(cur, child3);
    EXPECT_EQ_INT_LIST(lst, { 9, 8, 7, 1, 2, 13, 12, 11, 3, 6, 5, 4 });
  }
  TEST(int_list_splice_2) {
    lem::list<int> lst = { 1, 2, 3 };
    lem::list<int> child1 = { 9, 8, 7 };
    lem::list<int> child2 = { 6, 5, 4 };
    lem::list<int> child3 = { 13, 12, 11 };
    lem::list<int>::iterator cur = lst.begin();
    lem::list<int>::iterator head = child1.begin();
    lem::list<int>::iterator tail = child1.end();

    lst.splice(lst.begin(), lst, lst.begin());
    EXPECT_EQ_INT_LIST(lst, { 1, 2, 3 });
    lst.splice(++lst.begin(), lst, lst.begin());
    EXPECT_EQ_INT_LIST(lst, { 1, 2, 3 });

    lst.splice(lst.begin(), child1, head);
    EXPECT_EQ_INT_LIST(lst, { 9, 1, 2, 3 });
    lst.splice(lst.begin(), child1, --tail);
    EXPECT_EQ_INT_LIST(lst, { 7, 9, 1, 2, 3 });
    head = child2.begin();
    ++head;
    lst.splice(lst.end(), child2, head);
    EXPECT_EQ_INT_LIST(lst, { 7, 9, 1, 2, 3, 5 });
    head = child3.begin();
    ++head;
    ++cur;
    lst.splice(cur, child3, head);
    EXPECT_EQ_INT_LIST(lst, { 7, 9, 1, 12, 2, 3, 5 });
  }
  TEST(int_list_splice_3) {
    lem::list<int> lst = { 1, 2, 3 };
    lem::list<int> child1 = { 9, 8, 7 };
    lem::list<int>::iterator head = child1.begin();
    lem::list<int>::iterator tail = child1.end();

    lst.splice(lst.begin(), child1, ++head, --tail);
    EXPECT_EQ_INT_LIST(lst, { 8, 1, 2, 3 });
  }
  TEST(int_list_splice_error) {
    lem::list<int> lst = { 1, 2, 3 };
    lem::list<int> child1 = { 9, 8, 7 };
    lem::list<int> child2 = { 6, 5, 4 };
    lem::list<int> child3 = { 13, 12, 11 };
    lem::list<int>::iterator cur = lst.begin();
    lem::list<int>::iterator head = child1.begin();
    lem::list<int>::iterator tail = child1.end();

    EXPECT_ERROR(lst.splice(cur, lst), lem::self_splice);
  }
  TEST(int_list_merge) {
    lem::list<int> lst = { 1, 3, 5, 7, 9 };
    lem::list<int> other = { 0, 1, 2, 4, 5, 8 };

    lst.merge(other);
    EXPECT_EQ_INT_LIST(lst, { 0, 1, 1, 2, 3, 4, 5, 5, 7, 8, 9 });
  }
  TEST(int_list_reverse) {
    lem::list<int> lst = { 1, 3, 5, 7, 9 };

    lst.reverse();
    EXPECT_EQ_INT_LIST(lst, { 9, 7, 5, 3, 1 });
  }
  TEST(int_list_sort) {
    lem::list<int> lst = { 1, 0, -4, 2, -9, 5, 7, 3, -8, -4 };

    lst.sort();
    EXPECT_EQ_INT_LIST(lst, { -9, -8, -4, -4, 0, 1, 2, 3, 5, 7 });
  }
#endif
#ifdef TEST_DEQUE_
  #include "lemSTL/lem_deque"

  TEST(int_deque_ctor) {
    lem::deque<int> dq = { 1, 2 };

    EXPECT_EQ(dq.at(0), 1);
    EXPECT_EQ(dq.at(1), 2);

    lem::deque<int> dr(3, 3);

    EXPECT_EQ(dr.at(0), 3);
    EXPECT_EQ(dr.at(1), 3);
    EXPECT_EQ(dr.at(2), 3);
  }
#endif

int main(void) {
  #ifdef LEM_TEST_
    RUN_ALL_TESTS();
  #else
    lem::list<int> lst = { 1, 2, 3 };
    lem::list<int> lst2 = lst;
  #endif

  return 0;
}