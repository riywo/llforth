//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#ifndef LLVM_FORTH_UTIL_H
#define LLVM_FORTH_UTIL_H

#include "core.h"

namespace util {
    auto NullChar = ConstantInt::get(core::CharType, 0);

    const static core::Func PrintIntFunc {
        "print_int", FunctionType::get(core::VoidType, {core::IntType}, false)
    };
    const static core::Func PrintStrFunc {
        "print_str", FunctionType::get(core::VoidType, {core::StrType}, false)
    };
    const static core::Func ReadWordFunc {
        "read_word", FunctionType::get(core::IntType, {core::StrType}, false)
    };
    const static core::Func SkipCommentFunc {
        "skip_comment", FunctionType::get(core::VoidType, {}, false)
    };

    static void Initialize() {
        core::Func printf = {
                "printf", FunctionType::get(core::IntType, {core::StrType}, true)
        };
        core::Func getchar = {
                "getchar", FunctionType::get(core::CharType, {}, false)
        };
        core::CreateFunction(PrintIntFunc, [=](Function* f, BasicBlock* entry){
            auto arg = f->arg_begin();
            auto fmt = core::Builder.CreateGlobalStringPtr("%d ");
            core::CallFunction(printf, {fmt, arg});
            core::Builder.CreateRetVoid();
        });
        core::CreateFunction(PrintStrFunc, [=](Function* f, BasicBlock* entry){
            auto arg = f->arg_begin();
            auto fmt = core::Builder.CreateGlobalStringPtr("%s ");
            core::CallFunction(printf, {fmt, arg});
            core::Builder.CreateRetVoid();
        });
        core::CreateFunction(ReadWordFunc, [=](Function* f, BasicBlock* entry) {
            auto arg = f->arg_begin();
            auto loop = core::CreateBasicBlock("loop", f);
            auto loop_continue = core::CreateBasicBlock("loop_continue", f);
            auto end = core::CreateBasicBlock("end", f);
            core::Builder.CreateBr(loop);

            core::Builder.SetInsertPoint(loop);
            auto index = core::Builder.CreatePHI(core::IntType, 2);
            index->addIncoming(core::GetInt(0), entry);
            auto c = core::CallFunction(getchar);
            auto c_switch = core::Builder.CreateSwitch(c, loop_continue);
            c_switch->addCase(core::GetChar(' '), end);
            c_switch->addCase(core::GetChar('\n'), end);
            c_switch->addCase(core::GetChar(-1), end);

            core::Builder.SetInsertPoint(loop_continue);
            core::Builder.CreateStore(c, core::Builder.CreateGEP(arg, index));
            auto next_index = core::Builder.CreateAdd(index, core::GetInt(1));
            index->addIncoming(next_index, loop_continue);
            core::Builder.CreateBr(loop);

            core::Builder.SetInsertPoint(end);
            core::Builder.CreateStore(NullChar, core::Builder.CreateGEP(arg, index));
            core::Builder.CreateRet(index);
        });
        core::CreateFunction(SkipCommentFunc, [=](Function* f, BasicBlock* entry) {
            auto loop = core::CreateBasicBlock("loop", f);
            auto end = core::CreateBasicBlock("end", f);
            core::Builder.CreateBr(loop);

            core::Builder.SetInsertPoint(loop);
            auto c = core::CallFunction(getchar);
            auto c_switch = core::Builder.CreateSwitch(c, loop);
            c_switch->addCase(core::GetChar('\n'), end);
            c_switch->addCase(core::GetChar(-1), end);

            core::Builder.SetInsertPoint(end);
            core::Builder.CreateRetVoid();
        });
    };
}

#endif //LLVM_FORTH_UTIL_H
