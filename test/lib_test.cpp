//
// Created by Ryosuke Iwanaga on 2018-11-17.
//

#include "gtest/gtest.h"
#include "../lib.h"

namespace {
    class LibTest: public ::testing::Test {
    protected:
        void SetUp() override {
            outbuf.str("");
            org_out = std::cout.rdbuf();
            std::cout.rdbuf(outbuf.rdbuf());
        }

        void TearDown() override {
            std::cout.rdbuf(org_out);
        }

        std::stringstream outbuf;
        std::streambuf* org_out = nullptr;

        std::string out() {
            return outbuf.str();
        }
    };

    TEST_F(LibTest, PrintInt) {
        print_int(9999999999);
        ASSERT_EQ(out(), "9999999999 ");
    }

    TEST_F(LibTest, ReadWord) {
        char buf[1024];
        std::istringstream input("word1 word2\nword3");
        ASSERT_EQ(read_word(buf, 1024, input), 5);
        ASSERT_STREQ(buf, "word1");
        ASSERT_EQ(read_word(buf, 1024, input), 5);
        ASSERT_STREQ(buf, "word2");
        ASSERT_EQ(read_word(buf, 1024, input), 5);
        ASSERT_STREQ(buf, "word3");
    }
}
