#include <string.h>
#include <sys/select.h>
//linux only
#include <sys/epoll.h>
#include "poller.h"

namespace yanetlib {
namespace net {

struct Selecter::InternalData {
    fd_set rset, wset;
    //select's readset writeset can not reuse
    //during different oop
    fd_set _rset, _wset;
};

Selecter::Selecter(EventLoop *ev) :
    Poller(ev), data_(new InternalData){ }

Selecter::~Selecter() {
    delete data_;
}

int Selecter::InitPoller() {
    if (!data_) return -1;
    FD_ZERO(&data_->rset);
    FD_ZERO(&data_->wset);
    return 0;
}

int Selecter::AddEvent(int fd, int mask) {
    if (mask & YANET_READABLE) FD_SET(fd, &data_->rset);
    if (mask & YANET_WRITABLE) FD_SET(fd, &data_->wset);
    return 0;
}

int Selecter::DelEvent(int fd, int mask) {
    if (mask & YANET_READABLE) FD_CLR(fd, &data_->rset);
    if (mask & YANET_WRITABLE) FD_CLR(fd, &data_->wset);
    return 0;
}

int Selecter::PollEvent(struct timeval* tvp) {
    int retval, nevents = 0;

    memcpy(&data_->_rset, &data_->rset, sizeof(fd_set));
    memcpy(&data_->_wset, &data_->wset, sizeof(fd_set));

    retval = select(ev_->maxfd_+1, &data_->_rset, &data_->_wset,
                    NULL, tvp);
    if (retval > 0) {
        for (int j = 0; j <= ev_->maxfd_; ++j) {
            int mask = 0;
            EventCallBack *fe = ev_->events_[j];

            if (fe->Mask() == YANET_NONE) continue;
            if (fe->Mask() & YANET_READABLE && FD_ISSET(j, &data_->_rset))
                mask |= YANET_READABLE;
            if (fe->Mask() & YANET_WRITABLE && FD_ISSET(j, &data_->_wset))
                mask |= YANET_WRITABLE;
            ev_->fired_[nevents].fd = j;
            ev_->fired_[nevents].mask = mask;
            nevents++;
        }
    }
    return nevents;
}

//#ifdef linux
struct Epoller::InternalData {
    int efd;
    struct epoll_event events[YANET_MAX_POLL_SIZE];

