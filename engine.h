//
// Created by Ryosuke Iwanaga on 2018-11-03.
//

#ifndef LLVM_FORTH_ENGINE_H
#define LLVM_FORTH_ENGINE_H

#include "core.h"

namespace engine {
    const static auto AddressType = core::Builder.getInt8PtrTy();
    static StructType* CreateXtType() {
        auto xt_type = StructType::create(core::TheContext, "xt");
        auto xt_ptr_type = xt_type->getPointerTo();
        auto xt_ptr_ptr_type = xt_ptr_type->getPointerTo();
        xt_type->setBody({
                                xt_ptr_type,     // Previous word
                                core::StrType,   // Word of node
                                AddressType,     // Implementation address
                                xt_ptr_ptr_type, // Array of xt if colon word
                                core::IntType,   // Embedded integer for lit and branch
                        });
        return xt_type;
    };
    const static auto XtType = CreateXtType();
    const static auto XtPtrType = XtType->getPointerTo();
    const static auto XtPtrPtrType = XtPtrType->getPointerTo();
    static Constant* LastXt = ConstantPointerNull::get(XtPtrType);
    enum XtMember {
        XtPrevious, XtWord, XtAddress, XtColon, XtEmbedded
    };

    static Function* MainFunction;
    static BasicBlock* Entry;
    static BasicBlock* Next;
    static Value* PC;
    static Value* W;
    static Value* SP;
    static Constant* Stack;
    static Value* RSP;
    static Constant* RStack;
    static std::vector<BasicBlock*> NativeBlocks = {};

    struct Word {
        Constant* xt;
        BlockAddress* addr;
    };
    static std::map<std::string, Word> Dictionary = {};

    static Constant* AddXt(const std::string& word, Constant* lastXt, Constant* str,
            BlockAddress* addr, Constant* colon, Constant* embedded) {
        if (!lastXt)   { lastXt   = ConstantPointerNull::get(XtPtrType); }
        if (!str)      { str      = ConstantPointerNull::get(core::StrType); }
        if (!colon)    { colon    = ConstantPointerNull::get(XtPtrPtrType); }
        if (!embedded) { embedded = core::GetInt(0); }
        auto value = ConstantStruct::get(XtType, lastXt, str, addr, colon, embedded);
        return core::CreateGlobalVariable("xt_" + word, XtType, value);
    };

    static Word AddWord(const std::string& word, Constant* xt, BlockAddress* addr) {
        Word w;
        w.xt = xt;
        w.addr = addr;
        Dictionary[word] = w;
        return w;
    };

    static Word AddCompileWord(const std::string& word, BlockAddress* addr, int embedded) {
        auto xt = AddXt(word, nullptr, nullptr, addr, nullptr, core::GetInt(embedded));
        return AddWord(word, xt, addr);
    };

    static Word AddNativeWord(const std::string& word, const std::function<void()>& impl) {
        auto block = core::CreateBasicBlock("i_" + word, MainFunction);
        NativeBlocks.push_back(block);
        core::Builder.SetInsertPoint(block);
        impl();
        auto addr = BlockAddress::get(block);
        auto str = core::Builder.CreateGlobalStringPtr(word, "w_" + word);
        auto xt = AddXt(word, LastXt, str, addr, nullptr, nullptr);
        LastXt = xt;
        return AddWord(word, xt, addr);
    };

    static Word AddColonWord(const std::string& word, BlockAddress* addr, const std::vector<Constant*>& words) {
        auto str = core::Builder.CreateGlobalStringPtr(word, "w_" + word);
        auto words_type = ArrayType::get(XtPtrType, words.size());
        auto words_block = core::CreateGlobalVariable("col_" + word, words_type, ConstantArray::get(words_type, words));
        Constant* idx[] = {core::GetInt(0), core::GetInt(0)};
        auto xts = ConstantExpr::getGetElementPtr(words_type, words_block, idx);
        auto xt = AddXt(word, LastXt, str, addr, xts, nullptr);
        LastXt = xt;
        return AddWord(word, xt, addr);
    };

    static Value* GetXt() {
        return core::Builder.CreateLoad(W);
    };

