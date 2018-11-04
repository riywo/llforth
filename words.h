//
// Created by Ryosuke Iwanaga on 2018-11-03.
//

#ifndef LLVM_FORTH_WORD_H
#define LLVM_FORTH_WORD_H

#include "engine.h"
#include "util.h"

namespace words {
    static engine::Word Lit;
    static engine::Word Dot;
    static engine::Word Bye;
    static engine::Word Docol;
    static engine::Word Exit;

    static bool IsLitWord(engine::Word word) {
        return word.xt == Lit.xt;
    };

    static engine::Word CreateLitWord(const std::string& value) {
        return engine::AddCompileWord(value, Lit.addr, std::stoi(value));
    };

    static void Initialize() {
        util::Initialize();

        Bye = engine::AddNativeWord("bye", [](){
            engine::CreateRet(0);
        });
        Dot = engine::AddNativeWord(".", [](){
            auto value = engine::Pop();
            core::CallFunction(util::PrintFunc, value);
            engine::CreateBrNext();
        });
        Lit = engine::AddNativeWord("lit", [](){
            auto value = engine::GetXtEmbedded();
            engine::Push(value);
            engine::CreateBrNext();
        });
        Docol = engine::AddNativeWord("docol", [](){
            engine::RPush(core::Builder.CreateLoad(engine::PC));
            auto new_pc = engine::GetXtColon();
            core::Builder.CreateStore(new_pc, engine::PC);
            engine::CreateBrNext();
        });
        Exit = engine::AddNativeWord("exit", [](){
            auto return_pc = engine::RPop();
            core::Builder.CreateStore(return_pc, engine::PC);
            engine::CreateBrNext();
        });
        engine::AddColonWord("foo", Docol.addr, {
            CreateLitWord("1").xt,
            Dot.xt,
            Exit.xt,
        });
    };
}

#endif //LLVM_FORTH_WORD_H
