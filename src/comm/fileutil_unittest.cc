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
    EXPECT_EQ(IsFileExist(NULL), false);

    EXPECT_EQ(IsDirectory("."), true);
    EXPECT_EQ(IsDirectory("fileutil_unittest"), false);
    EXPECT_EQ(IsDirectory("./fileutil_unittest_not_exist"), false);
    EXPECT_EQ(IsDirectory(NULL), false);

    EXPECT_EQ(GetFileSize(NULL), 0);
    EXPECT_EQ(GetFileSize("fileutil_unittest") > 1000000, true);
    EXPECT_EQ(GetFileSize(".") > GetFileSize("fileutil_unittest") , true);
    EXPECT_EQ(GetFileSize("fileutil_unittest_not_exist"), 0);
}
