#include <assert.h>
#include "net_common.h"
#include "poller.h"
#include "gtest/gtest.h"

using namespace std;
using namespace yanetlib::net;

class MyTimer : public TimerCallBack {
    static int data_raw;
    int HandleTimer(long long id);
};

int MyTimer::data_raw = 0;

int MyTimer::HandleTimer(long long id) {
    printf("timer happen!(%d)\n", data_raw++);
    return 1000;
}

int main(int argc, char** argv)
{
    EventLoop *ev_loop = new EventLoop;

    assert(ev_loop->InitEventLoop() == 0);
    ev_loop->AddTimer(500, new MyTimer);

    ev_loop->Run();

    return 0;
}
