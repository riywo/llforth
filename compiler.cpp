//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#include "engine.h"
#include "dict.h"
#include "words.h"
#include "stack.h"

static void MainLoop(std::istream& input, std::vector<Constant*>* code) {
    std::string line;
    std::string token;
    while (std::getline(input, line)) {
        std::stringstream linestream(line);
        while (linestream >> token) {
            if (token == "\\") {
                break;
            }
            dict::Word word = {};
            auto found = dict::Dictionary.find(token);
            if (found == dict::Dictionary.end()) {
                word = words::AddLitWord(token);
            } else {
                word = found->second;
            }
            code->push_back(word.xt);
        }
    }
}

int main(int argc, char* argv[]) {
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
    auto code = std::vector<Constant*>();
    if (argc > 1) {
        std::ifstream input(argv[1]);
        if (input) {
            MainLoop(input, &code);
        } else {
            std::cerr << "No such file: " << argv[1] << std::endl;
            exit(1);
        }
    } else {
        MainLoop(std::cin, &code);
    }
    engine::Finalize(code);
    core::DumpModule();
}