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
    static dict::Word Fetch;
    static dict::Word Write;
    static dict::Word Docol;
    static dict::Word Inbuf;
    static dict::Word Word;
    static dict::Word Create;
    static dict::Word Finish;
    static dict::Word Exit;
    static dict::Word State;
    static dict::Word Comma;

    static Constant* StateValue;
    static Constant* HereValue;

    static Constant* StringBuffer;
    static Constant* ColonBuffer;

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
        HereValue = core::CreateGlobalVariable("here", core::IndexType, core::GetIndex(0), false);

        StringBuffer = core::CreateGlobalArrayVariable("string_buffer", core::CharType, 1024, false);
        ColonBuffer = core::CreateGlobalArrayVariable("colon_buffer", dict::XtPtrType, 1024, false);

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
        dict::AddNativeWord("drop", [](){
            stack::Drop();
            CreateBrNext();
        });
        dict::AddNativeWord(".S", [](){
            stack::Print();
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
            auto new_pc = core::Builder.CreateGEP(pc, offset);
            core::Builder.CreateStore(new_pc, engine::PC);
            CreateBrNext();
        });
        Skip = dict::AddNativeWord("skip", [](){
            auto pc = core::Builder.CreateLoad(engine::PC);
            auto new_pc = core::Builder.CreateGEP(pc, core::GetIndex(1));
            core::Builder.CreateStore(new_pc, engine::PC);
            CreateBrNext();
        });
        Branch0 = dict::AddNativeWord("branch0", [](){
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
        Docol = dict::AddNativeWord("docol", [](){
            stack::RPush(core::Builder.CreateLoad(engine::PC));
            auto new_pc = dict::GetXtColon();
            core::Builder.CreateStore(new_pc, engine::PC);
            CreateBrNext();
        });
        Exit = dict::AddNativeWord("exit", [](){
            auto return_pc = stack::RPop();
            core::Builder.CreateStore(return_pc, engine::PC);
            CreateBrNext();
        });
        Inbuf = dict::AddNativeWord("inbuf", [](){
            auto inbuf = core::Builder.CreateGEP(StringBuffer, {core::GetIndex(0), core::GetIndex(0)});
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
            core::CallFunction(util::StringCopyFunc, {word, name});
            core::Builder.CreateStore(dict::GetLastXt(),    core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtPrevious)}));
            core::Builder.CreateStore(word,                 core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtWord)}));
            core::Builder.CreateStore(Docol.addr,           core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtImplAddress)}));
            core::Builder.CreateStore(core::GetBool(false), core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtImmediate)}));
            stack::PushPtr(xt);
            core::Builder.CreateStore(core::GetIndex(0), HereValue);
            CreateBrNext();
        });
        Finish = dict::AddNativeWord("finish", [](){
            auto length = core::Builder.CreateLoad(HereValue);
            auto xts = core::Builder.CreateAlloca(dict::XtPtrType, length);
            auto xts_ptr = core::Builder.CreateGEP(xts, core::GetIndex(0));
            core::Builder.CreateStore(Exit.xt, xts_ptr);
            auto buf_ptr = core::Builder.CreateGEP(ColonBuffer, {core::GetIndex(0), core::GetIndex(0)});
//            core::Builder.CreateMemCpy(xts_ptr, 64, buf_ptr, 64, length);
//            core::CallFunction(util::MemoryCopyFunc, {xts_ptr, buf_ptr, length});

            auto xt = stack::PopPtr(dict::XtPtrType);
            //core::Builder.CreateStore(xts_ptr, core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtColon)}));
            core::Builder.CreateStore(buf_ptr, core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtColon)}));
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
            auto here = core::Builder.CreateLoad(HereValue);
            auto addr = core::Builder.CreateGEP(ColonBuffer, {core::GetIndex(0), here});
            core::Builder.CreateStore(xt, addr);
            auto next = core::Builder.CreateAdd(here, core::GetIndex(1));
            core::Builder.CreateStore(next, HereValue);
            CreateBrNext();
        });
        dict::AddColonWord(":", Docol.addr, {
            Inbuf.xt, Word.xt, Dup.xt,
            Branch0.xt, GetConstantIntToXtPtr(-4),
            Inbuf.xt, Create.xt,
            Lit.xt, GetConstantIntToXtPtr(1), State.xt, Write.xt,
            // TODO
            Exit.xt,
        });
        dict::AddColonWord(";", Docol.addr, {
            Lit.xt, GetConstantIntToXtPtr(0), State.xt, Write.xt,
            Lit.xt, Exit.xt, Comma.xt,
            Finish.xt,
            // TODO
            Exit.xt,
        }, true);
        dict::AddNativeWord("execute", [](){ // This definition must be the last
            auto xt = stack::PopPtr(dict::XtPtrType);
            core::Builder.CreateStore(xt, engine::W);
            engine::Jump();
        });
    };
}

#endif //LLVM_FORTH_WORD_H
