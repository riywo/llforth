//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#include "engine.h"
#include "words.h"

static std::vector<Constant*> MainLoop() {
    std::string token;
    auto code = std::vector<Constant*>();
    while (std::cin >> token) {
        auto found = engine::Dictionary.find(token);
        if (found == engine::Dictionary.end()) {
            throw "UNKNOWN";
        } else {
            auto word = found->second;
            if (words::IsLitWord(word)) {
                std::cin >> token;
                word = words::CreateLitWord(token);
            }
            code.push_back(word.xt);
        }
    }
    return code;
}

int main() {
    core::CreateModule("main");
    engine::Initialize();
    words::Initialize();
    auto code = MainLoop();
    engine::Finalize(code);
    core::Dump();
}