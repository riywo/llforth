#include <iostream>
#include <string>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

struct ForthCompiler {
    ForthCompiler() {
        builder = llvm::make_unique<llvm::IRBuilder<>>(context);
        module = llvm::make_unique<llvm::Module>("forth", context);
        llvm::Function* mainFunc = llvm::Function::Create(
                llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false),
                llvm::Function::ExternalLinkage, "main", module.get()
        );
        builder->SetInsertPoint(llvm::BasicBlock::Create(context, "", mainFunc));
        builder->CreateRet(builder->getInt32(42));
    }

    void dump() {
        module->print(llvm::outs(), nullptr);
    }

private:
    llvm::LLVMContext context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
};

int main() {
    ForthCompiler compiler;
    compiler.dump();
    return 0;
}