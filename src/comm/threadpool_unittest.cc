#include "gtest/gtest.h"
#include "thr_pool.h"

using namespace yanetlib::comm;
using namespace std;

class ThrPoolUnitTest : public ::testing::Test {
 protected:
     ThrPoolUnitTest() {
         sz_ = 4;
     }
     ~ThrPoolUnitTest() {}

     ThrPool *thp;
     int sz_;
};

void* my_test_fun(void* arg) {
    int val = (int)arg;
    printf("\nthis is my_test_fun:%d\n", val);
    return NULL;
}

class A {
 public:
     A() {}
     ~A() {}
     void a_fun(int val) {
         printf("\nthis is A::a_fun:%d\n", val);
     }
};

TEST_F(ThrPoolUnitTest, BasicTest) {
    thp = new ThrPool(sz_);
    A* a = new A;
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(thp->AddJob(&my_test_fun, (void*)i));
        EXPECT_TRUE(thp->AddJob(a, &A::a_fun, i));
    }
    sleep(1);
}
