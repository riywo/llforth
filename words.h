//
// Created by Ryosuke Iwanaga on 2018-11-03.
//

#ifndef LLVM_FORTH_WORD_H
#define LLVM_FORTH_WORD_H

#include "engine.h"
#include "dict.h"
#include "stack.h"
#include "util.h"

namespace words {
    static dict::Word Lit;
    static dict::Word Branch;
    static dict::Word Skip;
    static dict::Word Branch0;
    static dict::Word Throw;
    static dict::Word Dot;
    static dict::Word Dup;
    static dict::Word Swap;
    static dict::Word Fetch;
    static dict::Word Write;
    static dict::Word Here;
    static dict::Word HereFetch;
    static dict::Word Docol;
    static dict::Word Inbuf;
    static dict::Word Word;
    static dict::Word Create;
    static dict::Word Finish;
    static dict::Word Exit;
    static dict::Word State;
    static dict::Word Comma;

    static Constant* StateValue;

    static Constant* InputBuffer;

    static Constant* GetConstantIntToXtPtr(int num) {
        return ConstantExpr::getIntToPtr(ConstantInt::get(core::IntType, num), dict::XtPtrType);
    }

    static void CreateBrNext() {
        core::Builder.CreateBr(engine::Next);
    };

    static void CreateRet(int ret) {
        core::Builder.CreateRet(core::GetInt(ret));
    };

