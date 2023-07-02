//
// Created by jay on 6/30/23.
//

#ifndef CTL_FORMULA_PARSER_HPP
#define CTL_FORMULA_PARSER_HPP

#include <iostream>
#include "formula.hpp"

namespace ctl::formula {
struct parser {
  static ctlf_node parse(std::istream &strm);
};
}

#endif //CTL_FORMULA_PARSER_HPP
