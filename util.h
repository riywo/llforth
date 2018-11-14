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
    const static core::Func FindXtFunc {
        "find_xt", FunctionType::get(dict::XtPtrType, {core::StrType}, false)
    };
    const static core::Func StringToIntFunc {
        "string_to_int", FunctionType::get(core::IntType, {core::StrType}, false)
    };
    const static core::Func StringEqualFunc {
        "string_equal", FunctionType::get(core::BoolType, {core::StrType, core::StrType}, false)
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
        core::Func strtoll = {
                "strtoll", FunctionType::get(core::IntType, {core::StrType, core::StrType->getPointerTo(), core::IntType}, false)
        };
        core::Func strcmp = {
                "strcmp", FunctionType::get(core::IntType, {core::StrType, core::StrType}, false)
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
        core::CreateFunction(StringToIntFunc, [=](Function* f, BasicBlock* entry){
            auto str = f->arg_begin();
            auto endptr = core::Builder.CreateAlloca(core::StrType);
            auto base = core::GetInt(10);
            auto number = core::CallFunction(strtoll, {str, endptr, base});
            core::Builder.CreateRet(number);
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
        core::CreateFunction(StringEqualFunc, [=](Function* f, BasicBlock* entry) {
            auto args = f->arg_begin();
            auto a_str = args++;
            auto b_str = args++;
            auto cmp = core::CallFunction(strcmp, {a_str, b_str});
            auto icmp = core::Builder.CreateICmpEQ(cmp, core::GetInt(0));
            core::Builder.CreateRet(icmp);
        });
        core::CreateFunction(FindXtFunc, [=](Function* f, BasicBlock* entry){
            auto arg = f->arg_begin();
            auto loop = core::CreateBasicBlock("loop", f);
            auto check_word = core::CreateBasicBlock("check_word", f);
            auto loop_continue = core::CreateBasicBlock("loop_continue", f);
            auto end = core::CreateBasicBlock("end", f);
            auto not_found = core::CreateBasicBlock("not_found", f);
            auto last_xt = dict::GetLastXt();
            core::Builder.CreateBr(loop);

            core::Builder.SetInsertPoint(loop);
            auto xt = core::Builder.CreatePHI(dict::XtPtrType, 2);
            xt->addIncoming(last_xt, entry);
            auto is_null = core::Builder.CreateICmpEQ(core::Builder.CreatePtrToInt(xt, core::IntType), core::GetInt(0));
            core::Builder.CreateCondBr(is_null, not_found, check_word);

            core::Builder.SetInsertPoint(check_word);
            auto word = dict::GetXtWord(xt);
            auto is_equal = core::CallFunction(StringEqualFunc, {arg, word});
            core::Builder.CreateCondBr(is_equal, end, loop_continue);

            core::Builder.SetInsertPoint(loop_continue);
            auto next_xt = dict::GetXtPrevious(xt);
            xt->addIncoming(next_xt, loop_continue);
            core::Builder.CreateBr(loop);

            core::Builder.SetInsertPoint(end);
            core::Builder.CreateRet(xt);

            core::Builder.SetInsertPoint(not_found);
            core::Builder.CreateRet(dict::XtPtrNull);
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
