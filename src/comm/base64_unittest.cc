#include <string.h>
#include <string>
#include "base64.h"
#include "gtest/gtest.h"

using namespace std;
using namespace yanetlib::comm;

class Base64UnitTest : public testing::Test {
 protected:
     Base64UnitTest(){}
     ~Base64UnitTest(){}
};

TEST_F(Base64UnitTest, TestEncode) {
    Base64Encoder encoder(16);
    string input("\00\01\02\03\04\05", 6);
    char outbuf[512];
    int ilen = sizeof(outbuf);
    EXPECT_EQ(0, encoder.Encode(input.c_str(), 6, outbuf, &ilen));
    outbuf[ilen] = '\0';
    EXPECT_STREQ("AAECAwQF", outbuf);

    //reset
    ilen = sizeof(outbuf);
    input.assign("\00\01\02\03", 4);
    EXPECT_EQ(0, encoder.Encode(input.c_str(), 4, outbuf, &ilen));
    outbuf[ilen] = '\0';
    EXPECT_STREQ("AAECAw==", outbuf);

    //reset
    ilen = sizeof(outbuf);
    input = "ABCDEF";
    EXPECT_EQ(0, encoder.Encode(input.c_str(), 6, outbuf, &ilen));
    outbuf[ilen] = '\0';
    EXPECT_STREQ("QUJDREVG", outbuf);

    //reset
    ilen = sizeof(outbuf);
    input = "ABCDEFHIGKLMNOKPIST";
    EXPECT_EQ(0, encoder.Encode(input.c_str(), input.size(), outbuf, &ilen));
    outbuf[ilen] = '\0';
    EXPECT_STREQ("QUJDREVGSElHS0xN\r\nTk9LUElTVA==", outbuf);

}
TEST_F(Base64UnitTest, TestDecode) {
    string input = "QUJDREVG";
    string orgi = "ABCDEF";
    char outbuf[512];
    int ilen = sizeof(outbuf);
    Base64Decoder decoder;
    EXPECT_EQ(0, decoder.Decode(input.c_str(),input.size(), outbuf, &ilen));
    EXPECT_EQ(0, memcmp(outbuf, orgi.data(), ilen));

    //reset
    input = "AAECAw==";
    ilen = sizeof(outbuf);
    orgi.assign("\00\01\02\03", 4);
    EXPECT_EQ(0, decoder.Decode(input.c_str(),input.size(), outbuf, &ilen));
    EXPECT_EQ(0, memcmp(outbuf, orgi.data(), ilen));

    //reset
    input = "AAECAwQF";
    ilen = sizeof(outbuf);
    orgi.assign("\00\01\02\03\04\05\06", 6);
    EXPECT_EQ(0, decoder.Decode(input.c_str(),input.size(), outbuf, &ilen));
    EXPECT_EQ(0, memcmp(outbuf, orgi.data(), ilen));

    //reset
    input = "QUJDREVGSElHS0xN\r\nTk9LUElTVA==";
    ilen = sizeof(outbuf);
    orgi = "ABCDEFHIGKLMNOKPIST";
    EXPECT_EQ(0, decoder.Decode(input.c_str(),input.size(), outbuf, &ilen));
    EXPECT_EQ(0, memcmp(outbuf, orgi.data(), ilen));
}
