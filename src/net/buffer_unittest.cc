#include "gtest/gtest.h"
#include "net_common.h"
#include "buffer.hpp"

using namespace std;
using namespace yanetlib::net;

static const int kBufSize = 1<<20;

class BufferUnittest : public testing::Test {
 protected:
     BufferUnittest() {}
     ~BufferUnittest() {}
     Buffer<32> buffer;
};

TEST_F(BufferUnittest, InitTest) {
    Buffer<kBufSize> buffer1;
    Buffer<kBufSize> buffer2;
    EXPECT_TRUE(buffer1.InitBuffer(3));
    EXPECT_TRUE(buffer2.InitBuffer(1));
}


TEST_F(BufferUnittest, BasicTest) {
    char *buf = NULL;
    int len = 0;
    //32*2 bytes
    EXPECT_TRUE(buffer.InitBuffer(2));
    EXPECT_TRUE(buffer.GetSpace(buf, len));
    EXPECT_TRUE(buf && len == 32);
    buffer.PushData(32);

    EXPECT_TRUE(buffer.GetSpace(buf, len));
    EXPECT_TRUE(buf && len == 32);
    buffer.PushData(33);
    EXPECT_FALSE(buffer.GetSpace(buf, len));

    //buffer.PushData(0);
    buffer.PopData(64);
    EXPECT_TRUE(buffer.GetSpace(buf, len));
    EXPECT_TRUE(buf && len == 32);
}
