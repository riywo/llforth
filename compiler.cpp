//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#include "engine.h"
#include "dict.h"
#include "words.h"
#include "stack.h"
#include "util.h"

struct Token {
    enum Type {Word, Branch, Branch0} type;
    std::string value;
};

static std::string GetLabel(const std::string& str) {
    std::smatch sm;
    if (std::regex_match(str, sm, std::regex("(\\..+):"))) {
        return sm[1];
    } else {
        return "";
    }
}

static void MainLoop(std::istream& input, std::vector<Constant*>* code) {
    std::string line;
    std::map<std::string, unsigned long> labels ={};
    std::vector<Token> tokens = {};
    while (std::getline(input, line)) {
        std::string str;
        std::stringstream linestream(line);
        while (linestream >> str) {
            Token token = {};
            if (str == "\\") {
                break;
            } else if (str == "branch") {
                std::string goto_label;
                linestream >> goto_label;
                token = {Token::Type::Branch, goto_label};
            } else if (str == "branch0") {
                std::string goto_label;
                linestream >> goto_label;
                token = {Token::Type::Branch0, goto_label};
            } else {
                auto label = GetLabel(str);
                if (label != "") {
                    labels[label] = tokens.size();
                    continue;
                } else {
                    token = {Token::Type::Word, str};
                }
            }
            tokens.push_back(token);
        }
    }
    for (auto token: tokens) {
        dict::Word word = {};
        switch (token.type) {
            case Token::Type::Word: {
                auto found = dict::Dictionary.find(token.value);
                if (found == dict::Dictionary.end()) {
                    word = words::AddLitWord(token.value);
                } else {
                    word = found->second;
                }
                break;
            }
            case Token::Type::Branch: {
                auto found = labels.find(token.value);
                if (found == labels.end()) {
                    exit(1);
                } else {
                    int label_idx = (int)found->second;
                    int offset = label_idx - (int)code->size() - 1;
                    word = words::AddBranchWord(offset);
                }
                break;
            }
            case Token::Type::Branch0: {
                auto found = labels.find(token.value);
                if (found == labels.end()) {
                    exit(1);
                } else {
                    int label_idx = (int)found->second;
                    int offset = label_idx - (int)code->size() - 1;
                    word = words::AddBranch0Word(offset);
                }
                break;
            }
        }
        code->push_back(word.xt);
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