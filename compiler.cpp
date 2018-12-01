//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#include "engine.h"
#include "dict.h"
#include "words.h"
#include "stack.h"
#include "util.h"

struct Token {
    enum Type {Word, Int, Br, Label} type;
    Constant* xt;
    std::string value;
};

struct Code {
    enum Type {Word, Int, GotoLabel} type;
    Constant* xt;
    std::string value;
};

static Token ParseWord(const std::string& str) {
    std::smatch label_match;
    if (std::regex_match(str, label_match, std::regex("(\\..+):"))) {
        return {Token::Type::Label, nullptr, label_match[1]};
    } else {
        auto found = dict::Dictionary.find(str);
        if (found == dict::Dictionary.end()) {
            return {Token::Type::Int, words::Lit.xt, str};
        } else {
            return {Token::Type::Word, found->second.xt, str};
        }
    }
}

static void MainLoop(std::istream& input, std::vector<std::variant<Constant*,int>>* code) {
    std::string line;
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
                token = {Token::Type::Br, words::Branch.xt, goto_label};
            } else if (str == "0branch") {
                std::string goto_label;
                linestream >> goto_label;
                token = {Token::Type::Br, words::Branch0.xt, goto_label};
            } else {
                token = ParseWord(str);
            }
            tokens.push_back(token);
        }
    }
    std::vector<Code> inner_codes = {};
    std::map<std::string, unsigned long> labels = {};
    for (auto token: tokens) {
        switch (token.type) {
            case Token::Type::Word: {
                inner_codes.push_back(Code {Code::Type::Word, token.xt});
                break;
            }
            case Token::Type::Br: {
                inner_codes.push_back(Code {Code::Type::Word, token.xt});
                inner_codes.push_back(Code {Code::Type::GotoLabel, nullptr, token.value});
                break;
            }
            case Token::Type::Label: {
                labels[token.value] = inner_codes.size();
                break;
            }
            case Token::Type::Int: {
                inner_codes.push_back(Code {Code::Type::Word, token.xt});
                inner_codes.push_back(Code {Code::Type::Int, nullptr, token.value});
                break;
            }
        }
    }
    for (auto c: inner_codes) {
        Constant* xt;
        switch (c.type) {
            case Code::Type::Word: {
                xt = c.xt;
                break;
            }
            case Code::Type::GotoLabel: {
                auto found = labels.find(c.value);
                if (found == labels.end()) {
                    exit(-1);
                } else {
                    int label_idx = (int)found->second;
                    int offset = label_idx - (int)code->size();
                    xt = words::GetConstantIntToXtPtr(offset);
                }
                break;
            }
            case Code::Type::Int: {
                xt = words::GetConstantIntToXtPtr(std::stoi(c.value));
                break;
            }
        }
        code->push_back(xt);
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
            words::Finalize,
            dict::Finalize,
    };
    engine::Initialize();
    auto code = std::vector<std::variant<Constant*,int>>();
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