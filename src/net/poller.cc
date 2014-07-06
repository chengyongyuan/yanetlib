#include <string.h>
#include <sys/select.h>
//linux only
#include <sys/epoll.h>
#include "poller.h"

namespace yanetlib {
namespace net {

using std::vector;

struct Selecter::InternalData {
    fd_set rset, wset;
    //select's readset writeset can not reuse
    //during different oop
    fd_set _rset, _wset;
    int maxfd;

    InternalData() : maxfd(-1) { }
};

Selecter::Selecter() 
    : data_(NULL){ }

Selecter::~Selecter() {
    delete data_;
}

int Selecter::InitPoller() {
    data_ = new InternalData;
    if (!data_) return -1;
    FD_ZERO(&data_->rset);
    FD_ZERO(&data_->wset);
    return 0;
}

int Selecter::AddEvent(int fd, int mask) {
    if ((mask & (YANET_READABLE | YANET_WRITABLE)) && 
        fd > data_->maxfd) {
        data_->maxfd = fd;
    }
    if (mask & YANET_READABLE) FD_SET(fd, &data_->rset);
    if (mask & YANET_WRITABLE) FD_SET(fd, &data_->wset);
    return 0;
}

bool Selecter::DelEvent(int fd, int mask) {
    if (mask & YANET_READABLE) FD_CLR(fd, &data_->rset);
    if (mask & YANET_WRITABLE) FD_CLR(fd, &data_->wset);
    if (!FD_ISSET(fd, &data_->rset) && !FD_ISSET(fd, &data_->wset)) {
        if (fd == data_->maxfd) {
            int i;
            for (i = data_->maxfd-1; i >= 0; --i) {
                if (FD_ISSET(i, &data_->rset) || FD_ISSET(i, &data_->wset))
                    break;
            }
            data_->maxfd = i;
        }
    }
    return (!FD_ISSET(fd, &data_->rset) && !FD_ISSET(fd, &data_->wset));
}

void Selecter::PollEvent(struct timeval* tvp,
                        vector<int>* readable,
                        vector<int>* writeable) {
    int retval;

    memcpy(&data_->_rset, &data_->rset, sizeof(fd_set));
    memcpy(&data_->_wset, &data_->wset, sizeof(fd_set));

    retval = select(data_->maxfd+1, &data_->_rset, &data_->_wset,
                    NULL, tvp);
    if (retval > 0) {
        for (int j = 0; j <= data_->maxfd; ++j) {
            if (FD_ISSET(j, &data_->_rset))
                readable->push_back(j);
            if (FD_ISSET(j, &data_->_wset))
                writeable->push_back(j);
        }
    }
}

//#ifdef linux
struct Epoller::InternalData {
    int    efd;
    struct epoll_event events[YANET_MAX_POLL_SIZE];
    //keep old POLL EVENT
    int    oldmask[YANET_MAX_POLL_SIZE];

    InternalData() : efd(-1) {
    }
};

Epoller::Epoller() 
    : data_(NULL){ }

Epoller::~Epoller() {
    if (data_->efd != -1) close(data_->efd);
    delete data_;
}

int Epoller::InitPoller() {
    data_ = new InternalData;
    if (!data_) return -1;
    //1024 is just a hint for kernel
    data_->efd = epoll_create(1024);
    if (data_->efd == -1) return -3;
    return 0;
}

int Epoller::AddEvent(int fd, int mask) {
    struct epoll_event ee;
    //if the fd already add to poller, we need MOD, or
    //we need ADD
    int op = data_->oldmask[fd] == YANET_NONE ? EPOLL_CTL_ADD :
             EPOLL_CTL_MOD;
    ee.events = 0;
    //merge old
    data_->oldmask[fd] |= mask;
    mask = data_->oldmask[fd];
    if (mask & YANET_READABLE) ee.events |= EPOLLIN;
    if (mask & YANET_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0;
    ee.data.fd = fd;
    if (epoll_ctl(data_->efd, op, fd, &ee) == -1) return -1;
    return 0;
}

bool Epoller::DelEvent(int fd, int delmask) {
    struct epoll_event ee;
    data_->oldmask[fd] = data_->oldmask[fd] & (~delmask);
    int mask = data_->oldmask[fd] & (~delmask);

    ee.events = 0;
    if (mask & YANET_READABLE) ee.events |= EPOLLIN;
    if (mask & YANET_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0;
    ee.data.fd = fd;
    if (mask != YANET_NONE) {
        epoll_ctl(data_->efd, EPOLL_CTL_MOD, fd, &ee);
    } else {
        epoll_ctl(data_->efd, EPOLL_CTL_DEL, fd, &ee);
    }
    return (mask == YANET_NONE);
}

void Epoller::PollEvent(struct timeval* tvp,
                       vector<int>* readable,
                       vector<int>* writeable) {
    int retval;

    retval = epoll_wait(data_->efd, data_->events, YANET_MAX_POLL_SIZE,
                        tvp ? (tvp->tv_sec*1000 + tvp->tv_usec/1000) : -1);
    if (retval > 0) {
        for (int j = 0; j < retval; ++j) {
            struct epoll_event* e = data_->events + j;

            if (e->events & EPOLLIN)  readable->push_back(e->data.fd);
            if (e->events & EPOLLOUT) writeable->push_back(e->data.fd);
        }
    }
}

//--------------------------------------------------------------------------------
//Event Loop Implementation
//
//--------------------------------------------------------------------------------
EventLoop::EventLoop() :
    poller_(NULL), timer_manager_(NULL), stop_(0) { }

EventLoop::~EventLoop() {
    //delete can handle NULL
    delete poller_;
}

int EventLoop::InitEventLoop() {
    if ((poller_ = new Epoller) == NULL) return -1;
    if ((timer_manager_ = new TimerManager) == NULL) return -3;
    if (poller_->InitPoller() != 0) return -3;
    return 0;
}

int EventLoop::AddEvent(int fd, int mask, EventCallBack* evt) {
    if (fd > YANET_MAX_POLL_SIZE) return -1;

    events_[fd] = evt;
    if (poller_->AddEvent(fd, mask) != 0)
        return -3;
    
    return 0;
}

void EventLoop::DelEvent(int fd, int mask) {
    if (fd > YANET_MAX_POLL_SIZE) return ;
    EventCallBack* e = events_[fd];

    if (e == NULL) return ;
    if (poller_->DelEvent(fd, mask)) {
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
int EventLoop::ProcessEvent(int flags) {
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
                tvp = NULL;
            }
        }

        vector<int> readable, writeable;
        poller_->PollEvent(tvp, &readable, &writeable);
        for (size_t i = 0; i < readable.size(); ++i) {
            int fd = readable[i];
            EventCallBack* e = events_[fd];
            if (e != NULL) e->HandleRead(fd);
            processed++;
        }
        for (size_t i = 0; i < writeable.size(); ++i) {
            int fd = writeable[i];
            EventCallBack* e = events_[fd];
            if (e != NULL) e->HandleWrite(fd);
            processed++;
        }

    }
    //check timer event
    if (flags & YANET_TIME_EVENTS) 
        processed += ProcessTimerEvent();
    return processed;
}

void EventLoop::Run() {
    stop_ = 0;
    while (!stop_) {
        if (beforesleep_ != NULL)
            beforesleep_(this);
        ProcessEvent(YANET_ALL_EVENTS);
    }
}

} //namespace net
} //namespace yanetlib
