//

// Created by Ryosuke Iwanaga on 2018-11-17.
//

#include "lib.h"

#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
struct termios orig_termios;
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

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
    enableRawMode();
    int c;
    for (int i=0; i<max; i++) {
        c = getchar();
        switch(c) {
            //case EOF:
            case '\n':
            case ' ':
                return i;
            default: {
                buf[i] = (char) c;
                break;
            }
        }
    }
}
