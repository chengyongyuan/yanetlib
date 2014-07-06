#ifndef YANETLIB_NET_TIMER_H
#define YANETLIB_NET_TIMER_H

//Implement Timer interface, high performance user level
//timer is very essenial to network program. in general,
//timer can be implemented by  red-black tree, min-heap,
//link-list, multi-level link-list(linux kernel).
//For now, YANET choose the simplist link-list implementation
//just like REDIS. make it work if more important for me now :)
//event callback interface
// 
// colincheng @ 2014/07/06

namespace yanetlib {
namespace net {

#define YANET_NOMORE        -1

//some timer related utility function
void GetTime(long* nowsec, long* nowms); 
void AddMilliSecondsToNow(long long miliseconds, long* sec, long* ms);

//timer caller interface
class TimerCallBack {
 public:
     TimerCallBack() {}
     
     virtual ~TimerCallBack() {}

     //Timer Callback function, put timer logic here
     //RETURN: YANET_NOMORE: timer function done, delete the timer
     //        >=0, put it in timer list again( usually for repeat 
     //        timer event.)
     virtual int HandleTimer(/*EventLoop* ev, */long long id) = 0;

};

struct TimerListObject {
    long long      id; 
    long           when_sec;
    long           when_ms;
    TimerCallBack* te;
    struct TimerListObject* next;
    
    TimerListObject(long long id_, 
                           long sec,
                           long ms,
                           TimerCallBack* te_,
                           struct TimerListObject* next_) 
        : id(id_), when_sec(sec), when_ms(ms), 
          te(te_), next(next_) { }
};

class TimerManager {
 public:
     TimerManager();

     ~TimerManager();

     //add a timer event to eventloop
     //milliseconds 毫秒
     //RETURN: >=0:timerid <0:fail
     int AddTimer(long long milliseconds, TimerCallBack* te);

     //del a timer event to eventloop
     //RETURN: 0:OK -1:fail
     int DelTimer(long long id);

     //process every pending timer  event
     int ProcessTimerEvent();

     //return the Nearest Timer timer about to fire.
     TimerListObject* SearchNearestTimer() const;
     
     long long GetCurTimerId() const { return next_timer_id_; }

 private:
     long long GenNextTimerId() { return next_timer_id_ ++ ; }
 private:
     //keep trace max timer fd generated
     long long            next_timer_id_;

     //timer list head
     //TODO: we can implement timer more efficently,
     //but we want to make it work first:-
     TimerListObject*    timer_head_;

};

} //namespace net
} //namespace yanetlib
#endif //timer.h
