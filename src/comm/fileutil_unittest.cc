#include <time.h>
#include "fileutil.h"
#include "gtest/gtest.h"

using namespace ::yanetlib::comm;

//Unittest for fileutil
//mainy static class method...so just TEST
TEST(FileUtilTest, BasicTest) {
    EXPECT_EQ(IsFileExist("."), true);
    EXPECT_EQ(IsFileExist("./fileutil_unittest"), true);
    EXPECT_EQ(IsFileExist("./fileutil_unittest_not_exist"), false);
}
