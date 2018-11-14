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

        for (const auto initializer: Initializers) {
            core::Builder.SetInsertPoint(Entry);
            initializer(MainFunction, Entry);
        }
    };

    static void Finalize(const std::vector<Constant*>& code) {
        for (const auto finalizer: Finalizers) {
            core::Builder.SetInsertPoint(Entry);
            finalizer(code);
        }
        core::Builder.CreateBr(Next);

        core::Builder.SetInsertPoint(Next);
        auto pc = core::Builder.CreateLoad(PC);
        core::Builder.CreateStore(core::Builder.CreateLoad(pc), W);
        auto new_pc = core::Builder.CreateGEP(pc, core::GetIndex(1));
        core::Builder.CreateStore(new_pc, PC);
        Jump();
    };
}

#endif //LLVM_FORTH_ENGINE_H
