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

struct Node {
    enum Type {Word, Int, GotoLabel} type;
    Constant* xt;
    std::string value;
};

static Token StringToToken(const std::string& str) {
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

static std::vector<Token> Tokenize(std::istream& input) {
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
                token = StringToToken(str);
            }
            tokens.push_back(token);
        }
    }
    return tokens;
}

static std::tuple<std::vector<Node>,std::map<std::string, int>> Parse(const std::vector<Token>& tokens) {
    std::vector<Node> nodes = {};
    std::map<std::string, int> labels = {};
    for (auto token: tokens) {
        switch (token.type) {
            case Token::Type::Word: {
                nodes.push_back(Node {Node::Type::Word, token.xt});
                break;
            }
            case Token::Type::Br: {
                nodes.push_back(Node {Node::Type::Word, token.xt});
                nodes.push_back(Node {Node::Type::GotoLabel, nullptr, token.value});
                break;
            }
            case Token::Type::Label: {
                labels[token.value] = (int)nodes.size();
                break;
            }
            case Token::Type::Int: {
                nodes.push_back(Node {Node::Type::Word, token.xt});
                nodes.push_back(Node {Node::Type::Int, nullptr, token.value});
                break;
            }
        }
    }
    return std::make_tuple(nodes, labels);
}

static std::vector<std::variant<Constant*,int>> Compile(const std::vector<Node>& nodes, const std::map<std::string, int>& labels) {
    std::vector<std::variant<Constant*,int>> codes = {};
    for (auto node: nodes) {
        std::variant<Constant*,int> xt;
        switch (node.type) {
            case Node::Type::Word: {
                xt = node.xt;
                break;
            }
            case Node::Type::GotoLabel: {
                auto found = labels.find(node.value);
                if (found == labels.end()) {
                    exit(-1);
                } else {
                    xt = found->second;
                }
                break;
            }
            case Node::Type::Int: {
                xt = words::GetConstantIntToXtPtr(std::stoi(node.value));
                break;
            }
        }
        codes.push_back(xt);
    }
    return codes;
}

static void MainLoop(std::istream& input) {
    auto tokens = Tokenize(input);
    auto [nodes, labels] = Parse(tokens);
    auto code = Compile(nodes, labels);
    dict::AddColonWord("main", words::Docol.addr, code);
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
    auto code = std::vector<std::variant<Constant*,int>>();
    if (argc > 1) {
        std::ifstream input(argv[1]);
        if (input) {
            MainLoop(input);
        } else {
            std::cerr << "No such file: " << argv[1] << std::endl;
            exit(1);
        }
    } else {
        MainLoop(std::cin);
    }
    engine::Finalize(code);
    core::DumpModule();
}