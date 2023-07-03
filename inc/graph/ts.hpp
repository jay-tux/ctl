//
// Created by jay on 6/30/23.
//

#ifndef CTL_TS_HPP
#define CTL_TS_HPP

#include <vector>
#include <string>
#include <concepts>
#include <unordered_set>

namespace ctl::graph {
using prop = std::string;
template <typename N>
concept TS_node = requires(const typename N::ts_t &ts, N node, const N &cn, std::string p) {
  { node.name() } -> std::same_as<const std::string &>;
  { node.props() } -> std::same_as<const std::unordered_set<prop> &>;
  { cn.post_in(ts) } -> std::same_as<std::vector<const N *>>;
  { cn.pre_in(ts) } -> std::same_as<std::vector<const N *>>;
  { node.add_prop(p) } -> std::same_as<void>;
};

template <typename T>
concept TS = requires(std::string &&n, std::unordered_set<prop> &&p, size_t s, T t, const T &ct, bool init) {
  requires std::default_initializable<T>;
  requires TS_node<typename T::node>;
  { t.add(std::move(n), std::move(p), init, false) } -> std::same_as<size_t>;
  { t.add_transition(s, s) } -> std::same_as<void>;
  { t.all_nodes() } -> std::same_as<std::vector<typename T::node>>;
  { ct.all_nodes() } -> std::same_as<const std::vector<typename T::node> &>;
  { ct.initial_nodes() } -> std::same_as<std::unordered_set<const typename T::node *>>;
};

class dense_ts;

class sparse_ts {
public:
  class node {
  public:
    using ts_t = sparse_ts;

    inline node(std::string &&name, std::unordered_set<prop> &&prop) : nm{std::move(name)}, ap{std::move(prop)} {}
    [[nodiscard]] constexpr const std::string &name() const { return nm; }
    [[nodiscard]] constexpr const std::unordered_set<prop> &props() const { return ap; }
    [[nodiscard]] std::vector<const node *> post_in(const sparse_ts &ts) const;
    [[nodiscard]] std::vector<const node *> pre_in(const sparse_ts &ts) const;
    inline void add_prop(const prop &p) { ap.insert(p); }

  private:
    std::string nm;
    std::unordered_set<prop> ap;
    std::vector<size_t> transitions;
    std::vector<size_t> incoming_transitions;

    friend sparse_ts;
  };

  inline sparse_ts() = default;
  size_t add(std::string &&name, std::unordered_set<prop> &&ap, bool is_initial, bool is_accepting);
  void add_transition(size_t start, size_t end);
  inline std::vector<node> all_nodes() { return nodes; }
  constexpr const std::vector<node> &all_nodes() const { return nodes; }
  std::unordered_set<const node *> initial_nodes() const;

  [[nodiscard]] dense_ts make_dense() const;
  void dump() const;

private:
  std::vector<node> nodes;
  std::unordered_set<size_t> initial_states;
  std::unordered_set<size_t> accepting_states;
};

static_assert(TS_node<sparse_ts::node>);
static_assert(TS<sparse_ts>);

class dense_ts {
public:
  class node {
  public:
    using ts_t = dense_ts;
    inline node(std::string &&name, std::unordered_set<prop> &&prop, size_t idx) : nm{std::move(name)}, ap{std::move(prop)}, idx{idx} {}
    [[nodiscard]] constexpr const std::string &name() const { return nm; }
    [[nodiscard]] constexpr const std::unordered_set<prop> &props() const { return ap; }
    [[nodiscard]] std::vector<const node *> post_in(const dense_ts &ts) const;
    [[nodiscard]] std::vector<const node *> pre_in(const dense_ts &ts) const;
    inline void add_prop(const prop &p) { ap.insert(p); }
  private:

    std::string nm;
    std::unordered_set<prop> ap;
    size_t idx;
  };

  inline dense_ts() = default;
  size_t add(std::string &&name, std::unordered_set<prop> &&ap, bool is_initial, bool is_accepting);
  void add_transition(size_t start, size_t end);
  inline std::vector<node> all_nodes() { return nodes; }
  constexpr const std::vector<node> &all_nodes() const { return nodes; }
  std::unordered_set<const node *> initial_nodes() const;

  [[nodiscard]] sparse_ts make_sparse() const;
  void dump() const;

private:
  std::vector<node> nodes;
  std::unordered_set<size_t> initial_states;
  std::unordered_set<size_t> accepting_states;
  std::vector<std::vector<bool>> transitions;
};

static_assert(TS_node<dense_ts::node>);
static_assert(TS<dense_ts>);

using default_ts = sparse_ts;
}

#endif //CTL_TS_HPP
