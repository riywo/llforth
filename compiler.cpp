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
const static auto AddressType = Builder.getInt8PtrTy();
const static auto NodeType = StructType::create(TheContext, "node");
const static auto NodePtrType = NodeType->getPointerTo();
const static auto NodePtrPtrType = NodePtrType->getPointerTo();

static Function* MainFunc;
static BasicBlock* Entry;
static BasicBlock* Next;
static BasicBlock* Lit;
static Constant* LastNode = ConstantPointerNull::get(NodePtrType);
static std::map<std::string, Constant*> Dictionary = {};
static std::vector<BasicBlock*> NativeBlocks = {};

static Function* PrintFunc;

static AllocaInst* pc;
static AllocaInst* w;
static AllocaInst* sp;
static Constant* stack;

static ConstantInt* GetInt(int value) {
    return ConstantInt::get(IntType, value);
}

static Constant* CreateGlobalVariable(const std::string& name, Type* type, Constant* initial,
        bool isConstant=true, GlobalVariable::LinkageTypes linkageType=GlobalVariable::LinkageTypes::PrivateLinkage) {
    auto g = TheModule->getOrInsertGlobal(name, type);
    auto val = TheModule->getGlobalVariable(name);
    val->setLinkage(linkageType);
    val->setConstant(isConstant);
    val->setInitializer(initial);
    return g;
}

static Constant* AddDictionary(const std::string& word, BasicBlock* block, Constant* impl, int integer) {
    auto xt =  BlockAddress::get(block);
    auto str_ptr = Builder.CreateGlobalStringPtr(word, "k_" + word);
    auto node_val = ConstantStruct::get(NodeType, LastNode, str_ptr, xt, impl, GetInt(integer));
    auto node = CreateGlobalVariable("d_" + word, NodeType, node_val);
    Dictionary[word] = node;
    LastNode = node;
    return node;
}

static Constant* AddDictionary(const std::string& word, BasicBlock* block) {
    return AddDictionary(word, block, ConstantPointerNull::get(NodePtrPtrType), 0);
}

static Constant* AddDictionary(const std::string& word, BasicBlock* block, int integer) {
    return AddDictionary(word, block, ConstantPointerNull::get(NodePtrPtrType), integer);
}

static BasicBlock* AddNativeWord(const std::string& word, const std::function<void()>& impl) {
    auto block = BasicBlock::Create(TheContext, "i_" + word, MainFunc);
    Builder.SetInsertPoint(block);
    impl();
    NativeBlocks.push_back(block);
    AddDictionary(word, block);
    return block;
}

static Constant* AddLit(const std::string& word) {
    auto value = std::stoi(word);
    return AddDictionary(word, Lit, value);
}

static void Push(Value* value) {
    auto current_sp = Builder.CreateLoad(sp);
    auto addr = Builder.CreateGEP(stack, {GetInt(0), current_sp});
    Builder.CreateStore(value, addr);
    Builder.CreateStore(Builder.CreateAdd(current_sp, GetInt(1)), sp);
}

static LoadInst* Peek() {
    auto current_sp = Builder.CreateLoad(sp);
    auto top_sp = Builder.CreateSub(current_sp, GetInt(1));
    auto addr = Builder.CreateGEP(stack, {GetInt(0), top_sp});
    return Builder.CreateLoad(addr);
}

static LoadInst* Pop() {
    auto current_sp = Builder.CreateLoad(sp);
    auto top_sp = Builder.CreateSub(current_sp, GetInt(1));
    auto addr = Builder.CreateGEP(stack, {GetInt(0), top_sp});
    Builder.CreateStore(top_sp, sp);
    return Builder.CreateLoad(addr);
}

static void InitializePrint() {
    PrintFunc = Function::Create(FunctionType::get(Builder.getVoidTy(), {IntType}, false), Function::PrivateLinkage, "print", TheModule.get());
    auto entry = BasicBlock::Create(TheContext, "entry", PrintFunc);
    Builder.SetInsertPoint(entry);
    auto arg = PrintFunc->arg_begin();
    auto printf_type = FunctionType::get(IntType, {StrType}, true);
    auto printf = cast<Function>(TheModule->getOrInsertFunction("printf", printf_type));
    auto d = Builder.CreateGlobalStringPtr("%d ");
    Builder.CreateCall(printf, {d, arg});
    Builder.CreateRetVoid();
}

