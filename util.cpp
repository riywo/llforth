//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#include "util.h"

using namespace util;

const static core::Func _printf {
        "printf", FunctionType::get(core::IntType, {core::StrType}, true)
};

int main() {
    core::CreateModule("util");

    core::CreateFunction(PrintFunc, [](Function* f){
        auto arg = f->arg_begin();
        auto d = core::Builder.CreateGlobalStringPtr("%d ");
        core::CallFunction(_printf, {d, arg});
        core::Builder.CreateRetVoid();
    });

    core::Dump();
}