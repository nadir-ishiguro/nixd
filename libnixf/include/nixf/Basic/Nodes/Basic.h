#pragma once

#include "nixf/Basic/Range.h"
#include "nixf/Basic/UniqueOrRaw.h"

#include <boost/container/small_vector.hpp>

#include <cassert>
#include <string>

namespace nixf {

class Node;

class HaveSyntax {
public:
  virtual ~HaveSyntax() = default;

  /// \brief The syntax node before lowering.
  ///
  /// Nullable, because nodes may be created implicitly (e.g. nested keys).
  [[nodiscard]] virtual const Node *syntax() const = 0;
};

/// \brief Evaluable nodes, after lowering.
///
/// In libnixf, we do not evaluate the code actually, instead they will
/// be serialized into byte-codes, and then evaluated by C++ nix, via IPC.
class Evaluable : public HaveSyntax {};

class Node {
public:
  enum NodeKind {
#define NODE(NAME) NK_##NAME,
#include "nixf/Basic/NodeKinds.inc"
#undef NODE
    NK_BeginExpr,
#define EXPR(NAME) NK_##NAME,
#include "nixf/Basic/NodeKinds.inc"
#undef EXPR
    NK_EndExpr,
  };

private:
  NodeKind Kind;
  LexerCursorRange Range;

protected:
  explicit Node(NodeKind Kind, LexerCursorRange Range)
      : Kind(Kind), Range(Range) {}

public:
  [[nodiscard]] NodeKind kind() const { return Kind; }
  [[nodiscard]] LexerCursorRange range() const { return Range; }
  [[nodiscard]] PositionRange positionRange() const { return Range.range(); }
  [[nodiscard]] LexerCursor lCur() const { return Range.lCur(); }
  [[nodiscard]] LexerCursor rCur() const { return Range.rCur(); }
  [[nodiscard]] static const char *name(NodeKind Kind);
  [[nodiscard]] const char *name() const { return name(Kind); }

  using ChildVector = boost::container::small_vector<Node *, 8>;

  [[nodiscard]] virtual ChildVector children() const = 0;

  virtual ~Node() = default;

  /// \brief Descendant node that contains the given range.
  [[nodiscard]] const Node *descend(PositionRange Range) const {
    if (!positionRange().contains(Range)) {
      return nullptr;
    }
    for (const auto &Child : children()) {
      if (!Child)
        continue;
      if (Child->positionRange().contains(Range)) {
        return Child->descend(Range);
      }
    }
    return this;
  }

  [[nodiscard]] std::string_view src(std::string_view Src) const {
    auto Begin = lCur().offset();
    auto Length = rCur().offset() - Begin;
    return Src.substr(Begin, Length);
  }
};

class Expr : public Node, public Evaluable {
protected:
  explicit Expr(NodeKind Kind, LexerCursorRange Range) : Node(Kind, Range) {
    assert(NK_BeginExpr <= Kind && Kind <= NK_EndExpr);
  }

public:
  [[nodiscard]] const Node *syntax() const override { return this; }

  static bool classof(const Node *N) { return isExpr(N->kind()); }

  static bool isExpr(NodeKind Kind) {
    return NK_BeginExpr <= Kind && Kind <= NK_EndExpr;
  }

  /// \returns true if the expression might be evaluated to lambda.
  static bool maybeLambda(NodeKind Kind) {
    if (!isExpr(Kind))
      return false;
    switch (Kind) {
    case Node::NK_ExprInt:
    case Node::NK_ExprFloat:
    case Node::NK_ExprAttrs:
    case Node::NK_ExprString:
    case Node::NK_ExprPath:
      return false;
    default:
      return true;
    }
  }

  [[nodiscard]] bool maybeLambda() const { return maybeLambda(kind()); }
};

/// \brief Misc node, used for parentheses, keywords, etc.
///
/// This is used for representing nodes that only location matters.
/// Might be useful for linting.
class Misc : public Node {
public:
  Misc(LexerCursorRange Range) : Node(NK_Misc, Range) {}

  [[nodiscard]] ChildVector children() const override { return {}; }
};

/// \brief Identifier. Variable names, attribute names, etc.
class Identifier : public Node {
  std::string Name;

public:
  Identifier(LexerCursorRange Range, std::string Name)
      : Node(NK_Identifer, Range), Name(std::move(Name)) {}
  [[nodiscard]] const std::string &name() const { return Name; }

  [[nodiscard]] ChildVector children() const override { return {}; }
};

} // namespace nixf
