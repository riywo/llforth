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

    static void Initialize() {
        core::Func _printf = {
                "printf", FunctionType::get(core::IntType, {core::StrType}, true)
        };
        core::CreateFunction(PrintFunc, [=](Function* f){
            auto arg = f->arg_begin();
            auto d = core::Builder.CreateGlobalStringPtr("%d ");
            core::CallFunction(_printf, {d, arg});
            core::Builder.CreateRetVoid();
        });
    };
}

#endif //LLVM_FORTH_UTIL_H
