//
// Created by jay on 6/30/23.
//


#include <unordered_map>
#include <unordered_set>
#include "util.hpp"
#include "graph/graph_reader.hpp"
#include "exceptions.hpp"

using namespace ctl;
using namespace ctl::graph;

std::pair<std::string, size_t> parse_node(default_ts &ts, const std::string &node_line, size_t lineno) {
  auto paren_open = split_by(node_line, '(');
  if(paren_open.size() == 1) throw parse_error("Unexpected <EOL> (missing opening parenthesis) (at line " + std::to_string(lineno) + ")");
  if(paren_open.size() > 2) throw parse_error("Unexpected `(' (at line " + std::to_string(lineno) + ")");
  auto tokens = split_ws(paren_open[0]);
  if(tokens.empty()) throw parse_error("Unexpected <EOL>, expected `INITIAL', `ACCEPTING' or <name> (NODE (INITIAL|ACCEPTING)* <name> (props)) (at line " + std::to_string(lineno) + ")");

  bool is_init = false;
  bool is_accept = false;
  bool hit_end = false;
  std::string name;
  for(const auto &tok: tokens) {
    if(!hit_end) {
      if(tok == "INITIAL") is_init = true;
      else if(tok == "ACCEPTING") is_accept = true;
      else {
        name = tok;
        hit_end = true;
      }
    }
    else if(tok.empty() || tok == "(") continue;
    else {
      throw parse_error("Unexpected token `" + tok + "', expected `(' (at line " + std::to_string(lineno) + ")");
    }
  }

  auto &r = paren_open[1];
  std::unordered_set<std::string> atomics;
  for(const auto &tok: split_by(r.substr(0, r.size() - 1), ',')) {
    auto tok_s = strip(tok);
    if(tok_s.empty()) throw parse_error("Invalid atomic proposition (empty) (at line " + std::to_string(lineno) + ")");
    atomics.insert(tok_s);
  }

  std::string n = name;
  return std::make_pair(n, ts.add(std::move(name), std::move(atomics), is_init, is_accept));
}

std::pair<std::string, std::string> parse_trans(const std::string &trans_line, size_t lineno) {
  auto tokens = split_ws(trans_line);
  if(tokens.size() != 3) throw parse_error("Invalid transition definition. Expected <start> -> <end> (at line " + std::to_string(lineno) + ")");
  if(tokens[1] != "->") throw parse_error("Unexpected token `" + tokens[1] + "'. Expected `->' (at line " + std::to_string(lineno) + ")");
  return std::make_pair(tokens[0], tokens[2]);
}

default_ts graph_reader::parse(std::istream &strm) {
  default_ts res;
  size_t lineno = 0;

  std::string line;
  std::unordered_map<std::string, size_t> nodes;

  while(!strm.eof()) {
    ++lineno;
    std::getline(strm, line);
    if(line.starts_with("// ") || line.empty()) { continue; } // comment or empty line
    else if (line.starts_with("NODE ")) {
      const auto &[n, idx] = parse_node(res, line.substr(5), lineno);
      if(nodes.contains(n)) throw parse_error("Cannot redefine node " + n + " (at line " + std::to_string(lineno) + ")");
      nodes[n] = idx;
    } else if (line.starts_with("TRANS ")) {
      const auto &[s, e] = parse_trans(line.substr(6), lineno);
      if(!nodes.contains(s)) throw parse_error("Use of undefined node `" + s + "' (at line " + std::to_string(lineno) + ")");
      if(!nodes.contains(e)) throw parse_error("Use of undefined node `" + e + "' (at line " + std::to_string(lineno) + ")");
      res.add_transition(nodes[s], nodes[e]);
    }
    else {
      auto f = line.find(' ');
      throw parse_error("Invalid command `" + line.substr(0, f) + "' (at line " + std::to_string(lineno) + ")");
    }
  }

  return res;
}