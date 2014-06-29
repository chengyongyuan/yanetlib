#include "net_common.h"
#include "gtest/gtest.h"

using namespace std;
using namespace yanetlib::net;

class NetCommUnitTest : public testing::Test {
 protected:
     NetCommUnitTest() {}
     ~NetCommUnitTest() {}
};

TEST_F(NetCommUnitTest, BasicTest) {
    EXPECT_TRUE(1);
}
