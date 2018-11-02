//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#ifndef LLVM_FORTH_UTIL_H
#define LLVM_FORTH_UTIL_H

#include "core.h"

namespace util {
    const static core::Func PrintFunc {
        "print", FunctionType::get(core::Builder.getVoidTy(), {core::IntType}, false)
    };
}

#endif //LLVM_FORTH_UTIL_H
