#include <string.h>
#include <sys/select.h>
#include <stdio.h>
#include <errno.h>
//linux only
#include <sys/epoll.h>
#include "poller.h"

#include "comm/common.h"
#include "net_common.h"

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
    int oldmask = data_->oldmask[fd];

    ee.events = 0;
    //merge old
    data_->oldmask[fd] |= mask;
    mask = data_->oldmask[fd];
    //no need to invoke epoll_ctl
    if (oldmask == mask) return 0;
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


} //namespace net
} //namespace yanetlib
