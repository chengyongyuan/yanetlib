#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include "net_common.h"
#include "poller.h"
#include "eventloop.h"

namespace yanetlib {
namespace net {

using yanetlib::comm::Closure;
using namespace std;

//--------------------------------------------------------------------------------
//Signal event related Implementation
//
//--------------------------------------------------------------------------------
//currently can register at most 64 signals
//seems ok..

//static signal fd
static int signal_event_fd = -1;

typedef void (*YanetSigHandler) ( int signo );

//signal event processing's default signal handler
//simply write the signo to the notifier, so that
//the eventloop can receive it, can route it to the
//signo, calling user callback.
static void YanetSigEventHandler(int signo) {
    int fd = signal_event_fd;
    unsigned char sigmsg = signo;

    int save_errno = errno;
    send(fd, &sigmsg, sizeof(sigmsg), 0);
    errno = save_errno;
    
    return ;
}

struct SignalManager::Internal {
    //user registered callback, user are responsible to
    //release the resource.
    CallBack*       callbacks[MAX_SIGNAL_EVENT];
    //old signal handler, use to resotre old signal
    struct sigaction old_handlers[MAX_SIGNAL_EVENT];
    //unix socket pair for event notifying.
    int             sock_pair[2];
};

SignalManager::SignalManager() : data_(NULL) {
}

SignalManager::~SignalManager() {
    close(data_->sock_pair[0]);
    close(data_->sock_pair[1]);
    for (int i = 0; i < MAX_SIGNAL_EVENT; ++i) {
        if (data_->callbacks[i]) delete data_->callbacks[i];
        data_->callbacks[i] = NULL;
    }

    delete data_;
}

int SignalManager::GetFd() const {
    return data_->sock_pair[0];
}

int SignalManager::InitSignalEvent() {
    data_ = new Internal;
    if (data_ == NULL) return -1;
    memset(data_, 0 , sizeof(Internal));

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, data_->sock_pair) == -1) {
        return -3;
    }
    if (YanetNonBlock(NULL, data_->sock_pair[0]) != YANET_OK) {
        return -5;
    }
    if (YanetNonBlock(NULL, data_->sock_pair[1]) != YANET_OK) {
        return -7;
    }
    signal_event_fd =  data_->sock_pair[1];

    return 0;
}

int SignalManager::AddSignalHandler(int signo, CallBack* cb) {

    //added already
    if (data_->callbacks[signo]) {
        CallBack* old = data_->callbacks[signo];
        data_->callbacks[signo] = cb;
        delete old;
        //old sig handler function don't need change.
    } else {
        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_handler = YanetSigEventHandler;
        act.sa_flags |= SA_RESTART;
        sigfillset(&act.sa_mask);
        if (sigaction(signo, &act, &data_->old_handlers[signo]) == -1) {
            return -1;
        }
        data_->callbacks[signo] = cb;
    }
    return 0;
}

int SignalManager::DelSignalHandler(int signo) {
    if (data_->callbacks[signo]) {
        //restore old handler
        if (data_->callbacks[signo]) {
            if (sigaction(signo, &data_->old_handlers[signo], NULL) == -1) {
                return -1;
            }
            delete data_->callbacks[signo];
            data_->callbacks[signo] = NULL;
        }
    }
    return 0;
}

void SignalManager::HandleWrite(EventLoop* ev, int fd) {
}

void SignalManager::HandleRead(EventLoop* ev, int fd) {
    unsigned char signo;
    unsigned char sigbuf[1024];

    while(1) {
        int n = recv(fd, &sigbuf, sizeof(sigbuf), 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) return;
            return ;
        } else if (n == 0) {
            //== 0 ?
        }
        for (int i = 0; i < n; ++i) {
            signo = sigbuf[i];
            if (signo <= MAX_SIGNAL_EVENT) {
                if (data_->callbacks[signo])
                    data_->callbacks[signo]->Run();
            }
        }
    }
}


//--------------------------------------------------------------------------------
//Event Loop Implementation
//
//--------------------------------------------------------------------------------
EventLoop::EventLoop() :
    poller_(NULL), timer_manager_(NULL), signal_manager_(NULL), stop_(0) { }

EventLoop::~EventLoop() {
    //delete can handle NULL
    delete poller_;
}

int EventLoop::InitEventLoop() {
    if ((poller_ = new Epoller) == NULL) return -1;
    if ((timer_manager_ = new TimerManager) == NULL) return -3;
    if ((signal_manager_ = new SignalManager) == NULL) return -5;
    if (signal_manager_->InitSignalEvent() < 0) return -7;
    if (poller_->InitPoller() != 0) return -9;
    if (AddEvent(signal_manager_->GetFd(), YANET_READABLE,
                 signal_manager_) < 0) return -8;
    return 0;
}

