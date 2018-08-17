// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "dissemblance.h"

#include <iostream>

int main() {
    auto env = dissemblance::CoreEnvironemnt();
    while (true) {
        auto expr = dissemblance::Parse(&std::cin);
        if (expr) {
            auto val = dissemblance::Eval(expr, env);
            dissemblance::Expression::Serialize(val.get(), &std::cout);
            std::cout << std::endl;
        } else {
            break;
        }
    }
    return 0;
}
