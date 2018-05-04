#include <iostream>
#include <string>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

static llvm::LLVMContext TheContext;
static std::unique_ptr<llvm::Module> TheModule = llvm::make_unique<llvm::Module>("llvm-forth", TheContext);

struct Builder : llvm::IRBuilder<> {
    Builder() : llvm::IRBuilder<>(TheContext) {}

    llvm::GlobalVariable* createGlobalVariable(llvm::StringRef Name, llvm::Type* Type) {
        TheModule->getOrInsertGlobal(Name, Type);
        auto value = TheModule->getGlobalVariable(Name);
        value->setLinkage(llvm::GlobalVariable::LinkageTypes::CommonLinkage);
        return value;
    }

    llvm::Function* createMain() {
        auto type = getFunctionType(getInt32Ty());
        return createFunction("main", type, llvm::Function::ExternalLinkage);
    }

    llvm::ArrayType* getArrayType(llvm::Type* Type, uint64_t Size) {
        return llvm::ArrayType::get(Type, Size);
    }

    llvm::FunctionType* getFunctionType(llvm::Type* Result, llvm::ArrayRef<llvm::Type*> Params) {
        return llvm::FunctionType::get(Result, Params, false);
    }

    llvm::FunctionType* getFunctionType(llvm::Type* Result) {
        return llvm::FunctionType::get(Result, false);
    }

    llvm::Function* createFunction(const llvm::Twine& Name, llvm::FunctionType* Type,
                                   llvm::Function::LinkageTypes Linkage = llvm::Function::ExternalLinkage) {
        return llvm::Function::Create(Type, Linkage, Name, TheModule.get());
    }

    llvm::BasicBlock* createBasicBlock(const llvm::Twine& Name, llvm::Function* Parent) {
        return llvm::BasicBlock::Create(Context, Name, Parent);
    }

    void dump() {
        TheModule->print(llvm::outs(), nullptr);
    }
};

int main() {
    Builder builder;

    auto putchar = builder.createFunction("putchar", builder.getFunctionType(builder.getInt32Ty(), builder.getInt32Ty()));


    auto main = builder.createMain();

    auto entry = builder.createBasicBlock("entry", main);
    auto next = builder.createBasicBlock("next", main);

    auto code_type = builder.getArrayType(builder.getInt8PtrTy(), 2);
    auto code = builder.createGlobalVariable("code", code_type);
    auto foo = builder.createBasicBlock("foo", main);
    auto bar = builder.createBasicBlock("bar", main);
    code->setInitializer(llvm::ConstantArray::get(code_type, {
        llvm::BlockAddress::get(foo),
        llvm::BlockAddress::get(bar),
    }));

    builder.SetInsertPoint(entry);
    auto i = 0;
    auto addr = builder.CreateExtractElement(code, i, "addr");
    //auto br = builder.CreateIndirectBr(addr, 2);
    //br->addDestination(foo);
    //br->addDestination(bar);
//    builder.CreateInsertElement(code, llvm::BlockAddress::get(foo), builder.getInt64(0));
    builder.CreateBr(next);

    builder.SetInsertPoint(foo);
//    builder.CreateCall(putchar, builder.getInt32(66));
    builder.CreateBr(next);

    builder.SetInsertPoint(bar);
    builder.CreateBr(next);

    /*
    builder.SetInsertPoint(next);
    auto ip = builder.CreateExtractElement(code, builder.getInt64(0));
    auto br = builder.CreateIndirectBr(ip, 2);
    br->addDestination(foo);
    br->addDestination(bar);
    */

    builder.SetInsertPoint(next);
    builder.CreateRet(builder.getInt32(0));
    builder.dump();
    return 0;
}