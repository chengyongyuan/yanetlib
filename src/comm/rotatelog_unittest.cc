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
            assert(log1.Init(RotateLog::DEBUG, "log_by_size", RotateLog::ROTATE_BY_SIZE, 200)); 
            assert(log2.Init(RotateLog::DEBUG, "log_by_day", RotateLog::ROTATE_BY_DAY, 100));
            assert(log3.Init(RotateLog::DEBUG, "log_by_hour", RotateLog::ROTATE_BY_HOUR, 100));
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

//TestBySize + TestByCount OK
//TestByCount Done in shell.
TEST_F(RotateLogTest, TestBySize) {
    LOG_DEBUG(log1, "%s", "1234567890");
    LOG_DEBUG(log1, "%s", "1234567890");
    LOG_DEBUG(log1, "%s", "1234567890");
    struct stat stbuf;
    EXPECT_EQ(0, stat("log_by_size.log", &stbuf));
    EXPECT_TRUE(stbuf.st_size <= 200);
}

TEST_F(RotateLogTest, TestByDay) {
    LOG_DEBUG(log2, "%s:%u", "this is only a test", 10);
    LOG_DEBUG(log2, "%s:%u", "THIS IS A TEST", 20);
}

TEST_F(RotateLogTest, TestByHour) {
    LOG_DEBUG(log3, "%s:%u:%g", "this is only a test", 10, 3.14);
    LOG_DEBUG(log3, "%s:%u:%g", "THIS IS A TEST", 20,0.012);
}
