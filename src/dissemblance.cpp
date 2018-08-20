// Copyright 2015 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "dissemblance.h"
#include "number.h"

#include <cassert>
#include <iostream>
#include <sstream>

using namespace dissemblance;

static const Cons* dcastCons(const std::shared_ptr<Expression>& u) {
    return u ? u->asCons() : nullptr;
}
static const Symbol* dcastSymbol(const std::shared_ptr<Expression>& u) {
    return u ? u->asSymbol() : nullptr;
}
static const NumberValue* dcastNumberValue(const std::shared_ptr<Expression>& u) {
    return u ? u->asNumberValue() : nullptr;
}

struct dissemblance::NumberValue : public Expression {
    Number value;
    NumberValue(Number v) : value(v) {}
    const NumberValue* asNumberValue() const override { return this; }
    void serialize(std::ostream* o) const override {
        return value.serialize(o);
    }
};

struct dissemblance::Symbol : public Expression {
    std::string name ;
    Symbol(const std::string& n) : name(n) {}
    const Symbol* asSymbol() const override { return this; }
    void serialize(std::ostream* o) const override { *o << name; }
};

struct dissemblance::Cons : public Expression {
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
    Cons(std::shared_ptr<Expression> l, std::shared_ptr<Expression> r)
        : left(std::move(l)), right(std::move(r)) {}
    const Cons* asCons() const override { return this; }
    void serialize(std::ostream* o) const override {
        *o << "(";
        this->innerSerialize(o);
        *o << ")";
    }
    void innerSerialize(std::ostream* o) const {
        Expression::Serialize(left.get(), o);
        const Cons* current = this;
        while (const Cons* rightCons = dcastCons(current->right)) {
            *o << " ";
            Expression::Serialize(rightCons->left.get(), o);
            current = rightCons;
        }
        if (current->right) {
            *o << " . ";
            Expression::Serialize(current->right.get(), o);
        }
    }
};

struct dissemblance::Procedure : public Expression {
public:
    const Procedure* asProcedure() const override { return this; }
    virtual std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const = 0;
};

