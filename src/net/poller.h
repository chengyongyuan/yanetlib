#ifndef YANETLIB_NET_POLLER_H
#define YANETLIB_NET_POLLER_H

//This file implment a sys poller. currently: select
//and epoll is implemented. epoll is default for linux
//other system we just use select now (I don't think
//we will use other unix like system in short future.
//
//colincheng 2014/06/29
//
//TODO
//1. Refactor C version REDIS, decouple dependence  (DONE)
//2. remove hardcode MAX_POLL_SIZE

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include "timer.h"

namespace yanetlib {
namespace net {

#define YANET_MAX_POLL_SIZE 4*1024
#define YANET_NONE          0
#define YANET_READABLE      1
#define YANET_WRITABLE      2

#define YANET_FILE_EVENTS    1
#define YANET_TIME_EVENTS    2
#define YANET_ALL_EVENTS     3
#define YANET_DONT_WAIT      4 

//event callback interface
class EventCallBack {
 public:
     //Constructor
     EventCallBack() { }

     //If We have multiple EventLoop object in one process, it may
     //help to put eventloop pointer to callbacks
     virtual void HandleRead (/*EventLoop* ev */int fd)  = 0;
     virtual void HandleWrite(/*EventLoop* ev */int fd) = 0;
     virtual ~EventCallBack() {}

};

class Poller {
 public:
     //Init Poller
     //RETURN: 0:ok,  -1:erro
     virtual int InitPoller() = 0;

     //Add a event to the poller
     //RETURN: 0:ok,  -1:erro
     virtual int AddEvent(int fd, int mask) = 0;

     //Delete a event from the poller
     //RETURN: true: Del All Event(read&write), false: still have
     //some event to poll
     virtual bool DelEvent(int fd, int mask) = 0;

     //Poll for avaiable events.
     //put readable event in 'readable vec.', writeable
     //event in 'writeable vec.'
     virtual void PollEvent(struct timeval* tvp,
                           std::vector<int>* readable,
                           std::vector<int>* writeable) = 0;

     //Get poller name
     virtual std::string GetName() const = 0;

     virtual ~Poller() { }

};

class Selecter : public Poller {
 public:

     //poller private data
     struct InternalData;

     //Constructor
     Selecter();

     ~Selecter();

     int  InitPoller();

     int  AddEvent(int fd, int mask);

     bool  DelEvent(int fd, int mask);

     void PollEvent(struct timeval* tvp,
                   std::vector<int>* readable,
                   std::vector<int>* writeable);

     std::string GetName() const {
         return "Selecter";
     }

 private:
     //diable copy
     Selecter(const Selecter&);
     void operator=(const Selecter&);

    InternalData* data_;
};

class Epoller : public Poller {
 public:
     //Poller private data
     struct InternalData;

     Epoller();

     ~Epoller();

     int  InitPoller();

     int  AddEvent(int fd, int mask);

     bool  DelEvent(int fd, int delmask);

     void PollEvent(struct timeval* tvp,
                   std::vector<int>* readable,
                   std::vector<int>* writeable);

     std::string GetName() const {
         return "Epoller";
     }

 private:
     //disable copy
     Epoller(const Epoller&);
     void operator=(const Epoller&);

     InternalData* data_;
};

class EventLoop {
 public:
     typedef void BeforeSleepProc(EventLoop*);

     EventLoop();
     
     ~EventLoop();

     //inteface
     //RETURN: 0:OK  <0:fail
     int InitEventLoop();

     //set the stop flag of eventloop
     void StopEventLoop() { stop_ = 1; }

     //add a normal event to eventloop
     //RETURN: 0:OK <0:fail
     int AddEvent(int fd, int mask, EventCallBack* evt);

     //del a normal event to eventloop
     //caller are responsible for delete evt if neccessary.
     void DelEvent(int fd, int mask);

     //add a timer event to eventloop
     //milliseconds 毫秒
     //RETURN: 0:OK <0:fail
     int AddTimer(long long milliseconds, TimerCallBack* te);

     //del a timer event to eventloop
     //RETURN: 0:OK -1:fail
     int DelTimer(long long id);

     //process timer event.
     //RETURN: >=0: number of events processed
     int ProcessTimerEvent();

     //process normal event.
     //RETURN: >=0: number of events processed
     int ProcessEvent(int flags);

     void SetBeforeSleepProc(BeforeSleepProc* beforesleep) { 
         beforesleep_ = beforesleep;
     }

     //Run event loop
     void Run();

     std::string GetPollerName() const {
         return poller_->GetName();
     }

 private:
     //disable copy
     EventLoop(const EventLoop&);
     void operator=(const EventLoop&);

     //callback function before entering block waitting
     BeforeSleepProc*  beforesleep_;

     //poller object
     Poller*           poller_;

     //TODO: change it to variable length events
     //currenly everything is ok.
     EventCallBack*    events_[YANET_MAX_POLL_SIZE];

     TimerManager*    timer_manager_;

     //stop the event loop
     int           stop_;
};

inline int EventLoop::AddTimer(long long milliseconds, TimerCallBack* te) {
    return timer_manager_->AddTimer(milliseconds, te);
}

inline int EventLoop::DelTimer(long long id) {
    return timer_manager_->DelTimer(id);
}

inline int EventLoop::ProcessTimerEvent() {
    return timer_manager_->ProcessTimerEvent();
}


} //namespace net
} //namespace yanetlib

#endif //poller.h
