#pragma once

#include "Attrs.h"

namespace nixf {

class ExprSelect : public Expr {
  std::unique_ptr<Expr> E;
  std::unique_ptr<AttrPath> Path;
  std::unique_ptr<Expr> Default;

public:
  ExprSelect(LexerCursorRange Range, std::unique_ptr<Expr> E,
             std::unique_ptr<AttrPath> Path, std::unique_ptr<Expr> Default)
      : Expr(NK_ExprSelect, Range), E(std::move(E)), Path(std::move(Path)),
        Default(std::move(Default)) {
    assert(this->E && "E must not be null");
  }

  [[nodiscard]] Expr &expr() const {
    assert(E && "E must not be null");
    return *E;
  }

  [[nodiscard]] Expr *defaultExpr() const { return Default.get(); }

  [[nodiscard]] AttrPath *path() const { return Path.get(); }

  [[nodiscard]] ChildVector children() const override {
    return {E.get(), Path.get()};
  }
};

/// A call/apply to some function.
class ExprCall : public Expr {
  std::unique_ptr<Expr> Fn;
  std::vector<std::unique_ptr<Expr>> Args;

public:
  ExprCall(LexerCursorRange Range, std::unique_ptr<Expr> Fn,
           std::vector<std::unique_ptr<Expr>> Args)
      : Expr(NK_ExprCall, Range), Fn(std::move(Fn)), Args(std::move(Args)) {
    assert(this->Fn && "Fn must not be null");
  }

  [[nodiscard]] Expr &fn() const {
    assert(Fn && "Fn must not be null");
    return *Fn;
  }
  std::vector<std::unique_ptr<Expr>> &args() { return Args; }

  [[nodiscard]] const std::vector<std::unique_ptr<Expr>> &args() const {
    return Args;
  }

  [[nodiscard]] ChildVector children() const override {
    ChildVector Children;
    Children.reserve(Args.size());
    for (const auto &Member : Args) {
      Children.emplace_back(Member.get());
    }
    return Children;
  }
};

class ExprList : public Expr {
  std::vector<std::unique_ptr<Expr>> Elements;

public:
  ExprList(LexerCursorRange Range, std::vector<std::unique_ptr<Expr>> Elements)
      : Expr(NK_ExprList, Range), Elements(std::move(Elements)) {}

  [[nodiscard]] const std::vector<std::unique_ptr<Expr>> &elements() const {
    return Elements;
  }

  [[nodiscard]] std::vector<std::unique_ptr<Expr>> &elements() {
    return Elements;
  }

  [[nodiscard]] ChildVector children() const override {
    ChildVector Children;
    Children.reserve(Elements.size());
    for (const auto &Element : Elements) {
      Children.emplace_back(Element.get());
    }
    return Children;
  }
};

class ExprIf : public Expr {
  std::unique_ptr<Expr> Cond;
  std::unique_ptr<Expr> Then;
  std::unique_ptr<Expr> Else;

public:
  ExprIf(LexerCursorRange Range, std::unique_ptr<Expr> Cond,
         std::unique_ptr<Expr> Then, std::unique_ptr<Expr> Else)
      : Expr(NK_ExprIf, Range), Cond(std::move(Cond)), Then(std::move(Then)),
        Else(std::move(Else)) {}

  [[nodiscard]] Expr *cond() const { return Cond.get(); }
  [[nodiscard]] Expr *then() const { return Then.get(); }
  [[nodiscard]] Expr *elseExpr() const { return Else.get(); }

  [[nodiscard]] ChildVector children() const override {
    return {Cond.get(), Then.get(), Else.get()};
  }
};

class ExprAssert : public Expr {
  std::unique_ptr<Expr> Cond;
  std::unique_ptr<Expr> Value; // If "cond" is true, then "value" is returned.

public:
  ExprAssert(LexerCursorRange Range, std::unique_ptr<Expr> Cond,
             std::unique_ptr<Expr> Value)
      : Expr(NK_ExprAssert, Range), Cond(std::move(Cond)),
        Value(std::move(Value)) {}

  [[nodiscard]] Expr *cond() const { return Cond.get(); }
  [[nodiscard]] Expr *value() const { return Value.get(); }

  [[nodiscard]] ChildVector children() const override {
    return {Cond.get(), Value.get()};
  }
};

class ExprLet : public Expr {
  // 'let' binds 'in' expr

  std::unique_ptr<Misc> KwLet; // 'let', not null
  std::unique_ptr<Binds> B;
  std::unique_ptr<Misc> KwIn;
  std::unique_ptr<Expr> E;

public:
  ExprLet(LexerCursorRange Range, std::unique_ptr<Misc> KwLet,
          std::unique_ptr<Binds> B, std::unique_ptr<Misc> KwIn,
          std::unique_ptr<Expr> E)
      : Expr(NK_ExprLet, Range), KwLet(std::move(KwLet)), B(std::move(B)),
        KwIn(std::move(KwIn)), E(std::move(E)) {
    assert(this->KwLet && "KwLet should not be empty!");
  }

  [[nodiscard]] Binds *binds() const { return B.get(); }
  [[nodiscard]] Expr *expr() const { return E.get(); }
  [[nodiscard]] Misc &let() const { return *KwLet; }
  [[nodiscard]] Misc *in() const { return KwIn.get(); }

  [[nodiscard]] ChildVector children() const override {
    return {KwLet.get(), B.get(), KwIn.get(), E.get()};
  }
};

class ExprWith : public Expr {
  std::unique_ptr<Expr> With;
  std::unique_ptr<Expr> E;

public:
  ExprWith(LexerCursorRange Range, std::unique_ptr<Expr> With,
           std::unique_ptr<Expr> E)
      : Expr(NK_ExprWith, Range), With(std::move(With)), E(std::move(E)) {}

  [[nodiscard]] Expr *with() const { return With.get(); }
  [[nodiscard]] Expr *expr() const { return E.get(); }

  [[nodiscard]] ChildVector children() const override {
    return {With.get(), E.get()};
  }
};

} // namespace nixf
