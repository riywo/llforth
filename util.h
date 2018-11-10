//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#ifndef LLVM_FORTH_UTIL_H
#define LLVM_FORTH_UTIL_H

#include "core.h"

namespace util {
    const static core::Func PrintIntFunc {
        "print_int", FunctionType::get(core::Builder.getVoidTy(), {core::IntType}, false)
    };

    const static core::Func PrintStrFunc {
        "print_str", FunctionType::get(core::Builder.getVoidTy(), {core::StrType}, false)
    };

    const static core::Func ReadWordFunc {
        "read_word", FunctionType::get(core::IntType, {core::StrType}, false)
    };

    static void Initialize() {
        core::Func printf = {
                "printf", FunctionType::get(core::IntType, {core::StrType}, true)
        };
        core::Func getchar = {
                "getchar", FunctionType::get(core::CharType, {}, false)
        };
        core::CreateFunction(PrintIntFunc, [=](Function* f){
            auto arg = f->arg_begin();
            auto fmt = core::Builder.CreateGlobalStringPtr("%d ");
            core::CallFunction(printf, {fmt, arg});
            core::Builder.CreateRetVoid();
        });
        core::CreateFunction(PrintStrFunc, [=](Function* f){
            auto arg = f->arg_begin();
            auto fmt = core::Builder.CreateGlobalStringPtr("%s ");
            core::CallFunction(printf, {fmt, arg});
            core::Builder.CreateRetVoid();
        });
        core::CreateFunction(ReadWordFunc, [=](Function* f) {
            auto arg = f->arg_begin();
            auto c = core::CallFunction(getchar, {});
            core::Builder.CreateStore(c, arg);
            auto next = core::Builder.CreateGEP(arg, core::GetInt(1));
            core::Builder.CreateStore(ConstantInt::get(core::CharType, 0), next);
            core::Builder.CreateRet(core::GetInt(1));
        });
    };
}

#endif //LLVM_FORTH_UTIL_H
