// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef dissemblance_DEFINED
#define dissemblance_DEFINED

// Supported operators:
//     if define quote + - * / == != < > <= >= begin lambda

#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>

namespace dissemblance {

class Expression {
public:
    static void Serialize(const Expression*, std::ostream*);
    virtual ~Expression() {}
private: 
    virtual bool isCons() const { return false; }
    virtual void serialize(std::ostream*) const = 0;
};


class Environment {
public:
    Environment(std::shared_ptr<Environment>);
    void set(const std::string& s, std::shared_ptr<Expression> expr);
    std::shared_ptr<Expression> get(const std::shared_ptr<Expression>& e) const;
    std::shared_ptr<Expression> get(const std::string& s) const;
private:
    std::unordered_map<std::string, std::shared_ptr<Expression> > map;
    std::shared_ptr<Environment> outer;
};

std::shared_ptr<Expression> Parse(std::istream*);

std::shared_ptr<Environment> CoreEnvironemnt(); 

std::shared_ptr<Expression> Eval(const std::shared_ptr<Expression>&, std::shared_ptr<Environment>&);

}

#endif  // dissemblance_DEFINED
