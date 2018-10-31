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

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;

const static auto IntType = Builder.getInt32Ty();
const static auto StrType = Builder.getInt8PtrTy();
const static auto PtrType = Builder.getInt8PtrTy();
const static auto PtrPtrType = PtrType->getPointerTo();
const static auto NodeType = StructType::create(TheContext, "node");
const static auto NodePtrType = NodeType->getPointerTo();

static Function* MainFunc;
static BasicBlock* Entry;
static BasicBlock* Next;
static Constant* LastNode = ConstantPointerNull::get(NodePtrType);
static std::map<std::string, Constant*> xtMap = {};
static std::vector<BasicBlock*> NativeBlocks = {};

static Constant* CreateGlobalVariable(const std::string& name, Type* type, Constant* initial,
        bool isConstant=true, GlobalVariable::LinkageTypes linkageType=GlobalVariable::LinkageTypes::PrivateLinkage) {
    auto g = TheModule->getOrInsertGlobal(name, type);
    auto val = TheModule->getGlobalVariable(name);
    val->setLinkage(linkageType);
    val->setConstant(isConstant);
    val->setInitializer(initial);
    return g;
}

static void AddNativeWord(const std::string& word, const std::function<void()>& impl) {
    auto block = BasicBlock::Create(TheContext, "i_" + word, MainFunc);
    Builder.SetInsertPoint(block);
    impl();
    NativeBlocks.push_back(block);

    auto xt = BlockAddress::get(block);
    xtMap[word] = xt;

    auto str_ptr = Builder.CreateGlobalStringPtr(word, "k_" + word);
    auto node_val = ConstantStruct::get(NodeType, LastNode, str_ptr, xt, ConstantPointerNull::get(PtrPtrType));
    LastNode = CreateGlobalVariable("d_" + word, NodeType, node_val);
}

static void Initialize() {
    TheModule = make_unique<Module>("main", TheContext);
    MainFunc = Function::Create(FunctionType::get(IntType, false), Function::ExternalLinkage, "main", TheModule.get());
    Entry = BasicBlock::Create(TheContext, "entry", MainFunc);
    Next = BasicBlock::Create(TheContext, "next", MainFunc);

    NodeType->setBody(
            NodePtrType,
            StrType,
            PtrType,
            PtrPtrType);

    AddNativeWord("foo", [](){
        Builder.CreateBr(Next);
    });
    AddNativeWord("bar", [](){
        Builder.CreateBr(Next);
    });
    AddNativeWord("exit", [](){
        Builder.CreateRet(Builder.getInt32(0));
    });
}

static std::vector<Constant*> MainLoop() {
    std::string token;
    auto code = std::vector<Constant*>();
/**
    while (std::cin >> token) {
        if (token == "bye") {
            break;
        } else {
            auto xt = xtMap.find(token);
            if (xt == xtMap.end()) {

            } else {
                code.push_back(xt->second);
            }
        }
    }
**/
    code.push_back(xtMap.find("foo")->second);
    code.push_back(xtMap.find("bar")->second);
    code.push_back(xtMap.find("exit")->second);

    return code;
}

static void Finalize(const std::vector<Constant*>& code) {
    auto code_type = ArrayType::get(PtrType, code.size());
    auto code_block = CreateGlobalVariable("code", code_type, ConstantArray::get(code_type, code));

    Builder.SetInsertPoint(Entry);
    auto start = Builder.CreateGEP(code_block, {Builder.getInt32(0), Builder.getInt32(0)}, "start");
    auto pc = Builder.CreateAlloca(PtrPtrType, nullptr, "pc");
    auto w = Builder.CreateAlloca(PtrPtrType, nullptr, "w");
    Builder.CreateStore(start, pc);
    Builder.CreateBr(Next);

    Builder.SetInsertPoint(Next);
    auto current_pc = Builder.CreateLoad(pc);
    Builder.CreateStore(current_pc, w);
    Builder.CreateStore(Builder.CreateGEP(current_pc, Builder.getInt8(1)), pc);
    auto br = Builder.CreateIndirectBr(Builder.CreateLoad(Builder.CreateLoad(w)), NativeBlocks.size());
    for (auto block : NativeBlocks) {
        br->addDestination(block);
    }
}

int main() {
    Initialize();
    auto code = MainLoop();
    Finalize(code);
    TheModule->print(outs(), nullptr);
    return 0;
}
