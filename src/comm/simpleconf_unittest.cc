#include "config_parser.h"
#include "gtest/gtest.h"

#include <string>
#include <vector>
using namespace std;
using namespace yanetlib::comm;

TEST(SimpleConfTest, TestOpen) {
    SimpleConf conf;
    EXPECT_FALSE(conf.Init("nonexist.conf"));
    EXPECT_TRUE(conf.Init("sample.conf"));
}

TEST(SimpleConfTest, TestGetVal) {
    int interval = -1;
    double pi = -1.0;
    int port=-1;
    SimpleConf conf;
    EXPECT_TRUE(conf.Init("sample.conf"));
    EXPECT_TRUE(conf.GetIntVal("INTERVAL", interval));
    EXPECT_EQ(1000, interval);
    EXPECT_TRUE(conf.GetFloatVal("PI", pi));
    EXPECT_TRUE(pi-3.14159 < 0.0001);
    EXPECT_FALSE(conf.GetIntVal("PORT", port));
    EXPECT_EQ(0, port);
    
}

TEST(SimpleConfTest, TestGetArray) {
    vector<int> vi;
    vector<string> vs;
    vector<double> vd;
    int interval = -1;
    SimpleConf conf;
    EXPECT_TRUE(conf.Init("sample.conf"));
    EXPECT_TRUE(conf.GetIntArray("PORT_LIST1", vi));
    EXPECT_EQ(4U, vi.size());
    EXPECT_EQ(8000, vi[0]);
    EXPECT_EQ(14000, vi[3]);
    vi.clear();
    EXPECT_TRUE(conf.GetIntArray("PORT_LIST2", vi));
    EXPECT_EQ(4U, vi.size());
    EXPECT_EQ(10, vi[0]);
    EXPECT_EQ(70, vi[3]);

    //string
    vs.clear();
    EXPECT_TRUE(conf.GetArray("IPLIST1", vs));
    EXPECT_EQ(3U, vs.size());
    EXPECT_EQ("10.137.2.221", vs[0]);
    EXPECT_EQ("127.0.0.2", vs[2]);
    vs.clear();
    EXPECT_TRUE(conf.GetArray("IPLIST2", vs));
    EXPECT_EQ(3U, vs.size());
    EXPECT_EQ("10.137.2.0", vs[0]);
    EXPECT_EQ("127.0.0.8", vs[1]);

    //double
    EXPECT_TRUE(conf.GetFloatArray("PI_LIST", vd));
    EXPECT_EQ(2U, vd.size());
    EXPECT_TRUE(vd[0]-3.1 < 0.0001);

    //default
    int def = 111;
    EXPECT_FALSE(conf.GetIntVal("DEFAULT", interval));
    EXPECT_EQ(0, interval);
    EXPECT_FALSE(conf.GetIntVal("DEFAULT", interval, def));
    EXPECT_EQ(111, interval);

    
}
