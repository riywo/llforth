//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#include "engine.h"
#include "dict.h"
#include "words.h"
#include "stack.h"

static std::vector<Constant*> MainLoop() {
    std::string token;
    auto code = std::vector<Constant*>();
    while (std::cin >> token) {
        dict::Word word = {};
        auto found = dict::Dictionary.find(token);
        if (found == dict::Dictionary.end()) {
            word = words::AddLitWord(token);
        } else {
            word = found->second;
        }
        code.push_back(word.xt);
    }
    return code;
}

int main() {
    core::CreateModule("main");
    engine::Initializers = {
            dict::Initialize,
            stack::Initialize,
            words::Initialize,
    };
    engine::Finalizers = {
            dict::Finalize,
    };
    engine::Initialize();
    auto code = MainLoop();
    engine::Finalize(code);
    core::Dump();
}