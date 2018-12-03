//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#include "engine.h"
#include "dict.h"
#include "words.h"
#include "stack.h"
#include "util.h"

struct Token {
    enum Type {String, Br, Label, Colon, Semicolon, BrLabel, Immediate} type;
    std::string value;

    explicit Token(const std::string& str) {
        std::smatch label_match;
        if (std::regex_match(str, label_match, std::regex("(\\..+):"))) {
            type = Label;
            value = label_match[1];
        } else {
            value = str;
            if (str == ":") { type = Colon; }
            else if (str == ";") { type = Semicolon; }
            else if (str == "branch" || str == "0branch") { type = Br; }
            else if (str == "immediate") { type = Immediate; }
            else { type = String; }
        }
    }
};

std::ostream& operator<<(std::ostream& os, const Token& token) {
    return os << token.type << " " << token.value;
}

static std::vector<Token> Tokenize(std::istream& input) {
    std::string line;
    std::vector<Token> tokens = {};
    while (std::getline(input, line)) {
        std::string str;
        std::stringstream linestream(line);
        while (linestream >> str) {
            if (str == "\\") {
                break;
            } else {
                tokens.emplace_back(Token(str));
            }
        }
    }
    return tokens;
}

struct WordDefinition {
    struct Code {
        enum Type {Word, Int, BrLabel} type;
        std::string value;
        Constant* xt;
    };

    std::string name;
    bool is_immediate = false;
    std::map<std::string, int> labels = {};
    std::vector<Token> tokens = {};
    std::vector<Code> codes = {};

    void add_token(const Token& token) {
        if (token.type == Token::Type::String && name.empty()) {
            assert(tokens.empty());
            name = token.value;
        } else {
            tokens.push_back(token);
        }
    }

    void preprocess() {
        for (auto token: tokens) {
            switch (token.type) {
                case Token::Type::Label: {
                    labels[token.value] = (int)codes.size();
                    break;
                }
                case Token::Type::BrLabel: {
                    codes.push_back(Code{.type=Code::Type::BrLabel, .value=token.value});
                    break;
                }
                case Token::Type::String: {
                    auto found = dict::Dictionary.find(token.value);
                    if (found == dict::Dictionary.end()) {
                        codes.push_back(Code{.type=Code::Type::Word, .xt = words::Lit.xt, .value=token.value});
                        codes.push_back(Code{.type=Code::Type::Int, .xt = words::GetConstantIntToXtPtr(std::stoi(token.value)), .value=token.value});
                    } else {
                        codes.push_back(Code{.type=Code::Type::Word, .xt = found->second.xt, .value = token.value});
                    }
                    break;
                }
                default: {
                    assert(false);
                }
            }
        }
    }

    void compile() {
        preprocess();
        auto compiled = std::vector<std::variant<Constant*,int>>();
        for (auto code: codes) {
            switch (code.type) {
                case Code::Type::BrLabel: {
                    auto found = labels.find(code.value);
                    if (found == labels.end()) {
                        assert(false);
                    } else {
                        compiled.push_back(found->second);
                    }
                    break;
                }
                case Code::Type::Word: {
                    assert(code.xt != nullptr);
                    compiled.push_back(code.xt);
                    break;
                }
                case Code::Type::Int: {
                    try {
                        auto xt = words::GetConstantIntToXtPtr(std::stoi(code.value));
                        compiled.push_back(xt);
                    } catch(...) {
                        assert(false);
                    }
                    break;
                }
            }
        }
        dict::AddColonWord(name, words::Docol.addr, compiled, is_immediate);
    }
};

std::ostream& operator<<(std::ostream& os, const WordDefinition::Code& code) {
    return os << code.value;
}

std::ostream& operator<<(std::ostream& os, const WordDefinition& word) {
    os << word.name << " ";
    for (auto t: word.tokens) { os << t << ", "; }
    for (auto p: word.labels) { os << p.first << "=>" << p.second << " "; }
    for (auto c: word.codes) { os << c << ", "; }
    return os;
}

static std::vector<WordDefinition> Parse(const std::vector<Token>& tokens) {
    std::vector<WordDefinition> words = {};
    WordDefinition def;
    bool is_def = false;
    bool is_label = false;
    for (auto token: tokens) {
        switch (token.type) {
            case Token::Type::Colon: {
                assert(!is_def);
                is_def = true;
                def = WordDefinition();
                break;
            }
            case Token::Type::Semicolon: {
                assert(is_def);
                words.push_back(def);
                is_def = false;
                break;
            }
            case Token::Type::Br: {
                assert(is_def);
                token.type = Token::Type::String;
                def.add_token(token);
                is_label = true;
                break;
            }
            case Token::Type::String: {
                assert(is_def);
                if (is_label) {
                    token.type = Token::Type::BrLabel;
                    is_label = false;
                }
                def.add_token(token);
                break;
            }
            case Token::Type::Label: {
                assert(is_def);
                def.add_token(token);
                break;
            }
            case Token::Type::Immediate: {
                assert(!is_def);
                def.is_immediate = true;
                break;
            }
            default: {
                assert(false);
            }
        }
    }
    return words;
}

static void MainLoop(std::istream& input) {
    auto tokens = Tokenize(input);
    auto words = Parse(tokens);
    for (auto w: words) {
        w.compile();
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