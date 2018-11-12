//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#ifndef LLVM_FORTH_CORE_H
#define LLVM_FORTH_CORE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <functional>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/MemoryBuffer.h>

using namespace llvm;

namespace core {
    static LLVMContext TheContext;
    static IRBuilder<> Builder(TheContext);
    static std::unique_ptr<Module> TheModule;

    const static auto IntType = Builder.getInt64Ty();
    const static auto CharType = Builder.getInt8Ty();
    const static auto StrType = CharType->getPointerTo();

    struct Func {
        std::string name;
        FunctionType* type;
    };

    static ConstantInt* GetInt(uint64_t value) {
        return ConstantInt::get(IntType, value);
    }

    static ConstantInt* GetIndex(uint64_t value) {
        return ConstantInt::get(Builder.getInt32Ty(), value);
    }

    static ConstantInt* GetChar(char value) {
        return ConstantInt::get(CharType, (int)value);
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

    static Constant* CreateGlobalArrayVariable(const std::string& name, Type* elem_type, uint64_t size, ArrayRef<Constant*> initial, bool isConstant) {
        auto type = ArrayType::get(elem_type, size);
        if (initial.equals(nullptr)) {
            initial = UndefValue::get(type);
        }
        return CreateGlobalVariable(name, type, ConstantArray::get(type, initial), isConstant);
    };

    static Constant* CreateGlobalArrayVariable(const std::string& name, Type* elem_type, ArrayRef<Constant*> initial, bool isConstant=true) {
        return CreateGlobalArrayVariable(name, elem_type, initial.size(), initial, isConstant);
    }

    static Constant* CreateGlobalArrayVariable(const std::string& name, Type* elem_type, uint64_t size, bool isConstant=true) {
        return CreateGlobalArrayVariable(name, elem_type, size, nullptr, isConstant);
    }

    static Constant* CreateConstantGEP(Constant* constant) {
        auto type = constant->getType()->getPointerElementType();
        Constant* idx[] = {core::GetIndex(0), core::GetIndex(0)};
        return ConstantExpr::getGetElementPtr(type, constant, idx);
    }

    static BasicBlock* CreateBasicBlock(const std::string& name, Function* function) {
        return BasicBlock::Create(TheContext, name, function);
    }

    static Function* CreateFunction(const Func& func) {
        return Function::Create(func.type, Function::ExternalLinkage, func.name, TheModule.get());
    }

    static Function* CreateFunction(const Func& func, const std::function<void(Function*, BasicBlock*)>& impl) {
        auto f = CreateFunction(func);
        auto entry = CreateBasicBlock("entry", f);
        Builder.SetInsertPoint(entry);
        impl(f, entry);
        return f;
    }

    static CallInst* CallFunction(const Func& func, ArrayRef<Value*> args={}) {
        auto callee = cast<Function>(TheModule->getOrInsertFunction(func.name, func.type));
        return Builder.CreateCall(callee, args);
    }

}

#endif //LLVM_FORTH_CORE_H
