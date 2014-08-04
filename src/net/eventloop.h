#ifndef YANETLIB_NET_EVENTLOOP_H
#define YANETLIB_NET_EVENTLOOP_H

#include "comm/common.h"
#include "timer.h"
#include "poller.h"

//This file implement the main event loop class.
//It mainly handle socketevent, timerevent, 
//signal event right now. NOTE: I put SignalManger
//class definition in this file for There is some
//dependence btw EventLoop and SignalManger.
//Rightnow, I use hardcode MAX_SIGNAL_EVENT to impl
//signal event, for the purpose to minmize new/malloc
//to make code most robost(at the cost of more mem.)
//TODO:
//1. remove hardcode MAX_SIGNAL_EVENT 

namespace yanetlib {
namespace net {

#define YANET_FILE_EVENTS    1
#define YANET_TIME_EVENTS    2
#define YANET_ALL_EVENTS     3
#define YANET_DONT_WAIT      4 

#define MAX_SIGNAL_EVENT     64
#define CallBack yanetlib::comm::Closure

class EventLoop;
//event callback interface
class EventCallBack {
 public:
     //Constructor
     EventCallBack() { }

     //If We have multiple EventLoop object in one process, it may
     //help to put eventloop pointer to callbacks
     virtual void HandleRead (EventLoop* ev, int fd)  = 0;
     virtual void HandleWrite(EventLoop* ev, int fd) = 0;

     virtual ~EventCallBack() {}
};

class SignalManager : public EventCallBack {
 public:
     SignalManager();

     ~SignalManager();

     //interface
     //RETURN: 0:OK, <0: FAIL
     int InitSignalEvent();

     //Add a signal handler to the signal manger.
     //NOTE: a signle SIGNO can only register one signal.
     //so the one last added will replace a previous 
     //registeded one.
     //RETURN: 0:OK, <0: FAIL
     int AddSignalHandler(int signo, CallBack* callback);

     //Del the signal handler.
     int DelSignalHandler(int signo);

     // Get Internal fd
     int GetFd() const ;
 private:
     //EventCallBack inteface.
     void HandleRead(EventLoop* ev, int fd);
     void HandleWrite(EventLoop* ev, int fd);

 private:
     //disable copy
     SignalManager(const SignalManager&);
     void operator=(const SignalManager&);

     struct Internal;

     Internal* data_;
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

     //add a signal event . caller provied a 
     //callback created by new.
     //callback will be managered by eventloop.
     //when delete, eventloop will delete it.
     //when add the same signo many times, only
     //the last time will in effect.
     //RETURN: 0:OK <0:fail
     int AddSigEvent(int signo, CallBack* cb);

     //delete a signal event
     //RETURN: 0:OK <0:fail
     int DelSigEvent(int signo);

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

     SignalManager*   signal_manager_;

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

inline int EventLoop::AddSigEvent(int signo, CallBack* cb) {
    return signal_manager_->AddSignalHandler(signo, cb);
}

inline int EventLoop::DelSigEvent(int signo) {
    return signal_manager_->DelSignalHandler(signo);
}


} // namespace net
} // namespace yanetlib
#endif //eventloop.h
