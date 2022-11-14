#ifndef LEMSTL_LEM_NUM_H_
#define LEMSTL_LEM_NUM_H_

#include "../lem_exception"

namespace lem {
const char* getOrdTag(size_t const num) {
  size_t tenth = num % 10;
  size_t hundredth = num % 100;
  bool normal100 = !(tenth > 0 && tenth < 4) || (hundredth > 10 && hundredth < 14); // not x1st, x2nd and x3rd, caution for 11th, 12th and 13th;

  if (normal100) {
    return "-th";
  }

  switch (tenth) {
  case 1:
    return "-st";
  case 2:
    return "-nd";
  case 3:
    return "-rd";
  default:
    throw ::lem::impossible_ord(num);
  }

  return "-th";
}
} /* end lem */

#endif