static void Initialize() {
    TheModule = make_unique<Module>("main", TheContext);
    MainFunc = Function::Create(FunctionType::get(IntType, false), Function::ExternalLinkage, "main", TheModule.get());
    Entry = BasicBlock::Create(TheContext, "entry", MainFunc);
    Next = BasicBlock::Create(TheContext, "next", MainFunc);

    NodeType->setBody(
            NodePtrType,    // Previous node
            StrType,        // Word of node
            AddressType,    // Execution token (BlockAddress)
            NodePtrPtrType, // Array of nodes if colon word
            IntType         // Integer if lit
    );

    InitializePrint();

    Builder.SetInsertPoint(Entry);
    pc = Builder.CreateAlloca(NodePtrPtrType, nullptr, "pc");
    w = Builder.CreateAlloca(NodePtrPtrType, nullptr, "w");
    sp = Builder.CreateAlloca(IntType, nullptr, "sp");
    Builder.CreateStore(GetInt(0), sp);
    auto stack_type = ArrayType::get(IntType, 1024);
    stack = CreateGlobalVariable("stack", stack_type, UndefValue::get(stack_type), false);

    AddNativeWord("bye", [](){
        Builder.CreateRet(GetInt(0));
    });
    AddNativeWord("+", [](){
        auto right = Pop();
        auto left = Pop();
        auto result = Builder.CreateAdd(left, right);
        Push(result);
        Builder.CreateBr(Next);
    });
    AddNativeWord("-", [](){
        auto right = Pop();
        auto left = Pop();
        auto result = Builder.CreateSub(left, right);
        Push(result);
        Builder.CreateBr(Next);
    });
    AddNativeWord("*", [](){
        auto right = Pop();
        auto left = Pop();
        auto result = Builder.CreateMul(left, right);
        Push(result);
        Builder.CreateBr(Next);
    });
    AddNativeWord("/", [](){
        auto right = Pop();
        auto left = Pop();
        auto result = Builder.CreateUDiv(left, right);
        Push(result);
        Builder.CreateBr(Next);
    });
    AddNativeWord(".", [](){
        Builder.CreateCall(PrintFunc, {Pop()});
        Builder.CreateBr(Next);
    });
    Lit = AddNativeWord("lit", [](){
        auto node = Builder.CreateLoad(Builder.CreateLoad(w));
        auto value = Builder.CreateLoad(Builder.CreateGEP(node, {GetInt(0), GetInt(4)}));
        Push(value);
        Builder.CreateBr(Next);
    });
}

static std::vector<Constant*> MainLoop() {
    std::string token;
    auto code = std::vector<Constant*>();
    while (std::cin >> token) {
        auto node = Dictionary.find(token);
        if (node == Dictionary.end()) { // Not found
            code.push_back(AddLit(token));
        } else {
            code.push_back(node->second);
        }
    }
    return code;
}

static void Finalize(const std::vector<Constant*>& code) {
    Builder.SetInsertPoint(Entry);
    auto code_type = ArrayType::get(NodePtrType, code.size());
    auto code_block = CreateGlobalVariable("code", code_type, ConstantArray::get(code_type, code));
    auto start = Builder.CreateGEP(code_block, {GetInt(0), GetInt(0)}, "start");
    Builder.CreateStore(start, pc);
    Builder.CreateBr(Next);

    Builder.SetInsertPoint(Next);
    auto current_pc = Builder.CreateLoad(pc);
    Builder.CreateStore(current_pc, w);
    Builder.CreateStore(Builder.CreateGEP(current_pc, GetInt(1)), pc);
    auto node = Builder.CreateLoad(current_pc);
    auto addr = Builder.CreateLoad(Builder.CreateGEP(node, {GetInt(0), GetInt(2)}));
    auto br = Builder.CreateIndirectBr(addr, NativeBlocks.size());
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
