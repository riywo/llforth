//
// Created by Ryosuke Iwanaga on 2018-11-04.
//

#ifndef LLVM_FORTH_STACK_H
#define LLVM_FORTH_STACK_H

#include "core.h"
#include "dict.h"

namespace stack {
    static Value* SP;
    static Constant* Stack;
    static Value* RSP;
    static Constant* RStack;

    static void Push(Value* value) {
        auto current_sp = core::Builder.CreateLoad(SP);
        auto addr = core::Builder.CreateGEP(Stack, {core::GetInt(0), current_sp});
        core::Builder.CreateStore(value, addr);
        core::Builder.CreateStore(core::Builder.CreateAdd(current_sp, core::GetInt(1)), SP);
    }

    static LoadInst* Pop() {
        auto current_sp = core::Builder.CreateLoad(SP);
        auto top_sp = core::Builder.CreateSub(current_sp, core::GetInt(1));
        auto addr = core::Builder.CreateGEP(Stack, {core::GetIndex(0), top_sp});
        core::Builder.CreateStore(top_sp, SP);
        return core::Builder.CreateLoad(addr);
    }

    static void RPush(Value* value) {
        auto current_rsp = core::Builder.CreateLoad(RSP);
        auto addr = core::Builder.CreateGEP(RStack, {core::GetIndex(0), current_rsp});
        core::Builder.CreateStore(value, addr);
        core::Builder.CreateStore(core::Builder.CreateAdd(current_rsp, core::GetInt(1)), RSP);
    }

    static LoadInst* RPop() {
        auto current_rsp = core::Builder.CreateLoad(RSP);
        auto top_rsp = core::Builder.CreateSub(current_rsp, core::GetInt(1));
        auto addr = core::Builder.CreateGEP(RStack, {core::GetIndex(0), top_rsp});
        core::Builder.CreateStore(top_rsp, RSP);
        return core::Builder.CreateLoad(addr);
    }

    static void Initialize(Function* main, BasicBlock* entry) {
        SP = core::Builder.CreateAlloca(core::IntType, nullptr, "sp");
        core::Builder.CreateStore(core::GetInt(0), SP);
        auto stack_type = ArrayType::get(core::IntType, 1024);
        Stack = core::CreateGlobalVariable("stack", stack_type, UndefValue::get(stack_type), false);

        RSP = core::Builder.CreateAlloca(core::IntType, nullptr, "rsp");
        core::Builder.CreateStore(core::GetInt(0), RSP);
        auto rstack_type = ArrayType::get(dict::XtPtrPtrType, 1024);
        RStack = core::CreateGlobalVariable("rstack", rstack_type, UndefValue::get(rstack_type), false);
    }
}

#endif //LLVM_FORTH_STACK_H
