#include <iostream>
#include <string>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>

struct ForthInterpreter {
    ForthInterpreter() {
        builder = llvm::make_unique<llvm::IRBuilder<>>(context);
        module = llvm::make_unique<llvm::Module>("llvm-forth", context);
        createMain();

        createReturn(0);
    }

    void dump() {
        module->print(llvm::outs(), nullptr);
    }

private:
    llvm::LLVMContext context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;

    void createMain() {
        auto type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
        auto mainFunc = llvm::Function::Create(type, llvm::Function::ExternalLinkage, "main", module.get());
        auto basicBlock = llvm::BasicBlock::Create(context, "entry", mainFunc);
        builder->SetInsertPoint(basicBlock);
    }

    void createReturn(const uint32_t& ret) {
        builder->CreateRet(builder->getInt32(ret));
    }
};

int main() {
    ForthInterpreter interpreter;
    interpreter.dump();
    return 0;
}