int EventLoop::AddEvent(int fd, int mask, EventCallBack* evt) {
    if (fd > YANET_MAX_POLL_SIZE) return -1;

    events_[fd] = evt;
    events_set_.insert(EventID(fd, evt));
    if (poller_->AddEvent(fd, mask) != 0)
        return -3;
    
    return 0;
}

void EventLoop::DelEvent(int fd, int mask) {
    if (fd > YANET_MAX_POLL_SIZE) return ;
    EventCallBack* e = events_[fd];

    if (e == NULL) return ;
    if (poller_->DelEvent(fd, mask)) {
        events_set_.erase(EventID(fd, e));
        delete e;
        events_[fd] = NULL;
    }
}

//take from REDIS
/* Process every pending time event, then every pending file event
 * (that may be registered by time event callbacks just processed).
 * Without special flags the function sleeps until some file event
 * fires, or when the next time event occurrs (if any).
 *
 * If flags is 0, the function does nothing and returns.
 * if flags has YANET_ALL_EVENTS set, all the kind of events are processed.
 * if flags has YANET_FILE_EVENTS set, file events are processed.
 * if flags has YANET_TIME_EVENTS set, time events are processed.
 * if flags has YANET_DONT_WAIT set the function returns ASAP until all
 * the events that's possible to process without to wait are processed.
 *
 * The function returns the number of events processed. */
int EventLoop::ProcessEvent(int flags, int wait_ms) {
    int processed = 0;

    //Nothing to do, just return ASAP
    if (!(flags & YANET_TIME_EVENTS) && !(flags & YANET_FILE_EVENTS)) return 0;

    if (/*maxfd_ != -1 ||*/
       ((flags & YANET_TIME_EVENTS) && !(flags & YANET_DONT_WAIT))) {
        TimerListObject* shortest = NULL;
        struct timeval tv, *tvp = NULL;
        if ((flags & YANET_TIME_EVENTS) && !(flags & YANET_DONT_WAIT)) 
            shortest = timer_manager_->SearchNearestTimer();
        if (shortest) {
            long nowsec, nowms;

            //calculate the time missing for nearest timer for fire
            GetTime(&nowsec, &nowms);
            tvp = &tv;
            tvp->tv_sec = shortest->when_sec - nowsec;
            if (shortest->when_ms < nowms) {
                tvp->tv_usec = ((shortest->when_ms+1000 - nowms) * 1000);
                tvp->tv_sec --;
            } else {
                tvp->tv_usec = (shortest->when_ms - nowms) * 1000;
            }
            if (tvp->tv_sec < 0) tvp->tv_sec = 0;
            if (tvp->tv_usec < 0) tvp->tv_usec = 0;
        } else {
            //if we have to check events, but we set the DONT_WAIT flags
            //we need set tv to zero, so that we can return ASAP.
            if (flags & YANET_DONT_WAIT) {
                tv.tv_sec = tv.tv_usec = 0;
                tvp = &tv;
            } else {
                //otherwise we can just block
                if (wait_ms == -1) 
                    tvp = NULL;
                else {
                    if (wait_ms < MIN_EVENT_WAIT_TIME)
                        wait_ms = MIN_EVENT_WAIT_TIME;
                    tv.tv_sec = wait_ms/1000;
                    tv.tv_usec = (wait_ms%1000)*1000;
                    tvp = &tv;
                }
            }
        }

        vector<int> readable, writeable;
        poller_->PollEvent(tvp, &readable, &writeable);
        for (size_t i = 0; i < readable.size(); ++i) {
            int fd = readable[i];
            EventCallBack* e = events_[fd];
            if (e != NULL) e->HandleRead(this, fd);
            processed++;
        }
        for (size_t i = 0; i < writeable.size(); ++i) {
            int fd = writeable[i];
            EventCallBack* e = events_[fd];
            if (e != NULL) e->HandleWrite(this, fd);
            processed++;
        }

    }
    //check timer event
    if (flags & YANET_TIME_EVENTS) 
        processed += ProcessTimerEvent();
    return processed;
}

void EventLoop::HandleLoop() {
    for (set<EventID>::iterator it  = events_set_.begin();
            it != events_set_.end();
            ) {
        //event->HandleLoop may invalidate set iterator
        int fd = it->efd;
        EventCallBack* e = it->e;
        assert(e != NULL);
        ++it;
        e->HandleLoop(this, fd);
    }
}

void EventLoop::Run(int wait_ms) {
    stop_ = 0;
    while (!stop_) {
        if (beforesleep_ != NULL)
            beforesleep_(this);
        HandleLoop();
        ProcessEvent(YANET_ALL_EVENTS, wait_ms);
    }
}
} //namespace net
} //namespace yanetlib
