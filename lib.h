//
// Created by Ryosuke Iwanaga on 2018-12-22.
//

#ifndef LLFORTH_LIB_H
#define LLFORTH_LIB_H

class Reader {
public:
    Reader(int, char**);
    ~Reader();
    std::optional<std::string> read();

private:
    void* raw;
};

#endif //LLFORTH_LIB_H
