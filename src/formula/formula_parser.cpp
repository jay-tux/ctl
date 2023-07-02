//
// Created by jay on 6/30/23.
//

#include <stack>
#include <variant>
#include <util.hpp>

#include "formula/formula.hpp"
#include "formula/formula_parser.hpp"
#include "exceptions.hpp"

using namespace ctl;
using namespace ctl::formula;

#define TOKENS X(TRUE) X(ATOM) X(AND) X(NOT) X(EXISTS) X(NEXT) X(UNTIL) X(GLOBALLY) X(PAR_OPEN) X(PAR_CLOSE) X(IGNORE)

enum struct token_kind {
#define X(t) t,
  TOKENS
#undef X
};

std::string name(const token_kind &k) {
  switch(k) {
#define X(t) case token_kind::t: return #t;
    TOKENS
#undef X
  }
  return "";
}

std::ostream &operator<<(std::ostream &strm, const token_kind &tk) {
  switch(tk) {
#define X(t) case token_kind::t: return strm << #t;
    TOKENS
#undef X
  }
  return strm;
}

struct token {
  token_kind kind;
  std::string content;
};

std::ostream &operator<<(std::ostream &strm, const token &t) {
  strm << t.kind;
  if(!t.content.empty()) strm << " (" << t.content << ")";
  return strm;
}

std::vector<token> lex(std::istream &strm) {
  std::vector<token> res;
  bool was_bsl = false;
  bool was_fsl = false;
  bool building = false;
  std::string curr;

  while(int c = strm.get()) {
    if(c == -1) break;

    if(was_bsl) {
      switch(c) {
        case 'E': res.push_back({ token_kind::EXISTS, "" }); break;
        case 'X': res.push_back({ token_kind::NEXT, "" }); break;
        case 'G': res.push_back({ token_kind::GLOBALLY, "" }); break;
        case 'U': res.push_back({ token_kind::UNTIL, "" }); break;
        default: throw parse_error("Invalid token \\" + std::to_string(c) + ".");
      }
      was_bsl = false;
    }
    else if(was_fsl) {
      if(c == '\\') res.push_back({ token_kind::AND, "" });
      else throw parse_error("Invalid token /" + std::string(1, (char)c) + ".");
      was_fsl = false;
    }
    else if(building) {
      if(!isalnum(c) && c != '_') {
        building = false;
        if(curr == "true" || curr == "True" || curr == "TRUE") res.push_back({ token_kind::TRUE, "" });
        else res.push_back({ token_kind::ATOM, curr });
        strm.putback((char)c);
      }
      else curr += (char)c;
    }
    else {
      if(isspace(c)) continue;

      switch(c) {
        case '\\': was_bsl = true; break;
        case '(': res.push_back({ token_kind::PAR_OPEN, "" }); break;
        case ')': res.push_back({ token_kind::PAR_CLOSE, "" }); break;
        case '/': was_fsl = true; break;
        case '!': res.push_back({ token_kind::NOT, "" }); break;
        default:
          if(isalnum(c) || c == '_') {
            building = true;
            curr = (char) c;
          }
          else throw parse_error("Invalid start of token " + std::string(1, (char)c) + ".");
          break;
      }
    }
  }

  if(building) res.push_back({ token_kind::ATOM, curr });

  bool was_exists = false;
  for(size_t idx = 0; idx < res.size(); idx++) {
    switch(res[idx].kind) {
      case token_kind::EXISTS:
        was_exists = true;
        break;
      case token_kind::NEXT:
        if(!was_exists) throw parse_error("Encountered \\X without preceding \\E.");
        res[idx - 1].kind = token_kind::IGNORE;
        was_exists = false;
        break;
      case token_kind::GLOBALLY:
        if(!was_exists) throw parse_error("Encountered \\G without preceding \\E.");
        res[idx - 1].kind = token_kind::IGNORE;
        was_exists = false;
        break;
      default:
        was_exists = false;
        break;
    }
  }
  return res;
}

int prio(const token_kind t) {
  switch (t) {
    case token_kind::TRUE:
    case token_kind::ATOM:
    case token_kind::IGNORE:
    case token_kind::PAR_CLOSE:
    case token_kind::PAR_OPEN:
    case token_kind::EXISTS:
      return -1;
    case token_kind::UNTIL:
      return 1;
    case token_kind::AND:
      return 2;
    case token_kind::NEXT:
    case token_kind::GLOBALLY:
    case token_kind::NOT:
      return 3;
  }

  return -1;
}