namespace {

struct Token {
    enum Type {
        Atom,
        OpenParen,
        CloseParen,
        Dot,
        Eof,
    } type;
    Token(Type t) : type(t) {}
    std::string atom;
    Token(const std::string& s) : type(Atom), atom(s) {}
};

class Tokenizer {
    std::istream* ins;
    int nextChar;
    void read() { nextChar = ins->get(); }
public:
    Tokenizer(std::istream* i) : ins(i), nextChar(i->get()) {}
    Token::Type peek() {
        while (true) {
            switch (nextChar) {
                case EOF:
                    return Token::Eof;
                case ' ':
                case '\t':
                case '\n':
                    this->read();
                    break;
                case '(':
                    return Token::OpenParen;
                case ')':
                    return Token::CloseParen;
                case '.':
                    return Token::Dot;
                default:
                    return Token::Atom;
            }
        }
    }
    Token next() {
        Token::Type type = this->peek();
        if (type == Token::Atom) {
            std::stringstream atom;
            do {
                atom << static_cast<char>(nextChar);
                this->read();
                if (EOF == nextChar ||
                    nullptr != strchr(" )(\t\n", nextChar)) {
                    return Token(atom.str());
                }
            } while (true);
        } else {
            this->read();
            return Token(type);
        }
    }
};

std::shared_ptr<Expression> MakeAtom(const std::string& s) {
    assert(s.size() > 0);
    if ('0' <= s[0] && s[0] <= '9') {
        //must be some kind of number
        return std::make_shared<NumberValue>(Number(s));
    } else {
        return std::make_shared<Symbol>(s);
    }
}

std::shared_ptr<Expression> parse_expression(Tokenizer*);

std::shared_ptr<Expression> parse_list(Tokenizer* tokenizer) {
    std::shared_ptr<Expression> left, right;
    Token token = tokenizer->next();
    switch (token.type) {
        case Token::Atom:
            left = MakeAtom(token.atom);
            if (Token::Dot == tokenizer->peek()) {
                tokenizer->next();
                right = parse_expression(tokenizer);
                assert(tokenizer->peek() == Token::CloseParen);
                tokenizer->next();
            } else {
                right = parse_list(tokenizer);
            }
            return std::make_shared<Cons>(std::move(left), std::move(right));
        case Token::OpenParen:
            left = parse_list(tokenizer);
            right = parse_list(tokenizer);
            return std::make_shared<Cons>(std::move(left), std::move(right));
        case Token::CloseParen:
            return nullptr;
        case Token::Eof:
        case Token::Dot:
        default:
            assert(false);
            return nullptr;
    }
}

std::shared_ptr<Expression> parse_expression(Tokenizer* tokenizer) {
    Token token = tokenizer->next();
    switch (token.type) {
        case Token::Atom:
            return MakeAtom(token.atom);
        case Token::OpenParen:
            return parse_list(tokenizer);
        case Token::Eof:
            return nullptr;
        case Token::CloseParen:
        case Token::Dot:
        default:
            assert(false);
            return nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////

int length(const std::shared_ptr<Expression>& expr, int accumulator = 0) {
    if (!expr) { return accumulator; }
    const Cons* c = dcastCons(expr);
    if (!c) {
        return -1; // not well formed list
    }
    return length(c->right, 1 + accumulator);
}

const std::shared_ptr<Expression>& get_item(
        const std::shared_ptr<Expression>& expr, int index) {
    const Cons* cons = dcastCons(expr);
    assert(cons);
    if (0 == index) {
        return cons->left;
    } else {
        return get_item(cons->right, index - 1);
    }
}

const std::string& get_symbol(const std::shared_ptr<Expression>& expr) {
    const Symbol* symbol = dcastSymbol(expr);
    if (!symbol) {
        std::cerr << "missing symbol: ";
        Expression::Serialize(expr.get(), &std::cerr);
        std::cerr << "\n";
    }
    assert(symbol);
    return symbol->name;
}


class Quote : public Procedure {
public:
    void serialize(std::ostream* o) const override { *o << "quote"; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        assert(1 == length(expr));
        return get_item(expr, 0);
    }
};

class List : public Procedure {
    void serialize(std::ostream* o) const override { *o << "list"; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        std::shared_ptr<Cons> top;
        if (const Cons* c = dcastCons(expr)) {
            top = std::make_shared<Cons>(Eval(c->left, env), nullptr);
            Cons* ptr = top.get();
            while (c = dcastCons(c->right)) {
                ptr->right = std::make_shared<Cons>(Eval(c->left, env), nullptr);
                ptr = (Cons*)(ptr->right.get());
            }
        }
        return top;
    }
};

class Begin : public Procedure {
public:
    void serialize(std::ostream* o) const override { *o << "begin"; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        return Begin::Beginner(expr, env);
    }

private:
    static std::shared_ptr<Expression> Beginner(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) {
        const Cons* cons = dcastCons(expr);
        assert(cons); // takes list
        auto value = Eval(cons->left, env);
        if (cons->right) {
            return Begin::Beginner(cons->right, env);
        } else {
            return std::move(value);
        }
    }
};

class LambdaProc : public Procedure {
    std::shared_ptr<Expression> parameters;
    std::shared_ptr<Expression> procedure;
    std::shared_ptr<Environment> environment;
public:
    // (lambda (x) (+ 3 x))
    // (lambda (x y) (+ 3 x y))
    LambdaProc(std::shared_ptr<Expression> expr, std::shared_ptr<Environment>& env)
        : environment(env) {
        const Cons* cons = dcastCons(expr);
        assert(cons); // takes list
        parameters = cons->left;
        assert(length(parameters) >= 0);

        assert(cons->right);
        assert(length(cons->right) >= 1);
        procedure = std::make_shared<Cons>(
                std::make_shared<Symbol>(std::string("begin")), cons->right);
    }
    void serialize(std::ostream* o) const override {
        *o << "(lambda ";
        Expression::Serialize(parameters.get(), o);
        *o << " ";
        dcastCons(dcastCons(procedure)->right)->innerSerialize(o);
        *o << ")";
    }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& arguments,
            std::shared_ptr<Environment>& env) const override {
        auto scope = std::make_shared<Environment>(environment);
        const Cons* params = dcastCons(parameters);
        const Cons* args = dcastCons(arguments);
        while (params) {
            const std::string& symb = get_symbol(params->left);
            scope->set(symb, Eval(args->left, env));
            params = dcastCons(params->right);
            args = dcastCons(args->right);
            assert((params == nullptr) == (args == nullptr));
        }
        return Eval(procedure, scope);
    }
};

class Lambda : public Procedure {
public:
    void serialize(std::ostream* o) const override { *o << "LAMBDA"; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        return std::make_shared<LambdaProc>(expr, env);
    }
};

const Number& to_number(const std::shared_ptr<Expression>& expr) {
    const NumberValue* nv = dcastNumberValue(expr);
    assert(nv);  // is a number
    return nv->value;
}

typedef Number (*BinOp)(Number, Number);

template <BinOp Op, int Identity>
class Accumulate : public Procedure {
    static Number Do(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env,
            Number accumulator) {
        const Cons* c = dcastCons(expr);
        if (!c) {
            return accumulator;
        } else {
            return Accumulate<Op, Identity>::Do(
                    c->right, env,
                    Op(accumulator, to_number(Eval(c->left, env))));
        }
    }
    const char* name;

public:
    Accumulate(const char* n) : name(n) {}
    void serialize(std::ostream* o) const override { *o << name; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        return std::make_shared<NumberValue>(
                Accumulate<Op, Identity>::Do(expr, env, Number(Identity)));
    }
};


class Subtract : public Procedure {
public:
    void serialize(std::ostream* o) const override { *o << "SUBTRACT"; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        static const Number ZERO(0);
        switch (length(expr)) {
            case 1:
                // (- value)
                return std::make_shared<NumberValue>(
                        ZERO - to_number(Eval(get_item(expr, 0), env)));
            case 2:
                return std::make_shared<NumberValue>(
                        to_number(Eval(get_item(expr, 0), env))
                        - to_number(Eval(get_item(expr, 1), env)));
            default:
                assert(false);
        }
    }
};

template <BinOp Op>
class BinaryOperation : public Procedure {
private:
    const char* name;
public:
    BinaryOperation(const char* n) : name(n) {}
    void serialize(std::ostream* o) const override { *o << name; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        assert(2 == length(expr));
        return std::make_shared<NumberValue>(
                Op(to_number(Eval(get_item(expr, 0), env)),
                   to_number(Eval(get_item(expr, 1), env))));
    }
};

typedef bool (*ComparisonOp)(Number, Number);
template <ComparisonOp Op>
class ComparisonOperation : public Procedure {
private:
    const char* name;
public:
    ComparisonOperation(const char* n) : name(n) {}
    void serialize(std::ostream* o) const override { *o << name; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        assert(2 == length(expr));
        if (Op(to_number(Eval(get_item(expr, 0), env)),
               to_number(Eval(get_item(expr, 1), env)))) {
            return std::make_shared<NumberValue>(Number(1));  // TODO: intern this.
        } else {
            return nullptr;
        }
    }
};


class If : public Procedure {
public:
    void serialize(std::ostream* o) const override { *o << "if"; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        //        0     1    2
        // (if . (cond then else))
        assert(3 == length(expr));
        if (Eval(get_item(expr, 0), env)) {
            return Eval(get_item(expr, 1), env);
        } else {
            return Eval(get_item(expr, 2), env);
        }
    }
};

class Define : public Procedure {
public:
    void serialize(std::ostream* o) const override { *o << "define"; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        //            0        1
        // (define . (variable (+ b c d))
        assert(2 == length(expr));
        env->set(get_symbol(get_item(expr, 0)), Eval(get_item(expr, 1), env));
        // todo: define procedures without lambda keyword.
        return nullptr;
    }
};

class ConsProc : public Procedure {
public:
    void serialize(std::ostream* o) const override { *o << "cons"; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        assert(2 == length(expr));
        return std::make_shared<Cons>(Eval(get_item(expr, 0), env),
                                      Eval(get_item(expr, 1), env));
    }
};

class CarProc : public Procedure {
public:
    void serialize(std::ostream* o) const override { *o << "car"; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        assert(1 == length(expr));
        auto value = Eval(get_item(expr, 0), env);
        const Cons* c = dcastCons(value);
        assert(c);
        return std::move(c->left);
    }
};

class CdrProc : public Procedure {
public:
    void serialize(std::ostream* o) const override { *o << "cdr"; }
    std::shared_ptr<Expression> eval(
            const std::shared_ptr<Expression>& expr,
            std::shared_ptr<Environment>& env) const override {
        assert(1 == length(expr));
        auto value = Eval(get_item(expr, 0), env);
        const Cons* c = dcastCons(value);
        assert(c);
        return std::move(c->right);
    }
};

struct NumberOps {
    static Number Add(Number u, Number v) { return u + v; }
    static Number Multiply(Number u, Number v) { return u * v; }
    static Number Divide(Number u, Number v) { return u / v; }
    static bool Equal(Number u, Number v) { return u == v; }
    static bool NotEqual(Number u, Number v) { return u != v; }
    static bool LessThan(Number u, Number v) { return u < v; }
    static bool GreaterThan(Number u, Number v) { return u > v; }
    static bool LessEq(Number u, Number v) { return u <= v; }
    static bool GreaterEq(Number u, Number v) { return u >= v; }
};

}  // namespace
////////////////////////////////////////////////////////////////////////////////
void dissemblance::Expression::Serialize(const Expression* expr, std::ostream* o) {
    if (expr) {
        expr->serialize(o);
    } else {
        *o << "()";
    }
}

std::shared_ptr<Expression> dissemblance::Parse(std::istream* i) {
    assert(i);
    Tokenizer tokenizer(i);
    return parse_expression(&tokenizer);
}

dissemblance::Environment::Environment(std::shared_ptr<Environment> o) : outer(std::move(o)) {}

void dissemblance::Environment::set(const std::string& s, std::shared_ptr<Expression> expr) {
    map[s] = std::move(expr);
}

std::shared_ptr<Expression> dissemblance::Environment::get(
        const std::shared_ptr<Expression>& e) const {
    const Symbol* s = dcastSymbol(e);
    return s ? this->get(s->name) : nullptr;
}

std::shared_ptr<Expression> dissemblance::Environment::get(const std::string& s) const {
    const Environment* env = this;
    do {
        const auto emap = &env->map;
        auto i = emap->find(s);
        if (i != emap->end()) {
            return i->second;
        }
        env = env->outer.get();
    } while (env);
    return nullptr;
}

std::shared_ptr<Expression> dissemblance::Eval(
        const std::shared_ptr<Expression>& expr,
        std::shared_ptr<Environment>& env) {
    if (!expr) {
        return nullptr;  // special case
    }
    if (const Symbol* symbol = dcastSymbol(expr)) {
        return env->get(symbol->name);
    }
    const Cons* cons = dcastCons(expr);
    if (!cons) {
        return expr;  // e. g. number;
    }
    auto x = Eval(cons->left, env);
    const Procedure* proc = x->asProcedure();
    if (!proc) {
        Expression::Serialize(x.get(), &std::cerr);
        std::cerr << '\n';
        assert(false);
    }
    assert(proc);
    return proc->eval(cons->right, env);
}

std::shared_ptr<Environment> dissemblance::CoreEnvironemnt() {
    std::shared_ptr<Environment> core = std::make_shared<Environment>(nullptr);
    core->set("if", std::make_shared<If>());
    core->set("define", std::make_shared<Define>());
    core->set("quote", std::make_shared<Quote>());
    core->set("+", std::make_shared<Accumulate<NumberOps::Add, 0> >("+"));
    core->set("*", std::make_shared<Accumulate<NumberOps::Multiply, 1> >("*"));
    core->set("-", std::make_shared<Subtract>());
    core->set("begin", std::make_shared<Begin>());
    core->set("lambda", std::make_shared<Lambda>());
    core->set("cons", std::make_shared<ConsProc>());
    core->set("car", std::make_shared<CarProc>());
    core->set("cdr", std::make_shared<CdrProc>());
    core->set("list", std::make_shared<List>());
    core->set("/", std::make_shared<BinaryOperation<NumberOps::Divide> >("/"));
    core->set("=", std::make_shared<ComparisonOperation<NumberOps::Equal> >("="));
    core->set("==", std::make_shared<ComparisonOperation<NumberOps::Equal> >("=="));
    core->set("!=", std::make_shared<ComparisonOperation<NumberOps::NotEqual> >("!="));
    core->set("<", std::make_shared<ComparisonOperation<NumberOps::LessThan> >("<"));
    core->set(">", std::make_shared<ComparisonOperation<NumberOps::GreaterThan> >(">"));
    core->set("<=", std::make_shared<ComparisonOperation<NumberOps::LessEq> >("<="));
    core->set(">=", std::make_shared<ComparisonOperation<NumberOps::GreaterEq> >(">="));
    return std::move(core);
}

