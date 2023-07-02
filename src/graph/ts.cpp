//
// Created by jay on 6/30/23.
//

#include <algorithm>
#include <iostream>
#include "graph/ts.hpp"

using namespace ctl::graph;

std::vector<const sparse_ts::node *> sparse_ts::node::post_in(const sparse_ts &ts) const {
  std::vector<const sparse_ts::node *> res;
  for(const auto &next: transitions) {
    res.push_back(&ts.nodes[next]);
  }
  return res;
}

std::vector<const sparse_ts::node *> sparse_ts::node::pre_in(const sparse_ts &ts) const {
  std::vector<const sparse_ts::node *> res;
  for(const auto &next: incoming_transitions) {
    res.push_back(&ts.nodes[next]);
  }
  return res;
}

size_t sparse_ts::add(std::string &&name, std::unordered_set<prop> &&ap, bool is_initial, bool is_accepting) {
  nodes.emplace_back(std::move(name), std::move(ap));
  if(is_initial) initial_states.insert(nodes.size() - 1);
  if(is_accepting) accepting_states.insert(nodes.size() - 1);
  return nodes.size() - 1;
}

void sparse_ts::add_transition(size_t start, size_t end) {
  auto &r = nodes[start].transitions;
  if(std::find(r.begin(), r.end(), end) == r.end()) {
    r.push_back(end);
    nodes[end].incoming_transitions.push_back(start);
  }
}

std::vector<const dense_ts::node *> dense_ts::node::pre_in(const dense_ts &ts) const {
  std::vector<const dense_ts::node *> res;
  for(size_t i = 0; i < ts.transitions.size(); i++) {
    if(ts.transitions[i][idx]) res.push_back(&ts.nodes[i]);
  }
  return res;
}

std::vector<const dense_ts::node *> dense_ts::node::post_in(const dense_ts &ts) const {
  std::vector<const dense_ts::node *> res;
  auto &ref = ts.transitions[idx];
  for(size_t i = 0; i < ref.size(); i++) {
    if(ref[i]) res.push_back(&ts.nodes[i]);
  }
  return res;
}

size_t dense_ts::add(std::string &&name, std::unordered_set<prop> &&ap, bool is_initial, bool is_accepting) {
  nodes.emplace_back(std::move(name), std::move(ap), nodes.size());
  for(auto &v: transitions) { v.push_back(false); }
  transitions.emplace_back();
  transitions.back().resize(transitions.size(), false);
  if(is_initial) initial_states.insert(nodes.size() - 1);
  if(is_accepting) accepting_states.insert(nodes.size() - 1);
  return nodes.size() - 1;
}

void dense_ts::add_transition(size_t start, size_t end) {
  transitions[start][end] = true;
}

dense_ts sparse_ts::make_dense() const {
  dense_ts res;

  for(size_t i = 0; i < nodes.size(); i++) {
    const auto &n = nodes[i];
    auto name = n.name();
    auto prop = n.props();
    res.add(std::move(name), std::move(prop), initial_states.contains(i), accepting_states.contains(i));
  }

  for(size_t i = 0; i < nodes.size(); i++) {
    for(auto &sub: nodes[i].transitions) {
      res.add_transition(i, sub);
    }
  }

  return res;
}

sparse_ts dense_ts::make_sparse() const {
  sparse_ts res;

  for(size_t i = 0; i < nodes.size(); i++) {
    const auto &n = nodes[i];
    auto name = n.name();
    auto prop = n.props();
    res.add(std::move(name), std::move(prop), initial_states.contains(i), accepting_states.contains(i));
  }

  for(size_t i = 0; i < transitions.size(); i++) {
    for(size_t j = 0; j < transitions[i].size(); j++) {
      res.add_transition(i, j);
    }
  }

  return res;
}

void sparse_ts::dump() const {
  std::cout << " --- Sparse TS with " << nodes.size() << " nodes ---\n";
  for(size_t i = 0; i < nodes.size(); i++) {
    const auto &n = nodes[i];
    std::cout << "  -> Node `" << n.name() << "'.\n";
    std::cout << "    + Atomic propositions:";
    for(const auto &prop: n.props()) {
      std::cout << " " << prop;
    }
    std::cout << "\n";
    if(initial_states.contains(i)) {
      std::cout << "    + Initial state\n";
    }
    if(accepting_states.contains(i)) {
      std::cout << "    + Accepting state\n";
    }
    if(n.transitions.empty()) {
      std::cout << "    + No successors\n";
    }
    else {
      std::cout << "    + Successors: \n";
      for (const auto &j: n.transitions) {
        const auto &n2 = nodes[j];
        std::cout << "      ~> " << n2.name() << "; propositions:";
        for (const auto &p: n2.props()) {
          std::cout << " " << p;
        }
        std::cout << "\n";
      }
    }
  }
}

void dense_ts::dump() const {
  std::cout << " --- Dense TS with " << nodes.size() << " nodes ---\n";
  for(size_t i = 0; i < nodes.size(); i++) {
    const auto &n = nodes[i];
    std::cout << "  -> Node `" << n.name() << "'.\n";
    std::cout << "    + Atomic propositions:";
    for(const auto &prop: n.props()) {
      std::cout << " " << prop;
    }
    std::cout << "\n";
    if(initial_states.contains(i)) {
      std::cout << "    + Initial state\n";
    }
    if(accepting_states.contains(i)) {
      std::cout << "    + Accepting state\n";
    }
    if(std::all_of(transitions[i].begin(), transitions[i].end(), [](bool b){ return !b; })) {
      std::cout << "    + No successors\n";
    }
    else {
      std::cout << "    + Successors: \n";
      for (size_t j = 0; j < transitions[i].size(); j++) {
        if(transitions[i][j]) {
          const auto &n2 = nodes[j];
          std::cout << "      ~> " << n2.name() << "; propositions:";
          for (const auto &p: n2.props()) {
            std::cout << " " << p;
          }
          std::cout << "\n";
        }
      }
    }
  }
}

std::unordered_set<const sparse_ts::node *> sparse_ts::initial_nodes() const {
  std::unordered_set<const sparse_ts::node *> res;
  for(const auto &i: initial_states) res.insert(&nodes[i]);
  return res;
}

std::unordered_set<const dense_ts::node *> dense_ts::initial_nodes() const {
  std::unordered_set<const dense_ts::node *> res;
  for(const auto &i: initial_states) res.insert(&nodes[i]);
  return res;
}