// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef dissemblance_DEFINED
#define dissemblance_DEFINED

#include <istream>
#include <memory>
#include <ostream>
#include <string>

namespace dissemblance {

struct Cons;
struct NumberValue;
struct Symbol;
struct Procedure;

class Expression {
public:
    static void Serialize(const Expression*, std::ostream*);
    virtual ~Expression() {}
    virtual const Cons* asCons() const { return nullptr; }
    virtual const NumberValue* asNumberValue() const { return nullptr; }
    virtual const Symbol* asSymbol() const { return nullptr; }
    virtual const Procedure* asProcedure() const { return nullptr; }
    virtual void serialize(std::ostream*) const = 0;
};

class Environment {
public:
    Environment();
    ~Environment();
    Environment(Environment&&);
    Environment(const Environment&);
    Environment& operator=(Environment&&);
    Environment& operator=(const Environment&);
    struct Impl;
    std::shared_ptr<Impl> impl;
};

std::shared_ptr<Expression> Parse(std::istream*);

Environment CoreEnvironemnt();

std::shared_ptr<Expression> Eval(const std::shared_ptr<Expression>&, Environment&);

}

#endif  // dissemblance_DEFINED
