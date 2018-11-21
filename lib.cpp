//

// Created by Ryosuke Iwanaga on 2018-11-17.
//

#include "lib.h"

void print_int(long long int n) {
    std::cout << n << " ";
}

long long int read_word(char* buf, long long int max, std::istream& input) {
    std::string word;
    input >> word;
    assert(word.length() <= max);
    strcpy(buf, word.c_str());
    return (long long int) word.length();
}

long long int read_word(char* buf, long long int max) {
    return read_word(buf, max, std::cin);
}
