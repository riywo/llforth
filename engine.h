//
// Created by Ryosuke Iwanaga on 2018-11-03.
//

#ifndef LLVM_FORTH_ENGINE_H
#define LLVM_FORTH_ENGINE_H

#include "core.h"

namespace engine {
    static Function* MainFunction;
    static BasicBlock* Entry;
    static BasicBlock* Next;
    static Value* PC;
    static Value* W;

    static std::vector<std::function<void(Function*, BasicBlock*)>> Initializers = {};
    static std::vector<std::function<void(const std::vector<Constant*>&)>> Finalizers = {};
    static std::function<void()> Jump;

    static void Initialize() {
        core::Func func = {"main", FunctionType::get(core::IntType, false)};
        MainFunction = core::CreateFunction(func);
        Entry = core::CreateBasicBlock("entry", MainFunction);
        Next = core::CreateBasicBlock("next", MainFunction);

        core::Builder.SetInsertPoint(Entry);
        for (auto initializer: Initializers) {
            initializer(MainFunction, Entry);
        }
    };

    static void Finalize(const std::vector<Constant*>& code) {
        core::Builder.SetInsertPoint(Entry);
        for (auto finalizer: Finalizers) {
            finalizer(code);
        }
        core::Builder.CreateBr(Next);

        core::Builder.SetInsertPoint(Next);
        auto pc = core::Builder.CreateLoad(PC);
        core::Builder.CreateStore(core::Builder.CreateLoad(pc), W);
        auto new_pc = core::Builder.CreateGEP(pc, core::GetInt(1));
        core::Builder.CreateStore(new_pc, PC);
        Jump();
    };
}

#endif //LLVM_FORTH_ENGINE_H
