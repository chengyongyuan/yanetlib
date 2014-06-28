#include <time.h>
#include "fileutil.h"
#include "gtest/gtest.h"

using namespace ::yanetlib::comm;

//Unittest for fileutil
//mainy static class method...so just TEST
TEST(FileUtilTest, BasicTest) {
    EXPECT_TRUE(IsFileExist("."));
    EXPECT_TRUE(IsFileExist("./fileutil_unittest"));
    EXPECT_FALSE(IsFileExist("./fileutil_unittest_not_exist"));
    EXPECT_FALSE(IsFileExist(NULL));

    EXPECT_TRUE(IsDirectory("."));
    EXPECT_FALSE(IsDirectory("fileutil_unittest"));
    EXPECT_FALSE(IsDirectory("./fileutil_unittest_not_exist"));
    EXPECT_FALSE(IsDirectory(NULL));

    EXPECT_EQ(GetFileSize(NULL), 0);
    EXPECT_GT(GetFileSize("fileutil_unittest"), 1000000);
    EXPECT_GT(GetFileSize("."), GetFileSize("fileutil_unittest"));
    EXPECT_EQ(GetFileSize("fileutil_unittest_not_exist"), 0);

    EXPECT_STREQ(GetBaseName("/data/log.log"), "log.log");
    EXPECT_STREQ(GetBaseName("log.log"), "log.log");
    EXPECT_STREQ(GetBaseName("/data"), "data");
    EXPECT_STREQ(GetBaseName("/"), "");
    EXPECT_STREQ(GetBaseName(""), "");
    EXPECT_EQ((unsigned int)GetBaseName(NULL), 0);
}
