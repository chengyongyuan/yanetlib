#include <time.h>
#include "datetime.h"
#include "gtest/gtest.h"

using namespace ::yanetlib::comm;

//Unittest for datetime
//mainy static class method...so just TEST
TEST(DateTimeTest, BasicTest) {
    //1403411940  2014-06-21 21:39:00
    time_t t = 1403411940;
    EXPECT_EQ(DateTime::GetDateStr(t), "2014-06-21 21:39:00");
    EXPECT_EQ(DateTime::GetTimeStr(t), "21:39:00");
    EXPECT_EQ(DateTime::GetYear(t), "2014");
    EXPECT_EQ(DateTime::GetMonth(t), "06");
    EXPECT_EQ(DateTime::GetDay(t), "21");
    EXPECT_EQ(DateTime::GetHour(t), "21");
    fprintf(stderr, "CURRENT DATE:%s\n", DateTime::GetDateStr().c_str());
    fprintf(stderr, "CURRENT TIME:%s\n", DateTime::GetTimeStr().c_str());
    fprintf(stderr, "CURRENT YEAR:%s\n", DateTime::GetYear().c_str());
    fprintf(stderr, "CURRENT MONTH:%s\n", DateTime::GetMonth().c_str());
    fprintf(stderr, "CURRENT DAY:%s\n", DateTime::GetDay().c_str());
    fprintf(stderr, "CURRENT HOUR:%s\n", DateTime::GetHour().c_str());
}