int argc(const token_kind t) {
  switch(t) {
    case token_kind::TRUE:
    case token_kind::ATOM:
      return 0;
    case token_kind::NOT:
    case token_kind::NEXT:
    case token_kind::GLOBALLY:
    case token_kind::PAR_OPEN:
    case token_kind::PAR_CLOSE:
      return 1;
    case token_kind::AND:
    case token_kind::EXISTS:
    case token_kind::UNTIL:
      return 2;
    case token_kind::IGNORE:
      return -1;
  }
  return -1;
}

node_type to_node_type(const token_kind tk) {
  switch(tk) {
    case token_kind::TRUE: return node_type::TRUE;
    case token_kind::ATOM: return node_type::ATOMIC;
    case token_kind::AND: return node_type::CONJUNCTION;
    case token_kind::NOT: return node_type::NEGATION;
    case token_kind::EXISTS: return node_type::E_UNTIL;
    case token_kind::NEXT: return node_type::E_NEXT;
    case token_kind::UNTIL: return node_type::E_UNTIL;
    case token_kind::GLOBALLY: return node_type::E_ALWAYS;
    case token_kind::PAR_OPEN:
    case token_kind::PAR_CLOSE:
    case token_kind::IGNORE:
      return node_type::TRUE;
  }

  return node_type::TRUE;
}

ctlf_node parser::parse(std::istream &strm) {
  // \E (\E \X (p /\ True) /\ ! \E \G r) \U (\E \X r /\ s)
  auto tokens = lex(strm);
  std::vector<ctlf_node> tok_stack;
  std::vector<token_kind> operator_stack;

  auto apply = [&tok_stack](token_kind op){

    if(tok_stack.size() < (size_t)argc(op)) throw parse_error("Not enough arguments to pop for operator " + name(op) + " (" + std::to_string(tok_stack.size()) + "/" + std::to_string(argc(op)) + ").");
    if(op == token_kind::PAR_CLOSE || op == token_kind::PAR_OPEN) return;

    std::vector<ctlf_node> args;
    for(int i = 0; i < argc(op); i++) {
      auto top = tok_stack.back();
      args.push_back(top);
      tok_stack.pop_back();
    }
    auto temp = std::vector<ctlf_node>{ args.rbegin(), args.rend() };
    tok_stack.push_back(ctlf_node{ .n = to_node_type(op), .atom = "", .children = temp });
  };

  for(const auto &token: tokens) {
    switch(token.kind) {
      case token_kind::TRUE:
        tok_stack.push_back({ .n = node_type::TRUE, .atom = "true", .children = {} });
        break;
      case token_kind::ATOM:
        tok_stack.push_back({ .n = node_type::ATOMIC, .atom = token.content, .children = {} });
        break;
      case token_kind::IGNORE:
        break;
      case token_kind::PAR_OPEN:
      case token_kind::EXISTS:
        operator_stack.push_back(token.kind);
        break;
      case token_kind::PAR_CLOSE:
        while(!operator_stack.empty() && operator_stack.back() != token_kind::PAR_OPEN) {
          apply(operator_stack.back());
          operator_stack.pop_back();
        }
        operator_stack.pop_back();
        break;
      case token_kind::UNTIL:
        while(!operator_stack.empty() && operator_stack.back() != token_kind::EXISTS) {
          apply(operator_stack.back());
          operator_stack.pop_back();
        }
        operator_stack.pop_back();
        operator_stack.push_back(token.kind);
        break;
      default:
        while(!operator_stack.empty() && operator_stack.back() != token_kind::PAR_OPEN && prio(operator_stack.back()) >= prio(token.kind)) {
          apply(operator_stack.back());
          operator_stack.pop_back();
        }
        if(token.kind != token_kind::UNTIL && token.kind != token_kind::PAR_CLOSE)
          operator_stack.push_back(token.kind);
        break;
    }
  }

  while(!operator_stack.empty()) {
    apply(operator_stack.back());
    operator_stack.pop_back();
  }

  if(tok_stack.size() != 1) throw parse_error("Not exactly 1 element left after parsing.");

  return tok_stack.back();
}

/*
 expr::= True
       | IDENT[id]
       | ! <expr>
       | \E \X <expr>
       | \E \G <expr>
       | <expr> /\ <expr>
       | \E <expr> \U <expr>
       | ( <expr> )



 */