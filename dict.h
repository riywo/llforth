//
// Created by Ryosuke Iwanaga on 2018-11-04.
//

#ifndef LLVM_FORTH_DICT_H
#define LLVM_FORTH_DICT_H

#include "core.h"
#include "engine.h"

namespace dict {
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
        XtPrevious, XtWord, XtImplAddress, XtColon, XtEmbedded
    };

    struct Word {
        Constant* xt;
        BlockAddress* addr;
    };
    static std::vector<BasicBlock*> NativeBlocks = {};
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
        auto block = core::CreateBasicBlock("i_" + word, engine::MainFunction);
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
        auto words_array = core::CreateGlobalArrayVariable("col_" + word, XtPtrType, words);
        auto xts = core::CreateConstantGEP(words_array);
        auto xt = AddXt(word, LastXt, str, addr, xts, nullptr);
        LastXt = xt;
        return AddWord(word, xt, addr);
    };

    static Value* GetXt() {
        return core::Builder.CreateLoad(engine::W);
    };

    static Value* GetXtMember(XtMember member) {
        auto ptr = core::Builder.CreateGEP(GetXt(), {core::GetIndex(0), core::GetIndex(member)});
        return core::Builder.CreateLoad(ptr);
    };
    static Value* GetXtPrevious()    { return GetXtMember(XtPrevious);    };
    static Value* GetXtWord()        { return GetXtMember(XtWord);        };
    static Value* GetXtImplAddress() { return GetXtMember(XtImplAddress); };
    static Value* GetXtColon()       { return GetXtMember(XtColon);       };
    static Value* GetXtEmbedded()    { return GetXtMember(XtEmbedded);    };

    static void Initialize(Function* main, BasicBlock* entry) {
        engine::PC = core::Builder.CreateAlloca(XtPtrPtrType, nullptr, "pc");
        engine::W = core::Builder.CreateAlloca(XtPtrType, nullptr, "w");
    }

    static void Finalize(const std::vector<Constant*>& code) {
        auto code_array = core::CreateGlobalArrayVariable("code", XtPtrType, code);
        auto start = core::Builder.CreateGEP(code_array, {core::GetIndex(0), core::GetIndex(0)});
        core::Builder.CreateStore(start, engine::PC);

        engine::Jump = [](){
            auto br = core::Builder.CreateIndirectBr(GetXtImplAddress(), (unsigned int)NativeBlocks.size());
            for (auto block : NativeBlocks) {
                br->addDestination(block);
            }
        };
    }
}

#endif //LLVM_FORTH_DICT_H
