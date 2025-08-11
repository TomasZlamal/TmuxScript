#include "util.h"

namespace tmx {
namespace util {
int count_string(const std::string &haystack, const std::string &needle) {
  if (needle.length() == 0)
    return 0;
  int count = 0;
  for (size_t offset = haystack.find(needle); offset != std::string::npos;
       offset = haystack.find(needle, offset + needle.length())) {
    ++count;
  }
  return count;
}
int count_char(const std::string &s, char c) {
  int count = 0;
  bool ignore_next = 0;

  for (int i = 0; i < s.size(); i++) {
    // escape sequence-handling

    if (s[i] == c) {
      if (ignore_next)
        ;
      else {
        count++;
      }
    }
    if (s[i] == '\\')
      ignore_next = 1;
    else
      ignore_next = 0;
  };
  return count;
}
} // namespace util
} // namespace tmx
