#include <iostream>
#include <fstream>
#include "graph/graph_reader.hpp"
#include "formula/formula_parser.hpp"
#include "checker/checker.hpp"

int main(int argc, const char **argv) {
  if(argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <input graph file> <input formula file>\n";
    return -1;
  }

  std::ifstream strm(argv[1]);
  if(!strm.good()) {
    std::cerr << "Error: can't open file " << argv[1] << " for reading.\n";
    return -2;
  }

  ctl::graph::default_ts ts;
  try {
    ts = ctl::graph::graph_reader::parse(strm);
  }
  catch(const std::exception &exc) {
    std::cerr << "Error while parsing: " << exc.what() << "\n";
    return -3;
  }

  strm = std::ifstream(argv[2]);
  if(!strm.good()) {
    std::cerr << "Error: can't open file " << argv[2] << " for reading.\n";
    return -2;
  }

  ctl::formula::ctlf_node formula;
  try {
    formula = ctl::formula::parser::parse(strm);
  }
  catch(const std::exception &exc) {
    std::cerr << "Error while parsing: " << exc.what() << "\n";
    return -3;
  }

  ctl::checker::sat_calc calc;
  ctl::graph::sparse_ts ts2 = ts;

  auto sat = calc.sat(formula, ts2);

  std::cout << "SAT(";
  formula.dump();
  std::cout << ") = {\n";
  for(const auto &node: sat) {
    std::cout << "  node(" << node->name() << ", { ... })\n";
  }
  std::cout << "}\n";

  if(calc.models(ts2, formula)) std::cout << "M ⊨ phi\n";
  else std::cout << "M ⊭ phi \n";
}
