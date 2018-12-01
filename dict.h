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
        xt_type->setBody({
            xt_ptr_type,     // Previous word
            core::StrType,   // Word of node
            AddressType,     // Implementation address
            core::IndexType,
            core::BoolType,  // Immediate flag
        });
        return xt_type;
    };
    const static auto XtType = CreateXtType();
    const static auto XtPtrType = XtType->getPointerTo();
    const static auto XtPtrPtrType = XtPtrType->getPointerTo();
    const static auto XtPtrNull = ConstantPointerNull::get(XtPtrType);
    static Constant* _LastXt = XtPtrNull;
    static Constant* LastXt;
    enum XtMember {
        XtPrevious, XtWord, XtImplAddress, XtColon, XtImmediate,
    };

    static std::vector<Constant*> InitialMemory = {};
    static Constant* Memory;
    static Constant* HereValue;

    struct Word {
        Constant* xt;
        BlockAddress* addr;
        BasicBlock* block;
    };
    static std::vector<BasicBlock*> NativeBlocks = {};
    static std::map<std::string, Word> Dictionary = {};

    static Constant* GetConstantIntToXtPtr(int num) {
        return ConstantExpr::getIntToPtr(ConstantInt::get(core::IntType, num), XtPtrType);
    }

    static Constant* AddXt(const std::string& word, Constant* lastXt, Constant* str,
                           BlockAddress* addr, Constant* colon, Constant* flag) {
        if (!lastXt)   { lastXt   = ConstantPointerNull::get(XtPtrType); }
        if (!str)      { str      = ConstantPointerNull::get(core::StrType); }
        if (!colon)    { colon    = core::GetIndex(-1); }
        if (!flag)     { flag     = core::GetBool(false); }
        auto value = ConstantStruct::get(XtType, lastXt, str, addr, colon, flag);
        return core::CreateGlobalVariable("xt_" + word, XtType, value);
    };

    static Word AddWord(const std::string& word, Constant* xt, BlockAddress* addr, BasicBlock* block=nullptr) {
        Word w;
        w.xt = xt;
        w.addr = addr;
        w.block = block;
        Dictionary[word] = w;
        return w;
    };

    static Word AddNativeWord(const std::string& word, const std::function<void()>& impl) {
        auto block = core::CreateBasicBlock("i_" + word, engine::MainFunction);
        core::Builder.SetInsertPoint(block);
        impl();
        NativeBlocks.push_back(block);
        auto addr = BlockAddress::get(block);
        auto str = core::Builder.CreateGlobalStringPtr(word);
        auto xt = AddXt(word, _LastXt, str, addr, nullptr, nullptr);
        _LastXt = xt;
        return AddWord(word, xt, addr, block);
    };

    static Word AddColonWord(const std::string& word, BlockAddress* addr, std::vector<std::variant<Constant*,int>> words, bool flag=false) {
        auto str = core::Builder.CreateGlobalStringPtr(word);
        auto start = InitialMemory.size();
        auto here = core::GetIndex(start);
        std::vector<Constant*> compiled_words = {};
        for (auto word: words) {
            try {
                auto i = std::get<int>(word);
                compiled_words.push_back(GetConstantIntToXtPtr(start + i));
            }
            catch (const std::bad_variant_access&) {
                compiled_words.push_back(std::get<Constant*>(word));
            }
        }
        InitialMemory.insert(InitialMemory.end(), compiled_words.begin(), compiled_words.end());
        auto xt = AddXt(word, _LastXt, str, addr, here, core::GetBool(flag));
        _LastXt = xt;
        return AddWord(word, xt, addr);
    };

    static Value* GetXt() {
        return core::Builder.CreateLoad(engine::W);
    };

    static Value* GetXtMember(Value* xt, XtMember member) {
        auto ptr = core::Builder.CreateGEP(xt, {core::GetIndex(0), core::GetIndex(member)});
        return core::Builder.CreateLoad(ptr);
    };
    static Value* GetXtMember(XtMember member) {
        return GetXtMember(GetXt(), member);
    };
    static Value* GetXtPrevious(Value* xt)    { return GetXtMember(xt, XtPrevious);    };
    static Value* GetXtWord(Value* xt)        { return GetXtMember(xt, XtWord);        };
    static Value* GetXtImplAddress(Value* xt) { return GetXtMember(xt, XtImplAddress); };
    static Value* GetXtColon(Value* xt)       { return GetXtMember(xt, XtColon);       };
    static Value* GetXtImmediate(Value* xt)   { return GetXtMember(xt, XtImmediate);   };
    static Value* GetXtPrevious()    { return GetXtMember(XtPrevious);    };
    static Value* GetXtWord()        { return GetXtMember(XtWord);        };
    static Value* GetXtImplAddress() { return GetXtMember(XtImplAddress); };
    static Value* GetXtColon()       { return GetXtMember(XtColon);       };
    static Value* GetXtImmediate()   { return GetXtMember(XtImmediate);   };

    static Value* GetLastXt() {
        return core::Builder.CreateLoad(LastXt);
    };

    static void Initialize(Function* main, BasicBlock* entry) {
        Memory = core::CreateGlobalVariable("dict_memory", ArrayType::get(XtPtrType, 1024));
        HereValue = core::CreateGlobalVariable("here", core::IndexType);
        engine::PC = core::Builder.CreateAlloca(XtPtrPtrType, nullptr, "pc");
        engine::W = core::Builder.CreateAlloca(XtPtrType, nullptr, "w");
        LastXt = core::CreateGlobalVariable("last_xt", XtPtrType);
        engine::Jump = [](){
            auto br = core::Builder.CreateIndirectBr(GetXtImplAddress(), (unsigned int)NativeBlocks.size());
            for (auto block : NativeBlocks) {
                br->addDestination(block);
            }
        };
    }

    static void Finalize(const std::vector<std::variant<Constant*,int>>& code) {
        HereValue = core::CreateGlobalVariable("here", core::IndexType, core::GetIndex(InitialMemory.size()), false);
        InitialMemory.resize(1024, ConstantPointerNull::get(XtPtrType));
        Memory = core::CreateGlobalArrayVariable("dict_memory", XtPtrType, InitialMemory, false);
        auto start = core::Builder.CreateGEP(Memory, {core::GetIndex(0), core::GetIndex(0)});
        core::Builder.CreateStore(start, engine::PC);
        LastXt = core::CreateGlobalVariable("last_xt", XtPtrType, _LastXt, false);
    }
}

#endif //LLVM_FORTH_DICT_H
