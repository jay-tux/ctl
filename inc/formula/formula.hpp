//
// Created by jay on 6/30/23.
//

#ifndef CTL_FORMULA_HPP
#define CTL_FORMULA_HPP

#include <iostream>
#include <vector>
#include <optional>
#include <string>

namespace ctl::formula {
enum struct node_type { TRUE, ATOMIC, CONJUNCTION, NEGATION, E_NEXT, E_UNTIL, E_ALWAYS };

class ctlf_node {
public:
  node_type n;
  std::string atom;
  std::vector<ctlf_node> children;

  [[nodiscard]] std::string generate_var() const;
  void replace_subtree_by(const std::string &replacement);
  void dump() const;
  void dump_tree(size_t d = 0) const;

private:
  static size_t counter;
};
}

#endif //CTL_FORMULA_HPP
