//
// Created by jay on 7/2/23.
//

#ifndef CTL_CHECKER_HPP
#define CTL_CHECKER_HPP

#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <stack>

#include "formula/formula_parser.hpp"
#include "graph/ts.hpp"

namespace ctl::checker {
struct sat_calc {
  template <typename T> using set_t = std::unordered_set<T>;

  template <typename T>
  void set_minus_inplace(set_t<T> &one, const set_t<T> &other) {
    set_t<T> res;
    for(const auto &v: one) {
      if(!other.contains(v)) res.insert(v);
    }

    res.swap(one);
  }

  template <typename T>
  set_t<T> intersect_set(const set_t<T> &one, const set_t<T> &other) {
    set_t<T> res;
    for(const auto v: other) {
      if(one.contains(v)) res.insert(v);
    }
    return res;
  }

  template <typename T, template <typename> typename C>
  set_t<const T *> intersect(const set_t<const T *> &one, const C<const T *> &other) {
    set_t<const T *> res;
    for(const auto v: other) {
      if(one.contains(v)) res.insert(v);
    }
    return res;
  }

  template <graph::TS TS>
  set_t<const typename TS::node *> sat_atom(const std::string &atom, const TS &ts) {
    using N = typename TS::node;
    set_t<const N *> res;
    for(const N &node: ts.all_nodes()) {
      const auto &r = node.props();
      if(r.contains(atom)) res.insert(&node);
    }
    return res;
  }

  template <graph::TS TS>
  set_t<const typename TS::node *> sat_not_atom(const std::string &atom, const TS &ts) {
    using N = typename TS::node;
    set_t<const N *> res;
    for(const N &node: ts.all_nodes()) {
      const auto &r = node.props();
      if(!r.contains(atom)) res.insert(&node);
    }
    return res;
  }

  template <graph::TS TS>
  set_t<const typename TS::node *> sat_conjunction(const std::string &a1, const std::string &a2, const TS &ts) {
    auto s1 = sat_atom(a1, ts);
    auto s2 = sat_atom(a2, ts);
    return intersect_set(s1, s2);
  }

  template <graph::TS TS>
  set_t<const typename TS::node *> sat_e_next(const std::string &next, const TS &ts) {
    auto s1 = sat_atom(next, ts);
    set_t<const typename TS::node *> res;
    for(const auto &node: ts.all_nodes()) {
      auto succ = node.post_in(ts);
      if(std::any_of(succ.begin(), succ.end(), [&s1](const auto &v){ return s1.contains(v); })) res.insert(&node);
    }
    return res;
  }

  template <graph::TS TS>
  set_t<const typename TS::node *> sat_e_until(const std::string &pre, const std::string &post, const TS &ts) {
    set_t<const typename TS::node *> res = sat_atom(post, ts);
    set_t<const typename TS::node *> restriction = sat_atom(pre, ts);

    set_t<const typename TS::node *> add;
    for(const auto &n: res) {
      auto in = intersect(restriction, n->pre_in(ts));
      add.insert(in.begin(), in.end());
    }
    set_minus_inplace(add, res);

    while(!add.empty()) {
      res.insert(add.begin(), add.end());
      decltype(add) cp;
      add.swap(cp);

      for(const auto &n: cp) {
        auto in = intersect(restriction, n->pre_in(ts));
        add.insert(in.begin(), in.end());
      }

      set_minus_inplace(add, res);
    }

    return res;
  }

  template <graph::TS TS>
  set_t<const typename TS::node *> sat_e_always(const std::string &atom, const TS &ts) {
    set_t<const typename TS::node *> res = sat_atom(atom, ts);
    std::unordered_map<const typename TS::node *, size_t> c;
    for(const auto v: res) {
      c[v] = intersect(res, v->post_in(ts)).size();
    }
    decltype(res) e;
    for(const auto &[k, v]: c) {
      if(v == 0) e.insert(k);
    }

    while(!e.empty()) {
      const typename TS::node *n = *e.begin();
      res.erase(n);
      e.erase(n);

      for(const auto &p: intersect(res, n->pre_in(ts))) {
        size_t count = --c[p];
        if(count == 0) e.insert(p);
      }
    }

    return res;
  }

  template <graph::TS TS>
  set_t<const typename TS::node *> sat(formula::ctlf_node formula, TS &ts) {
    std::stack<formula::ctlf_node *> backtrack;
    backtrack.push(&formula);

    auto mod_sat = [&ts](formula::ctlf_node *ptr, set_t<const typename TS::node *> s) {
      std::string name = ptr->generate_var();

      ptr->replace_subtree_by(name);
      for(auto *ptr: s) {
        const_cast<typename TS::node *>(ptr)->add_prop(name);
      }
    };

    while(!backtrack.empty()) {
      formula::ctlf_node *curr = backtrack.top();
      bool can_check = true;
      for(auto &c: curr->children) {
        if(c.n != formula::node_type::ATOMIC && c.n != formula::node_type::TRUE) {
          can_check = false;
          backtrack.push(&c);
        }
      }

      if(can_check) {
        switch(curr->n) {
          case formula::node_type::TRUE:
          case formula::node_type::ATOMIC:
            break;
          case formula::node_type::CONJUNCTION:
            mod_sat(curr, sat_conjunction(curr->children[0].atom, curr->children[1].atom, ts));
            break;
          case formula::node_type::NEGATION:
            mod_sat(curr, sat_not_atom(curr->children[0].atom, ts));
            break;
          case formula::node_type::E_NEXT:
            mod_sat(curr, sat_e_next(curr->children[0].atom, ts));
            break;
          case formula::node_type::E_UNTIL:
            mod_sat(curr, sat_e_until(curr->children[0].atom, curr->children[1].atom, ts));
            break;
          case formula::node_type::E_ALWAYS:
            mod_sat(curr, sat_e_always(curr->children[0].atom, ts));
            break;
        }

        backtrack.pop();
      }
    }

    return sat_atom(formula.atom, ts);
  }

  template <graph::TS TS>
  bool models(const TS &ts, const formula::ctlf_node &formula) {
    formula::ctlf_node dup_f = formula;
    TS dup_ts = ts;
    auto sat_nodes = sat(dup_f, dup_ts);
    auto init_nodes = dup_ts.initial_nodes();
    for(const auto &n: init_nodes) {
      if(sat_nodes.contains(n)) return true;
    }
    return false;
  }
};
}

#endif //CTL_CHECKER_HPP
