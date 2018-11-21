//
// Created by Ryosuke Iwanaga on 2018-11-17.
//

#ifndef LLFORTH_LIB_H
#define LLFORTH_LIB_H

#include <iostream>
#include <string>
#include <sstream>

extern "C" {
    void print_int(long long int n);
    long long int read_word(char* buf, long long int max);
};

long long int read_word(char* buf, long long int max, std::istream& input);

#endif //LLFORTH_LIB_H
