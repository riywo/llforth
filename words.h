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
    static dict::Word Branch0;
    static dict::Word Throw;
    static dict::Word Dot;
    static dict::Word Dup;
    static dict::Word Docol;
    static dict::Word Inbuf;
    static dict::Word Word;
    static dict::Word Create;
    static dict::Word Exit;

    static Constant* StringBuffer;
    static Constant* ColonBuffer;

    static dict::Word AddLitWord(const std::string& value) {
        return dict::AddCompileWord(value, Lit.addr, std::stoi(value));
    };

    static dict::Word AddBranchWord(int offset) {
        std::string name = "branch=" + std::to_string(offset);
        return dict::AddCompileWord(name, Branch.addr, offset);
    };

    static dict::Word AddBranch0Word(int offset) {
        std::string name = "branch0=" + std::to_string(offset);
        return dict::AddCompileWord(name, Branch0.addr, offset);
    };

    static void CreateBrNext() {
        core::Builder.CreateBr(engine::Next);
    };

    static void CreateRet(int ret) {
        core::Builder.CreateRet(core::GetInt(ret));
    };

    static void Initialize(Function* main, BasicBlock* entry) {
        util::Initialize();

        StringBuffer = core::CreateGlobalArrayVariable("string_buffer", core::CharType, 1024, false);
        ColonBuffer = core::CreateGlobalArrayVariable("colon_buffer", dict::XtType, 1024, false);

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
            auto value = dict::GetXtEmbedded();
            stack::Push(value);
            CreateBrNext();
        });
        Branch = dict::AddNativeWord("branch", [](){
            auto value = dict::GetXtEmbedded();
            auto pc = core::Builder.CreateLoad(engine::PC);
            auto new_pc = core::Builder.CreateGEP(pc, value);
            core::Builder.CreateStore(new_pc, engine::PC);
            CreateBrNext();
        });
        Branch0 = dict::AddNativeWord("branch0", [](){
            auto tos = stack::Pop();
            auto is_zero = core::Builder.CreateICmpEQ(tos, core::GetInt(0));
            core::Builder.CreateCondBr(is_zero, Branch.block, engine::Next);
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
            stack::Push(core::Builder.CreatePtrToInt(inbuf, core::IntType));
            CreateBrNext();
        });
        Word = dict::AddNativeWord("word", [](){
            auto buf = core::Builder.CreateIntToPtr(stack::Pop(), core::StrType);
            auto res = core::CallFunction(util::ReadWordFunc, {buf, core::GetInt(1024)});
            stack::Push(res);
            auto is_failed = core::Builder.CreateICmpSLT(res, core::GetInt(0));
            core::Builder.CreateCondBr(is_failed, Throw.block, engine::Next);
        });
        dict::AddNativeWord("prints", [](){
            auto str = core::Builder.CreateIntToPtr(stack::Pop(), core::StrType);
            core::CallFunction(util::PrintStrFunc, str);
            CreateBrNext();
        });
        dict::AddNativeWord("number", [](){
            auto str = core::Builder.CreateIntToPtr(stack::Pop(), core::StrType);
            stack::Push(core::CallFunction(util::StringToIntFunc, str));
            CreateBrNext();
        });
        dict::AddNativeWord("find", [](){
            auto str = core::Builder.CreateIntToPtr(stack::Pop(), core::StrType);
            auto found = core::CallFunction(util::FindXtFunc, str);
            stack::Push(core::Builder.CreatePtrToInt(found, core::IntType));
            CreateBrNext();
        });
        dict::AddNativeWord("\\", [](){
            core::CallFunction(util::SkipCommentFunc);
            CreateBrNext();
        });
        Create = dict::AddNativeWord("create", [](){
            auto xt = core::Builder.CreateAlloca(dict::XtType);
            auto name = core::Builder.CreateIntToPtr(stack::Pop(), core::StrType);
            auto length = stack::Pop();
            auto word = core::Builder.CreateAlloca(core::CharType, length);
            core::CallFunction(util::StringCopyFunc, {word, name});
            core::Builder.CreateStore(word, core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtWord)}));
            core::Builder.CreateStore(Docol.addr, core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(dict::XtImplAddress)}));
            stack::Push(core::Builder.CreatePtrToInt(xt, core::IntType));
            CreateBrNext();
        });
        dict::AddColonWord(":", Docol.addr, {
            Inbuf.xt, Word.xt, Dup.xt,
            AddBranch0Word(-2).xt,
            Inbuf.xt, Create.xt,
            // TODO
            Dot.xt,
            Exit.xt,
        });
        dict::AddNativeWord("execute", [](){ // This definition must be the last
            auto xt = core::Builder.CreateIntToPtr(stack::Pop(), dict::XtPtrType);
            core::Builder.CreateStore(xt, engine::W);
            engine::Jump();
        });
    };
}

#endif //LLVM_FORTH_WORD_H
