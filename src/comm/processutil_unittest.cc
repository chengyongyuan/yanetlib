#include "processutil.h"
#include "gtest/gtest.h"

using namespace ::yanetlib::comm;

//Unittest for processutil
//mainy static class method...so just TEST
TEST(ProcessUtilTest, BasicTest) {
    EXPECT_FALSE(MySystem(NULL));
    EXPECT_TRUE(MySystem("echo %d%s", 0, "K"));
    char *str = new char[1024*1024 + 1];
    memset(str, 0xff, 1024*1024);
    str[1024*1024] = 0;
    EXPECT_FALSE(MySystem(str));
}
