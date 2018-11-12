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
    static dict::Word Dot;
    static dict::Word Bye;
    static dict::Word Docol;
    static dict::Word Exit;

    static Constant* Inbuf;

    static dict::Word AddLitWord(const std::string& value) {
        return dict::AddCompileWord(value, Lit.addr, std::stoi(value));
    };

    static void CreateBrNext() {
        core::Builder.CreateBr(engine::Next);
    };

    static void CreateRet(int ret) {
        core::Builder.CreateRet(core::GetInt(ret));
    };

    static void Initialize(Function* main, BasicBlock* entry) {
        util::Initialize();

        Inbuf = core::CreateGlobalArrayVariable("inbuf", core::CharType, 1024, false);

        Bye = dict::AddNativeWord("bye", [](){
            CreateRet(0);
        });
        Dot = dict::AddNativeWord(".", [](){
            auto value = stack::Pop();
            core::CallFunction(util::PrintIntFunc, value);
            CreateBrNext();
        });

        Lit = dict::AddNativeWord("lit", [](){
            auto value = dict::GetXtEmbedded();
            stack::Push(value);
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
        dict::AddNativeWord("inbuf", [](){
            auto inbuf = core::Builder.CreateGEP(Inbuf, {core::GetIndex(0), core::GetIndex(0)});
            stack::Push(core::Builder.CreatePtrToInt(inbuf, core::IntType));
            CreateBrNext();
        });
        dict::AddNativeWord("word", [](){
            auto buf = core::Builder.CreateIntToPtr(stack::Pop(), core::StrType);
            stack::Push(core::CallFunction(util::ReadWordFunc, buf));
            CreateBrNext();
        });
        dict::AddNativeWord("prints", [](){
            auto str = core::Builder.CreateIntToPtr(stack::Pop(), core::StrType);
            core::CallFunction(util::PrintStrFunc, str);
            CreateBrNext();
        });
        dict::AddNativeWord("find", [](){
            auto str = core::Builder.CreateIntToPtr(stack::Pop(), core::StrType);

            CreateBrNext();
        });
        dict::AddNativeWord("\\", [](){
            core::CallFunction(util::SkipCommentFunc);
            CreateBrNext();
        });
        dict::AddColonWord("foo", Docol.addr, {
            AddLitWord("1").xt,
            Dot.xt,
            Exit.xt,
        });
    };
}

#endif //LLVM_FORTH_WORD_H
