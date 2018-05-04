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
    auto top = builder.createBasicBlock("top", main);
    auto next = builder.createBasicBlock("next", main);
    auto A = builder.createBasicBlock("A", main);
    auto B = builder.createBasicBlock("B", main);
    auto exit = builder.createBasicBlock("exit", main);

    auto code_type = builder.getArrayType(builder.getInt8PtrTy(), 5);
    auto code = builder.createGlobalVariable("code", code_type);
    code->setLinkage(llvm::GlobalVariable::LinkageTypes::PrivateLinkage);
    code->setConstant(true);
    code->setInitializer(llvm::ConstantArray::get(code_type, {
        llvm::BlockAddress::get(A),
        llvm::BlockAddress::get(B),
        llvm::BlockAddress::get(A),
        llvm::BlockAddress::get(B),
        llvm::BlockAddress::get(exit),
    }));

    builder.SetInsertPoint(entry);
    auto start = builder.CreateGEP(code, {builder.getInt32(0), builder.getInt32(0)}, "start");
    builder.CreateBr(top);

    builder.SetInsertPoint(top);
    auto ip = builder.CreatePHI(builder.getInt8PtrTy()->getPointerTo(), 2, "ip");
    auto addr = builder.CreateLoad(ip, "addr");
    auto new_ip = builder.CreateGEP(ip, builder.getInt32(1), "new_ip");
    ip->addIncoming(start, entry);
    ip->addIncoming(new_ip, next);
    auto br = builder.CreateIndirectBr(addr, 3);
    br->addDestination(A);
    br->addDestination(B);
    br->addDestination(exit);

    builder.SetInsertPoint(next);
    builder.CreateBr(top);

    builder.SetInsertPoint(A);
    builder.CreateCall(putchar, builder.getInt32(65));
    builder.CreateBr(next);

    builder.SetInsertPoint(B);
    builder.CreateCall(putchar, builder.getInt32(66));
    builder.CreateBr(next);

    builder.SetInsertPoint(exit);
    builder.CreateCall(putchar, builder.getInt32(10));
    builder.CreateRet(builder.getInt32(0));

    builder.dump();
    return 0;
}