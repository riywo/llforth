//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#ifndef LLVM_FORTH_CORE_H
#define LLVM_FORTH_CORE_H

#include <iostream>
#include <string>
#include <functional>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

namespace core {
    static LLVMContext TheContext;
    static IRBuilder<> Builder(TheContext);
    static std::unique_ptr<Module> TheModule;

    const static auto IntType = Builder.getInt32Ty();
    const static auto StrType = Builder.getInt8PtrTy();

    struct Func {
        std::string name;
        FunctionType* type;
    };

    static ConstantInt* GetInt(uint64_t value) {
        return ConstantInt::get(IntType, value);
    }

    static void CreateModule(const std::string& name) {
        TheModule = llvm::make_unique<Module>(name, TheContext);
    }

    static void DumpModule() {
        TheModule->print(outs(), nullptr);
    }

    static Constant* CreateGlobalVariable(const std::string& name, Type* type) {
        return TheModule->getOrInsertGlobal(name, type);
    }

    static Constant* CreateGlobalVariable(const std::string& name, Type* type, Constant* initial, bool isConstant=true) {
        auto g = CreateGlobalVariable(name, type);
        auto val = TheModule->getGlobalVariable(name);
        val->setLinkage(GlobalVariable::LinkageTypes::PrivateLinkage);
        val->setConstant(isConstant);
        val->setInitializer(initial);
        return g;
    }

    static BasicBlock* CreateBasicBlock(const std::string& name, Function* function) {
        return BasicBlock::Create(TheContext, name, function);
    }

    static Function* CreateFunction(const Func& func) {
        return Function::Create(func.type, Function::ExternalLinkage, func.name, TheModule.get());
    }

    static Function* CreateFunction(const Func& func, const std::function<void(Function*)>& impl) {
        auto f = CreateFunction(func);
        auto entry = CreateBasicBlock("entry", f);
        Builder.SetInsertPoint(entry);
        impl(f);
        return f;
    }

    static CallInst* CallFunction(const Func& func, ArrayRef<Value*> args) {
        auto callee = cast<Function>(TheModule->getOrInsertFunction(func.name, func.type));
        return Builder.CreateCall(callee, args);
    }
}

#endif //LLVM_FORTH_CORE_H
