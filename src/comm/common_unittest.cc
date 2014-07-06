#include <pthread.h>
#include <string>
#include <vector>
#include "common.h"
#include "gtest/gtest.h"

using namespace std;
using namespace ::yanetlib::comm;

typedef NullMutex MutexLock;

//Unittest for many basic function in common.cc
//Things in common.cc are those most fundamental.
//
class CommonTest : public ::testing::Test {
    protected:
        //for ScopedPtr Test
        class InnerType {
            public:
            InnerType(char c) : c_(c) {
                fprintf(stderr, "Constructor InnerType(%c)\n", c_);
            }
            ~InnerType() {
                fprintf(stderr, "Destructor InnerType(%c)\n", c_);
            }
            char c_;
        };

        //For Mutex 
        static void* ThreadFunMutex(void* arg) {
            static volatile int sa;
            {
                ScopedLock<MutexLock> lock(&mutex_);
                fprintf(stderr, "thread %lu got the lock.sa=%d\n", pthread_self(), sa);
                sa++;
                sleep(5);
            }
            return 0;    
        }

        CommonTest() {
        }

        ~CommonTest() {
        }

        virtual void SetUp() {
        }

        virtual void TearDown() {
        }
        static MutexLock mutex_;
};

MutexLock CommonTest::mutex_;


void ATTRIBUTE_ALWAYS_INLINE test_inline() {
    for (int i = 0; i < 10; ++i) {
        fprintf(stderr, "i=%d\n", i);
    }
}
//test many usefull macros
TEST_F(CommonTest, TestMacro) {
    char a[16];
    int  b[4];
    int aa = 10;
    COMPILE_ASSERT(sizeof(uint32_t)==4, uint32t_size_not_eq_4);
    COMPILE_ASSERT(sizeof(uint64_t)==8, uint64t_size_not_eq_8);
    COMPILE_ASSERT(sizeof(short)==2,    short_sizse_not_eq_2);
    EXPECT_EQ(16U, ARRAY_SIZE(a));
    EXPECT_EQ(4U, ARRAY_SIZE(b));
    if (PREDICT_TRUE(aa > 10)) {
        fprintf(stderr, "aa > 10\n");
    }
    test_inline();
}

//mutex/threading
TEST_F(CommonTest, TestMutex) {
    pthread_t th1, th2, th3;

    pthread_create(&th1, NULL, ThreadFunMutex, &th1);
    pthread_create(&th2, NULL, ThreadFunMutex, &th2);
    pthread_create(&th3, NULL, ThreadFunMutex, &th3);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    pthread_join(th3, NULL);
}

//ScopedPtr
TEST_F(CommonTest, TestScopedPtr) {
    {
        ScopedPtr<InnerType> p1(new InnerType('A'));
        ScopedPtr<InnerType> p2(new InnerType('B'));
    }
    ScopedPtr<InnerType> p3(new InnerType('C'));
    ScopedPtr<InnerType> p4(new InnerType('D'));
}

//Inner Log
TEST_F(CommonTest, TestInnerLog) {
    int a = 10;
    int b = 11;
    COLIN_CHECK_EQ(a, 10);
    COLIN_CHECK_NE(a, b);
    COLIN_CHECK_EQ(b, 11);
    //COLIN_LOG(FATAL) << "this is last line of log";
    COLIN_LOG(INFO) << "this is info log";
    //COLIN_CHECK_GT(a, 11);
    //COLIN_CHECK_LT(a, 1);
    //COLIN_CHECK_EQ(a, b);
}

TEST_F(CommonTest, ColorPrintTest) {
    printf("THIS IS"ANSI_COLOR_GREEN" GREEN \n"ANSI_COLOR_RESET);
    printf("THIS IS"ANSI_COLOR_RED" RED \n"ANSI_COLOR_RESET);
    EXPECT_TRUE(true);
}
