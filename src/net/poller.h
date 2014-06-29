#ifndef YANETLIB_NET_POLLER_H
#define YANETLIB_NET_POLLER_H

//This file implment a sys poller. currently: select
//and epoll is implemented. epoll is default for linux
//other system we just use select now (I don't think
//we will use other unix like system in short future.
//
//colincheng 2014/06/29
//

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string>

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

#define YANET_NOMORE        -1

class EventLoop;

//event callback interface
class EventCallBack {
 public:
     //Constructor
     EventCallBack() : mask_(YANET_NONE) { }

     virtual void HandleRead(EventLoop* ev, int fd, int mask)  = 0;
     virtual void HandleWrite(EventLoop* ev, int fd, int mask) = 0;
     virtual ~EventCallBack() {}

     //not for encapulation, for easy to use.
     int& Mask() { return mask_; }
 protected:
     int mask_;
};

//timer callker interface
class TimerCallBack {
 public:
     //non-trival constructor
     TimerCallBack(long long id, long sec, long ms) :
         id_(id), when_sec_(sec), when_ms_(ms), next_(NULL) { }
     
     virtual ~TimerCallBack() {}

     //timer interface
     virtual int HandleTimer(EventLoop* ev, long long id) = 0;

 protected:
     friend class EventLoop;

     long long      id_;
     long           when_sec_;
     long           when_ms_;
     TimerCallBack* next_;
};

class Poller {
 public:
     Poller(EventLoop *ev) : ev_(ev) { };
     
     virtual ~Poller() {};

     //Init Poller
     //RETURN: 0:ok,  -1:erro
     virtual int InitPoller() = 0;

     //Add a event to the poller
     //RETURN: 0:ok,  -1:erro
     virtual int AddEvent(int fd, int mask) = 0;

     //Delete a event from the poller
     //RETURN: 0:ok,  -1:erro
     virtual int DelEvent(int fd, int mask) = 0;

     //Poll for avaiable events.
     //RETURN: >0: num of events. -1:error
     virtual int PollEvent(struct timeval* tvp) = 0;

     //Get poller name
     virtual std::string GetName() const;

 protected:
     EventLoop* const ev_;
};

class Selecter : public Poller {
 public:

     //poller private data
     struct InternalData;

     //Constructor
     Selecter(EventLoop *ev);

     ~Selecter();

     int InitPoller();

     int AddEvent(int fd, int mask);

     int DelEvent(int fd, int mask);

     int PollEvent(struct timeval* tvp);

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

     Epoller(EventLoop *ev);

     ~Epoller();

     int InitPoller();

     int AddEvent(int fd, int mask);

     int DelEvent(int fd, int delmask);

     int PollEvent(struct timeval* tvp);

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

     struct FiredEvent {
         int mask;
         int fd;
     };

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
     //RETURN: 0:OK <0:fail
     void DelEvent(int fd, int mask);

     //add a timer event to eventloop
     //milliseconds 毫秒
     //RETURN: 0:OK <0:fail
     int AddTimer(TimerCallBack* te/*long long milliseconds*/);

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
     TimerCallBack* SearchNearestTimer() const;
     void GetTime(long* nowsec, long* nowms) const ;
     void AddMilliSecondsToNow(long long miliseconds, 
                               long* sec, long* ms) const;
 private:
     //disable copy
     EventLoop(const EventLoop&);
     void operator=(const EventLoop&);

     //c++ friendship does not inherit.
     friend class Selecter;
     //if def linux
     friend class Epoller;

     //callback function before entering block waitting
     BeforeSleepProc*  beforesleep_;

     //poller object
     Poller*           poller_;
     //keep track max pollerable fd 
     int               maxfd_;

     //keep trace max timer fd generated
     long long         next_timer_id_;

     //TODO: change it to variable length events
     //currenly everything is ok.
     EventCallBack*    events_[YANET_MAX_POLL_SIZE];

     //keep track active events.
     FiredEvent        fired_ [YANET_MAX_POLL_SIZE];

     //timer list head
     //TODO: we can implement timer more efficently,
     //but we want to make it work first:-
     TimerCallBack*    timer_evt_head_;

     //stop the event loop
     int           stop_;
};

} //namespace net
} //namespace yanetlib

#endif //poller.h
