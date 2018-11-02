//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#include "core.h"
#include "util.h"
#include "dict.h"

int main() {
    core::CreateModule("main");
    auto mainFunc = core::CreateFunction(core::MainFunc);
    auto entry = core::CreateBasicBlock("entry", mainFunc);
    core::Builder.SetInsertPoint(entry);
    core::CallFunction(words::FindNodeFunc, {ConstantPointerNull::get(core::StrType)});
    core::Builder.CreateRet(core::GetInt(0));
    core::Dump();
}