    static Value* GetXtMember(XtMember member) {
        auto ptr = core::Builder.CreateGEP(GetXt(), {core::GetInt(0), core::GetInt(member)});
        return core::Builder.CreateLoad(ptr);
    };
    static Value* GetXtPrevious() { return GetXtMember(XtPrevious); };
    static Value* GetXtWord()     { return GetXtMember(XtWord);     };
    static Value* GetXtAddress()  { return GetXtMember(XtAddress);  };
    static Value* GetXtColon()    { return GetXtMember(XtColon);    };
    static Value* GetXtEmbedded() { return GetXtMember(XtEmbedded); };

    static void Push(Value* value) {
        auto current_sp = core::Builder.CreateLoad(SP);
        auto addr = core::Builder.CreateGEP(Stack, {core::GetInt(0), current_sp});
        core::Builder.CreateStore(value, addr);
        core::Builder.CreateStore(core::Builder.CreateAdd(current_sp, core::GetInt(1)), SP);
    }

    static LoadInst* Pop() {
        auto current_sp = core::Builder.CreateLoad(SP);
        auto top_sp = core::Builder.CreateSub(current_sp, core::GetInt(1));
        auto addr = core::Builder.CreateGEP(Stack, {core::GetInt(0), top_sp});
        core::Builder.CreateStore(top_sp, SP);
        return core::Builder.CreateLoad(addr);
    }

    static void RPush(Value* value) {
        auto current_rsp = core::Builder.CreateLoad(RSP);
        auto addr = core::Builder.CreateGEP(RStack, {core::GetInt(0), current_rsp});
        core::Builder.CreateStore(value, addr);
        core::Builder.CreateStore(core::Builder.CreateAdd(current_rsp, core::GetInt(1)), RSP);
    }

    static LoadInst* RPop() {
        auto current_rsp = core::Builder.CreateLoad(RSP);
        auto top_rsp = core::Builder.CreateSub(current_rsp, core::GetInt(1));
        auto addr = core::Builder.CreateGEP(RStack, {core::GetInt(0), top_rsp});
        core::Builder.CreateStore(top_rsp, RSP);
        return core::Builder.CreateLoad(addr);
    }

    static void CreateRet(int ret) {
        core::Builder.CreateRet(core::GetInt(ret));
    };

    static void CreateBrNext() {
        core::Builder.CreateBr(Next);
    };

    static void Initialize() {
        core::Func func = {"main", FunctionType::get(core::IntType, false)};
        MainFunction = core::CreateFunction(func);
        Entry = core::CreateBasicBlock("entry", MainFunction);
        Next = core::CreateBasicBlock("next", MainFunction);

        core::Builder.SetInsertPoint(Entry);
        PC = core::Builder.CreateAlloca(XtPtrPtrType, nullptr, "pc");
        W = core::Builder.CreateAlloca(XtPtrType, nullptr, "w");

        auto stack_type = ArrayType::get(core::IntType, 1024);
        SP = core::Builder.CreateAlloca(core::IntType, nullptr, "sp");
        core::Builder.CreateStore(core::GetInt(0), SP);
        Stack = core::CreateGlobalVariable("stack", stack_type, UndefValue::get(stack_type), false);

        auto rstack_type = ArrayType::get(XtPtrPtrType, 1024);
        RSP = core::Builder.CreateAlloca(core::IntType, nullptr, "rsp");
        core::Builder.CreateStore(core::GetInt(0), RSP);
        RStack = core::CreateGlobalVariable("rstack", rstack_type, UndefValue::get(rstack_type), false);
    };

    static void Finalize(const std::vector<Constant*>& code) {
        core::Builder.SetInsertPoint(Entry);
        auto code_type = ArrayType::get(XtPtrType, code.size());
        auto code_block = core::CreateGlobalVariable("code", code_type, ConstantArray::get(code_type, code));
        auto start = core::Builder.CreateGEP(code_block, {core::GetInt(0), core::GetInt(0)});
        core::Builder.CreateStore(start, PC);
        core::Builder.CreateBr(Next);

        core::Builder.SetInsertPoint(Next);
        auto pc = core::Builder.CreateLoad(PC);
        core::Builder.CreateStore(core::Builder.CreateLoad(pc), W);
        auto new_pc = core::Builder.CreateGEP(pc, core::GetInt(1));
        core::Builder.CreateStore(new_pc, PC);
        auto br = core::Builder.CreateIndirectBr(GetXtAddress(), (unsigned int)NativeBlocks.size());
        for (auto block : NativeBlocks) { br->addDestination(block); }
    };
}

#endif //LLVM_FORTH_ENGINE_H
