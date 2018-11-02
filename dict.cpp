//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#include "dict.h"

using namespace dict;

int main() {
    NodeType->setBody(NodeMemberTypes);

    core::CreateModule("dict");

//    auto lastNode = core::CreateGlobalVariable("last_node", NodePtrType, ConstantPointerNull::get(NodePtrType), false);

    core::CreateFunction(FindNodeFunc, [](Function* f){
        auto arg = f->arg_begin();
        core::Builder.CreateRet(ConstantPointerNull::get(NodePtrType));
    });

    core::CreateFunction(CreateNodeFunc, [=](Function* f){
        auto arg = f->arg_begin();
        auto last = GetLastNode();
        core::Builder.CreateRet(core::Builder.CreateLoad(last));
    });

    core::Dump();
}
