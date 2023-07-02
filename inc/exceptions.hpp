//
// Created by jay on 6/30/23.
//

#ifndef CTL_EXCEPTIONS_HPP
#define CTL_EXCEPTIONS_HPP

#include <stdexcept>

namespace ctl {
struct parse_error : std::logic_error {
  using logic_error::logic_error;
};
}

#endif //CTL_EXCEPTIONS_HPP
