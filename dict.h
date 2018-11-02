//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#ifndef LLVM_FORTH_DICT_H
#define LLVM_FORTH_DICT_H

#include "core.h"
#include "util.h"

namespace dict {
    const static auto NodeType = StructType::create(core::TheContext, "node");
    const static auto NodePtrType = NodeType->getPointerTo();
    const static auto NodePtrPtrType = NodePtrType->getPointerTo();

//    static auto LastNode = core::CreateGlobalVariable("last_node", NodePtrType);

    const static Constant* GetLastNode() {
        return core::CreateGlobalVariable("last_node", NodePtrType);
    };

    const static std::vector<Type*> NodeInputTypes = {
            core::StrType,     // Word of node
            core::AddressType, // BlockAddress
            core::XTPtrType,   // Array of xt if colon word
            core::IntType      // Integer if lit
    };

    const static std::vector<Type*> NodeMemberTypes = {
            NodePtrType,       // Previous node
            core::StrType,     // Word of node
            core::AddressType, // BlockAddress
            core::XTPtrType,   // Array of xt if colon word
            core::IntType      // Integer if lit
    };

    const static core::Func FindNodeFunc {
        "find_node", FunctionType::get(NodePtrType, {core::StrType}, false)
    };

    const static core::Func CreateNodeFunc {
        "create_node", FunctionType::get(NodePtrType, NodeInputTypes, false)
    };

    /**
    static Constant* AddNativeWord(const std::string& word, const std::function<void()>& impl) {
        if (!core::MainFunction) {
            throw "AddNativeWord() must run with main module";
        }
        auto block = BasicBlock::Create(core::TheContext, "i_" + word, core::MainFunction);
        core::Builder.SetInsertPoint(block);
        impl();
        auto xt = BlockAddress::get(block);
        auto str_ptr = core::Builder.CreateGlobalStringPtr(word, "k_" + word);
        auto last_node = core::Builder.CreateLoad(LastNode);
        auto no_col = ConstantPointerNull::get(core::XTPtrType);
//        auto node_val = ConstantStruct::get(NodeType, last_node, str_ptr, xt, no_col, core::GetInt(0));
//        auto node = CreateGlobalVariable("d_" + word, NodeType, node_val);
//        LastNode = node;
//        return node;
        return ConstantPointerNull::get(NodePtrType);
    };
        **/
}

#endif //LLVM_FORTH_DICT_H
