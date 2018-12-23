//
// Created by Ryosuke Iwanaga on 2018-11-02.
//

#include "engine.h"
#include "dict.h"
#include "words.h"
#include "stack.h"
#include "util.h"
#include "lib.h"

extern "C" {
    void* create_reader(int, char**);
    int read_word_from_reader(void*, char*, int);
    void destroy_reader(void*);
}

Reader::Reader(int argc, char** argv) {
    raw = create_reader(argc, argv);
}

Reader::~Reader() {
    destroy_reader(raw);
}

std::optional<std::string> Reader::read() {
    char buf[1024];
    char* ptr = buf;
    int num = read_word_from_reader(raw, ptr, 1024);
    if (num == 0) {
        switch(buf[0]) {
            case 0:
            case 10:
                return read();
            default:
                return std::nullopt;
        }
    }
    std::string s(buf);
    assert(s.size() == num);
    return s;
}

struct Token {
    enum Type {
        String,
        Br,
        Label,
        Colon,
        Semicolon,
        BrLabel,
        Immediate,
        LitToken,
    } type;
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
            else if (str == "'") { type = LitToken; }
            else { type = String; }
        }
    }
};

std::ostream& operator<<(std::ostream& os, const Token& token) {
    return os << token.type << " " << token.value;
}

static std::vector<Token> Tokenize(Reader* reader) {
    std::string line;
    std::vector<Token> tokens = {};
    while (auto str = reader->read()) {
        auto token = Token(*str);
        tokens.emplace_back(token);
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
                        codes.push_back(Code{.type=Code::Type::Word, .xt = words::Lit.xt, .value="lit"});
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
    //for (auto t: word.tokens) { os << t << ", "; }
    //for (auto p: word.labels) { os << p.first << "=>" << p.second << " "; }
    for (auto c: word.codes) { os << c << " "; }
    return os;
}

static std::vector<WordDefinition> Parse(const std::vector<Token>& tokens) {
    std::vector<WordDefinition> words = {};
    WordDefinition def;
    bool is_def = false;
    bool is_label = false;
    bool is_lit_token = false;
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
                def.add_token(Token("exit"));
                words.push_back(def);
                is_def = false;
                break;
            }
            case Token::Type::Br: {
                assert(is_def);
                if (is_lit_token) {
                    is_lit_token = false;
                } else {
                    assert(!is_label);
                    is_label = true;
                }
                token.type = Token::Type::String;
                def.add_token(token);
                break;
            }
            case Token::Type::String: {
                assert(is_def);
                if (is_label) {
                    token.type = Token::Type::BrLabel;
                    is_label = false;
                }
                if (is_lit_token) {
                    is_lit_token = false;
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
                auto last_word = words.back();
                words.pop_back();
                last_word.is_immediate = true;
                words.push_back(last_word);
                break;
            }
            case Token::Type::LitToken: {
                assert(is_def);
                assert(!is_lit_token);
                is_lit_token = true;
                token.type = Token::Type::String;
                token.value = "lit";
                def.add_token(token);
                break;
            }
            default: {
                assert(false);
            }
        }
    }
    return words;
}

static void MainLoop(int argc, char** argv) {
    Reader reader(argc, argv);
    auto tokens = Tokenize(&reader);
    auto words = Parse(tokens);
    for (auto w: words) {
        w.compile();
        std::cerr << w << std::endl;
    }
}

int main(int argc, char** argv) {
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

    MainLoop(argc, argv);

    engine::Finalize();
    core::DumpModule();
}