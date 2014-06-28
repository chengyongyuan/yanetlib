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

    //EXPECT_EQ(EXPECT_VAL, REAL_VAL
    EXPECT_EQ(0UL, GetFileSize(NULL));
    EXPECT_TRUE(GetFileSize("fileutil_unittest") > 1000000);
    EXPECT_TRUE(GetFileSize(".") > GetFileSize("fileutil_unittest"));
    EXPECT_EQ(0UL, GetFileSize("fileutil_unittest_not_exist"));
    EXPECT_EQ(0UL, GetFileSize(NULL));
    EXPECT_GT(GetFileSize("fileutil_unittest"), 1000000UL);
    EXPECT_GT(GetFileSize("."), GetFileSize("fileutil_unittest"));
    EXPECT_EQ(0UL, GetFileSize("fileutil_unittest_not_exist"));

    EXPECT_STREQ("log.log", GetBaseName("/data/log.log"));
    EXPECT_STREQ("log.log", GetBaseName("log.log"));
    EXPECT_STREQ("data", GetBaseName("/data"));
    EXPECT_STREQ("", GetBaseName("/"));
    EXPECT_STREQ("", GetBaseName(""));
    EXPECT_EQ(0UL, GetBaseName(NULL));
}