    InternalData() : efd(-1) {
    }
};

Epoller::Epoller(EventLoop *ev) :
    Poller(ev), data_(new InternalData){ }

Epoller::~Epoller() {
    if (data_->efd != -1) close(data_->efd);
    delete data_;
}

int Epoller::InitPoller() {
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
    int op = ev_->events_[fd]->Mask() == YANET_NONE ? EPOLL_CTL_ADD :
             EPOLL_CTL_MOD;
    ee.events = 0;
    //merge old
    mask |= ev_->events_[fd]->Mask();
    if (mask & YANET_READABLE) ee.events |= EPOLLIN;
    if (mask & YANET_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0;
    ee.data.fd = fd;
    if (epoll_ctl(data_->efd, op, fd, &ee) == -1) return -1;
    return 0;
}

int Epoller::DelEvent(int fd, int delmask) {
    struct epoll_event ee;
    int mask = ev_->events_[fd]->Mask() & (~delmask);

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
    return 0;
}

int Epoller::PollEvent(struct timeval* tvp) {
    int retval, nevents = 0;

    retval = epoll_wait(data_->efd, data_->events, YANET_MAX_POLL_SIZE,
                        tvp ? (tvp->tv_sec*1000 + tvp->tv_usec/1000) : -1);
    if (retval > 0) {
        nevents = retval;
        for (int j = 0; j < nevents; ++j) {
            int mask = 0;
            struct epoll_event* e = data_->events + j;

            if (e->events & EPOLLIN)  mask |= YANET_READABLE;
            if (e->events & EPOLLOUT) mask |= YANET_WRITABLE;
            ev_->fired_[j].fd = e->data.fd;
            ev_->fired_[j].mask = mask;
        }
    }
    return nevents;
}

//--------------------------------------------------------------------------------
//Event Loop Implementation
//
//--------------------------------------------------------------------------------
EventLoop::EventLoop() : poller_(NULL), maxfd_(-1), next_timer_id_(0), 
                         timer_evt_head_(NULL), stop_(0) { }

EventLoop::~EventLoop() {
    //delete can handle NULL
    delete poller_;
}

int EventLoop::InitEventLoop() {
    poller_ = new Epoller(this);
    if (poller_ == NULL) return -1;
    if (poller_->InitPoller() != 0) return -3;
    return 0;
}

int EventLoop::AddEvent(int fd, int mask, EventCallBack* evt) {
    if (fd > YANET_MAX_POLL_SIZE) return -1;

    events_[fd] = evt;
    if (poller_->AddEvent(fd, mask) != 0)
        return -3;
    evt->Mask() |= mask;
    if (fd > maxfd_) maxfd_ = fd;
    
    return 0;
}

void EventLoop::DelEvent(int fd, int mask) {
    if (fd > YANET_MAX_POLL_SIZE) return ;
    EventCallBack* e = events_[fd];

    if (e->Mask() == YANET_NONE) return ;
    e->Mask() &= (~mask);
    if (fd == maxfd_ && e->Mask() == YANET_NONE) {
        int i;
        for (i = maxfd_-1; i >= 0; --i) {
            if (events_[i]->Mask() != YANET_NONE) break;
        }
        maxfd_ = i;
    }
    poller_->DelEvent(fd, mask);
}

void EventLoop::GetTime(long* nowsec, long* nowms) const {
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    *nowsec = tv.tv_sec;
    *nowms = tv.tv_usec/1000;
}

void EventLoop::AddMilliSecondsToNow(long long miliseconds, 
                                     long* sec, long* ms) const {
    long cur_sec, cur_ms, when_sec, when_ms;

    GetTime(&cur_sec, &cur_ms);
    when_sec = cur_sec + miliseconds/1000;
    when_ms = cur_ms + miliseconds%1000;
    if (when_ms > 1000) {
        when_sec ++;
        when_ms -= 10000;
    }
    *sec = when_sec;
    *ms = when_ms;
    
}

int EventLoop::AddTimer(TimerCallBack* te/*long long milliseconds*/) {
    long long id = next_timer_id_++;
    //long now_sec, now_ms;

    //AddMilliSecondsToNow(milliseconds, &now_sec, &now_ms);
    //TimerCallBack* te = new TimerCallBack(id, now_sec, now_ms); 
    if (te == NULL) return -1;
    te->next_ = timer_evt_head_;
    timer_evt_head_ = te;

    return id;
}

int EventLoop::DelTimer(long long id) {
    TimerCallBack* te, *prev = NULL;

    te = timer_evt_head_;
    while (te) {
        if (te->id_ == id) {
            if (prev == NULL)
                timer_evt_head_ = te->next_;
            else
                prev->next_ = te->next_;
            //TODO: TimeDead Callback
            //te->TimerEnd()
            delete te;
            return 0;
        }
        prev = te;
        te = te->next_;
    }
    //Not found the timer event with id
    return -1;
}

//Steal from REDIS
/* Search the first timer to fire.
 * This operation is useful to know how many time the select can be
 * put in sleep without to delay any event.
 * If there are no timers NULL is returned.
 *
 * Note that's O(N) since time events are unsorted.
 * Possible optimizations (not needed by Redis so far, but...):
 * 1) Insert the event in order, so that the nearest is just the head.
 *    Much better but still insertion or deletion of timers is O(N).
 * 2) Use a skiplist to have this operation as O(1) and insertion as O(log(N)).
 */
TimerCallBack* EventLoop::SearchNearestTimer() const {
    TimerCallBack* te = timer_evt_head_;
    TimerCallBack* nearest = NULL;

    while(te) {
        if (!nearest || te->when_sec_ < nearest->when_sec_ ||
                (te->when_sec_ == nearest->when_sec_ &&
                 te->when_ms_ < nearest->when_ms_))
            nearest = te;
        te = te->next_;
    }
    return nearest;
}

int EventLoop::ProcessTimerEvent() {
    int processed = 0;
    TimerCallBack* te;
    long long maxid;

    te = timer_evt_head_;
    maxid = next_timer_id_-1;
    while (te) {
        long nowsec, nowms;
        long long id;
        if (te->id_ > maxid) {
            te = te->next_;
            continue;
        }
        GetTime(&nowsec, &nowms);
        //need process this timer
        if (nowsec > te->when_sec_ || 
            (nowsec == te->when_sec_ && nowms > te->when_ms_)) {
            int retval;

            id = te->id_;
            retval = te->HandleTimer(this, id);
            processed++;
            //stealed from REDIS
            /* After an event is processed our time event list may
             * no longer be the same, so we restart from head.
             * Still we make sure to don't process events registered
             * by event handlers itself in order to don't loop forever.
             * To do so we saved the max ID we want to handle.
             *
             * FUTURE OPTIMIZATIONS:
             * Note that this is NOT great algorithmically. Redis uses
             * a single time event so it's not a problem but the right
             * way to do this is to add the new elements on head, and
             * to flag deleted elements in a special way for later
             * deletion (putting references to the nodes to delete into
             * another linked list). */
            if (retval != YANET_NOMORE) {
                AddMilliSecondsToNow(retval, &te->when_sec_, &te->when_ms_);
            } else {
                DelTimer(id);
            }
            te = timer_evt_head_;
        } else {
            te = te->next_;
        }
    }
    return processed;
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
    int processed = 0, nevents;

    //Nothing to do, just return ASAP
    if (!(flags & YANET_TIME_EVENTS) && !(flags & YANET_FILE_EVENTS)) return 0;

    if (maxfd_ != -1 ||
       ((flags & YANET_TIME_EVENTS) && !(flags & YANET_DONT_WAIT))) {
        TimerCallBack* shortest = NULL;
        struct timeval tv, *tvp = NULL;
        if ((flags & YANET_TIME_EVENTS) && !(flags & YANET_DONT_WAIT)) 
            shortest = SearchNearestTimer();
        if (shortest) {
            long nowsec, nowms;

            //calculate the time missing for nearest timer for fire
            GetTime(&nowsec, &nowms);
            tvp = &tv;
            tvp->tv_sec = shortest->when_sec_ - nowsec;
            if (shortest->when_ms_ < nowms) {
                tvp->tv_usec = ((shortest->when_ms_+1000 - nowms) * 1000);
                tvp->tv_sec --;
            } else {
                tvp->tv_usec = (shortest->when_ms_ - nowms) * 1000;
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

        nevents = poller_->PollEvent(tvp);
        for (int i = 0; i < nevents; ++i) {
            EventCallBack* e = events_[fired_[i].fd];
            int mask = fired_[i].mask;
            int fd = fired_[i].fd;

            //e->Mask() & mask & YANET_DONT_WAIT code ...here means,
            //a already proceesed event remove  an element that fired
            //and we still dont' processed, so we check if the event
            //still valid
            if (e->Mask() & mask & YANET_READABLE) {
                e->HandleRead(this, fd, mask);
            }
            if (e->Mask() & mask & YANET_WRITABLE) {
                e->HandleWrite(this, fd, mask);
            }
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
