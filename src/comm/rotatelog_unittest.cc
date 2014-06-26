#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "rotate_log.h"


using namespace std;
using namespace ::yanetlib::comm;

class RotateLogTest : public ::testing::Test {
    protected:
        RotateLogTest() {}
        
        ~RotateLogTest(){}

        virtual void SetUp() {
            assert(log1.Init(RotateLog::DEBUG, "log_by_size", RotateLog::ROTATE_BY_SIZE, 50)); 
            assert(log2.Init(RotateLog::DEBUG, "log_by_hour", RotateLog::ROTATE_BY_DAY, 100));
            assert(log3.Init(RotateLog::DEBUG, "log_by_minute", RotateLog::ROTATE_BY_HOUR, 100));
        }

        virtual void TearDown() {
        }
        //log by size
        RotateLog log1;
        //log by hour
        RotateLog log2;
        //log by minute
        RotateLog log3;
};

TEST_F(RotateLogTest, TestBySize) {
    LOG_DEBUG(log1, "%s", "1234567890");
    LOG_DEBUG(log1, "%s", "1234567890");
    LOG_DEBUG(log1, "%s", "1234567890");
    struct stat stbuf;
    EXPECT_EQ(0, stat("log_by_size.log", &stbuf));
    EXPECT_TRUE(stbuf.st_size < 50);
}
