//
// Created by jay on 6/30/23.
//

#ifndef CTL_GRAPH_READER_HPP
#define CTL_GRAPH_READER_HPP

#include <iostream>
#include <stdexcept>
#include "ts.hpp"

namespace ctl::graph {
struct graph_reader {
  static default_ts parse(std::istream &strm);
};
}

#endif //CTL_GRAPH_READER_HPP
