//
// Created by jay on 6/30/23.
//

#include <random>
#include <iostream>

#include "formula/formula.hpp"

using namespace ctl::formula;

size_t ctlf_node::counter = std::random_device{}();

std::ostream &operator<<(std::ostream &strm, const node_type &nt) {
  switch(nt) {
    case node_type::TRUE: return strm << "true";
    case node_type::ATOMIC: return strm << "atom";
    case node_type::CONJUNCTION: return strm << "conjunction (/\\)";
    case node_type::NEGATION: return strm << "negation (!)";
    case node_type::E_NEXT: return strm << "next (\\E \\X)";
    case node_type::E_UNTIL: return strm << "until (\\E \\U)";
    case node_type::E_ALWAYS: return strm << "always (\\E \\G)";
  }
  return strm;
}

std::string ctlf_node::generate_var() const {
  std::string res;

  switch(n) {
    case node_type::TRUE: res = "true"; break;
    case node_type::ATOMIC: res = atom; break;
    case node_type::CONJUNCTION: res = children[0].atom + " /\\ " + children[1].atom; break;
    case node_type::NEGATION: res = "! " + children[0].atom; break;
    case node_type::E_NEXT: res = "\\E NEXT " + children[0].atom; break;
    case node_type::E_UNTIL: res = "\\E " + children[0].atom + " U " + children[1].atom; break;
    case node_type::E_ALWAYS: res = "\\E ALWAYS " + children[0].atom; break;
  }

  res += " (" + std::to_string(counter) + ")";
  counter++;

  return res;
}

void ctlf_node::replace_subtree_by(const std::string &replacement) {
  n = node_type::ATOMIC;
  atom = replacement;
  children.clear();
}

void ctlf_node::dump() const {
  switch(n) {
    case node_type::TRUE: std::cout << "true"; break;
    case node_type::ATOMIC: std::cout << atom; break;
    case node_type::CONJUNCTION:
      std::cout << "(";
      children[0].dump();
      std::cout << ") /\\ (";
      children[1].dump();
      std::cout << ")";
      break;
    case node_type::NEGATION:
      std::cout << "!(";
      children[0].dump();
      std::cout << ")";
      break;
    case node_type::E_NEXT:
      std::cout << "\\E \\X (";
      children[0].dump();
      std::cout << ")";
      break;
    case node_type::E_UNTIL:
      std::cout << "\\E (";
      children[0].dump();
      std::cout << ") \\U (";
      children[1].dump();
      std::cout << ")";
      break;
    case node_type::E_ALWAYS:
      std::cout << "\\E \\G (";
      children[0].dump();
      std::cout << ")";
      break;
  }
}

void ctlf_node::dump_tree(size_t d) const {
  for(size_t i = 0; i < d; i++) std::cout << "|  ";
  std::cout << "+-> " << n;
  if(!atom.empty()) std::cout << " (" << atom << ")";
  std::cout << "\n";
  for(const auto &c: children) c.dump_tree(d + 1);
}