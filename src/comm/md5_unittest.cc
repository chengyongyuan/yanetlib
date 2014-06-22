#include <string.h>
#include <string>
#include "md5.h"
#include "gtest/gtest.h"

using namespace std;
using namespace ::yanetlib::comm;

//Unit test for Md5 class
class Md5Test : public testing::Test {
    protected:

        Md5Test() {
            pstr1 = "abc";
            pstr2 = "a";
            pstr3 = "this is a test";
            pstr4 = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
        }
        const char *pstr1;
        const char *pstr2;
        const char *pstr3;
        const char *pstr4;
};

TEST_F(Md5Test, BasicTest) {
    //char md5buf[16];
    EXPECT_EQ("d41d8cd98f00b204e9800998ecf8427e", Md5::Md5HashString("", 0));
    EXPECT_EQ("900150983cd24fb0d6963f7d28e17f72", Md5::Md5HashString(pstr1, 3));
    EXPECT_EQ("0cc175b9c0f1b6a831c399e269772661", Md5::Md5HashString(pstr2, 1));
    EXPECT_EQ("54b0c58c7ce9f2a8b551351102ee0938", Md5::Md5HashString(pstr3, strlen(pstr3)));
    EXPECT_EQ("57edf4a22be3c955ac49da2e2107b67a", Md5::Md5HashString(pstr4, strlen(pstr4)));
}