    static void Initialize(Function* main, BasicBlock* entry) {
        util::Initialize();

        StateValue = core::CreateGlobalVariable("state", core::IntType, core::GetInt(0), false);

        InputBuffer = core::CreateGlobalArrayVariable("input_buffer", core::CharType, 1024, false);

        dict::AddNativeWord("bye", [](){
            CreateRet(0);
        });
        Throw = dict::AddNativeWord("throw", [](){
            core::Builder.CreateRet(stack::Pop());
        });
        Dot = dict::AddNativeWord(".", [](){
            auto value = stack::Pop();
            core::CallFunction(util::PrintIntFunc, value);
            CreateBrNext();
        });
        Dup = dict::AddNativeWord("dup", [](){
            stack::Dup();
            CreateBrNext();
        });
        Swap = dict::AddNativeWord("swap", [](){
            auto first = stack::Pop();
            auto second = stack::Pop();
            stack::Push(first);
            stack::Push(second);
            CreateBrNext();
        });
        dict::AddNativeWord("drop", [](){
            stack::Drop();
            CreateBrNext();
        });
        dict::AddNativeWord(".S", [](){
            stack::Print();
            CreateBrNext();
        });
        dict::AddNativeWord("+", [](){
            auto right = stack::Pop();
            auto left = stack::Pop();
            stack::Push(core::Builder.CreateAdd(left, right));
            CreateBrNext();
        });
        dict::AddNativeWord("-", [](){
            auto right = stack::Pop();
            auto left = stack::Pop();
            stack::Push(core::Builder.CreateSub(left, right));
            CreateBrNext();
        });
        dict::AddNativeWord("*", [](){
            auto right = stack::Pop();
            auto left = stack::Pop();
            stack::Push(core::Builder.CreateMul(left, right));
            CreateBrNext();
        });
        dict::AddNativeWord("/", [](){
            auto right = stack::Pop();
            auto left = stack::Pop();
            stack::Push(core::Builder.CreateSDiv(left, right));
            CreateBrNext();
        });
        dict::AddNativeWord(">", [](){
            auto right = stack::Pop();
            auto left = stack::Pop();
            auto icmp = core::Builder.CreateICmpSGT(left, right);
            stack::Push(core::Builder.CreateIntCast(icmp, core::IntType, true));
            CreateBrNext();
        });
        dict::AddNativeWord("<", [](){
            auto right = stack::Pop();
            auto left = stack::Pop();
            auto icmp = core::Builder.CreateICmpSLT(left, right);
            stack::Push(core::Builder.CreateIntCast(icmp, core::IntType, true));
            CreateBrNext();
        });
        dict::AddNativeWord(">=", [](){
            auto right = stack::Pop();
            auto left = stack::Pop();
            auto icmp = core::Builder.CreateICmpSGE(left, right);
            stack::Push(core::Builder.CreateIntCast(icmp, core::IntType, true));
            CreateBrNext();
        });
        dict::AddNativeWord("<=", [](){
            auto right = stack::Pop();
            auto left = stack::Pop();
            auto icmp = core::Builder.CreateICmpSLE(left, right);
            stack::Push(core::Builder.CreateIntCast(icmp, core::IntType, true));
            CreateBrNext();
        });
        dict::AddNativeWord("=", [](){
            auto right = stack::Pop();
            auto left = stack::Pop();
            auto icmp = core::Builder.CreateICmpEQ(left, right);
            stack::Push(core::Builder.CreateIntCast(icmp, core::IntType, true));
            CreateBrNext();
        });
        Lit = dict::AddNativeWord("lit", [](){
            auto pc = core::Builder.CreateLoad(engine::PC);
            auto value = core::Builder.CreateLoad(pc);
            stack::PushPtr(value);
            auto new_pc = core::Builder.CreateGEP(pc, core::GetIndex(1));
            core::Builder.CreateStore(new_pc, engine::PC);
            CreateBrNext();
        });
        Branch = dict::AddNativeWord("branch", [](){
            auto pc = core::Builder.CreateLoad(engine::PC);
            auto value = core::Builder.CreateLoad(pc);
            auto offset = core::Builder.CreatePtrToInt(value, core::IndexType);
            auto new_pc = core::Builder.CreateGEP(dict::Memory, {core::GetIndex(0), offset});
            core::Builder.CreateStore(new_pc, engine::PC);
            CreateBrNext();
        });
        Skip = dict::AddNativeWord("skip", [](){
            auto pc = core::Builder.CreateLoad(engine::PC);
            auto new_pc = core::Builder.CreateGEP(pc, core::GetIndex(1));
            core::Builder.CreateStore(new_pc, engine::PC);
            CreateBrNext();
        });
        Branch0 = dict::AddNativeWord("0branch", [](){
            auto is_zero = core::Builder.CreateICmpEQ(stack::Pop(), core::GetInt(0));
            core::Builder.CreateCondBr(is_zero, Branch.block, Skip.block);
        });
        State = dict::AddNativeWord("state", [](){
            auto addr = ConstantExpr::getPtrToInt(StateValue, core::IntType);
            stack::Push(addr);
            CreateBrNext();
        });
        Fetch = dict::AddNativeWord("@", [](){
            auto addr = stack::PopPtr(core::IntPtrType);
            stack::Push(core::Builder.CreateLoad(addr));
            CreateBrNext();
        });
        Write = dict::AddNativeWord("!", [](){
            auto addr = stack::PopPtr(core::IntPtrType);
            auto value = stack::Pop();
            core::Builder.CreateStore(value, addr);
            CreateBrNext();
        });
        Here = dict::AddNativeWord("here", [](){
            auto here = core::Builder.CreateLoad(dict::HereValue);
            stack::Push(core::Builder.CreateIntCast(here, core::IntType, true));
            CreateBrNext();
        });
        HereFetch = dict::AddNativeWord("here@", [](){
            auto here = core::Builder.CreateLoad(dict::HereValue);
            stack::PushPtr(core::Builder.CreateGEP(dict::Memory, {core::GetIndex(0), here}));
            CreateBrNext();
        });
        Docol = dict::AddNativeWord("docol", [](){
            stack::RPush(core::Builder.CreateLoad(engine::PC));
            auto index = dict::GetXtColon();
            auto new_pc = core::Builder.CreateGEP(dict::Memory, {core::GetIndex(0), index});
            core::Builder.CreateStore(new_pc, engine::PC);
            CreateBrNext();
        });
        Exit = dict::AddNativeWord("exit", [](){
            auto return_pc = stack::RPop();
            core::Builder.CreateStore(return_pc, engine::PC);
            CreateBrNext();
        });
        Inbuf = dict::AddNativeWord("inbuf", [](){
            auto inbuf = core::Builder.CreateGEP(InputBuffer, {core::GetIndex(0), core::GetIndex(0)});
            stack::PushPtr(inbuf);
            CreateBrNext();
        });
        Word = dict::AddNativeWord("word", [](){
            auto buf = stack::PopPtr(core::StrType);
            auto res = core::CallFunction(util::ReadWordFunc, {buf, core::GetInt(1024)});
            stack::Push(res);
            auto is_failed = core::Builder.CreateICmpSLT(res, core::GetInt(0));
            core::Builder.CreateCondBr(is_failed, Throw.block, engine::Next);
        });
        dict::AddNativeWord("prints", [](){
            auto str = stack::PopPtr(core::StrType);
            core::CallFunction(util::PrintStrFunc, str);
            CreateBrNext();
        });
        dict::AddNativeWord("number", [](){
            auto str = stack::PopPtr(core::StrType);
            stack::Push(core::CallFunction(util::StringToIntFunc, str));
            CreateBrNext();
        });
        dict::AddNativeWord("find", [](){
            auto str = stack::PopPtr(core::StrType);
            auto found = core::CallFunction(util::FindXtFunc, str);
            stack::PushPtr(found);
            CreateBrNext();
        });
        dict::AddNativeWord("cr", [](){
            auto cr = core::Builder.CreateGlobalStringPtr("\n");
            core::CallFunction(util::PrintStrFunc, cr);
            CreateBrNext();
        });
        dict::AddNativeWord("\\", [](){
            core::CallFunction(util::SkipCommentFunc);
            CreateBrNext();
        });
        Create = dict::AddNativeWord("create", [](){
            auto xt = core::Builder.CreateAlloca(dict::XtType);
            auto name = stack::PopPtr(core::StrType);
            auto length = stack::Pop();
            auto word = core::Builder.CreateAlloca(core::CharType, length);
            auto here = core::Builder.CreateLoad(dict::HereValue);
            core::CallFunction(util::StringCopyFunc, {word, name});
            core::Builder.CreateStore(dict::GetLastXt(),    core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtPrevious)}));
            core::Builder.CreateStore(word,                 core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtWord)}));
            core::Builder.CreateStore(Docol.addr,           core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtImplAddress)}));
            core::Builder.CreateStore(core::GetBool(false), core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtImmediate)}));
            core::Builder.CreateStore(here,                 core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtColon)}));
            core::Builder.CreateStore(xt, dict::LastXt);
            CreateBrNext();
        });
        dict::AddNativeWord("flag", [](){
            auto xt = stack::PopPtr(dict::XtPtrType);
            auto flag = dict::GetXtImmediate(xt);
            stack::Push(core::Builder.CreateIntCast(flag, core::IntType, true));
            CreateBrNext();
        });
        Comma = dict::AddNativeWord(",", [](){
            auto xt = stack::PopPtr(dict::XtPtrType);
            auto here = core::Builder.CreateLoad(dict::HereValue);
            auto here_memory = core::Builder.CreateGEP(dict::Memory, {core::GetIndex(0), here});
            core::Builder.CreateStore(xt, here_memory);
            auto next = core::Builder.CreateAdd(here, core::GetIndex(1));
            core::Builder.CreateStore(next, dict::HereValue);
            CreateBrNext();
        });
        dict::AddNativeWord("execute", [](){ // This definition must be the last
            auto xt = stack::PopPtr(dict::XtPtrType);
            core::Builder.CreateStore(xt, engine::W);
            engine::Jump();
        });
        dict::AddColonWord(":", Docol.addr, {
                Inbuf.xt, Word.xt, Dup.xt,
                Branch0.xt, 0,
                Inbuf.xt, Create.xt,
                Lit.xt, GetConstantIntToXtPtr(1), State.xt, Write.xt,
                Exit.xt,
        });
        dict::AddColonWord(";", Docol.addr, {
                Lit.xt, GetConstantIntToXtPtr(0), State.xt, Write.xt,
                Lit.xt, Exit.xt, Comma.xt,
                Exit.xt,
        }, true);
    };
}

#endif //LLVM_FORTH_WORD_H
