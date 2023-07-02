//
// Created by jay on 6/30/23.
//

#ifndef CTL_UTIL_HPP
#define CTL_UTIL_HPP
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <iterator>

#ifdef _DEBUG
#define LOG(expr) std::cout << expr << "\n"
#else
#define LOG(expr)
#endif

namespace ctl {
inline std::vector<std::string> split_by(const std::string &s, char c) {
  std::vector<std::string> res;
  size_t curr = 0;
  size_t idx = s.find(c);
  while(idx != std::string::npos) {
    res.push_back(s.substr(curr, idx - curr));
    curr = idx + 1;
    idx = s.find(c, curr);
  }

  res.push_back(s.substr(curr));
  return res;
}

inline std::vector<std::string> split_ws(const std::string &s) {
  std::istringstream buf(s);
  return std::vector<std::string>{std::istream_iterator<std::string>(buf), std::istream_iterator<std::string>()};
}

inline std::string strip(const std::string &s) {
  size_t start = 0;
  for(; start < s.length() && isspace(s[start]); start++) {}

  size_t end = s.size() - 1;
  for(; end > 0 && isspace(s[end]); end--) {}
  return (start <= end) ? s.substr(start, end - start + 1) : "";
}
}

#endif //CTL_UTIL_HPP
