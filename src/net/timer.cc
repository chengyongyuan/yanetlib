#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include "timer.h"

namespace yanetlib {
namespace net {
    
void GetTime(long* nowsec, long* nowms) {
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    *nowsec = tv.tv_sec;
    *nowms = tv.tv_usec/1000;
}

void AddMilliSecondsToNow(long long miliseconds, long* sec, long* ms) {
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

TimerManager::TimerManager() : next_timer_id_(-1), timer_head_(NULL) { }

TimerManager::~TimerManager() { 
    TimerListObject* tobj = timer_head_;

    while(tobj) {
        timer_head_ = tobj->next;;
        delete tobj;
        tobj = timer_head_;
    }
    timer_head_ = NULL;
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
TimerListObject* TimerManager::SearchNearestTimer() const {
    TimerListObject* tobj = timer_head_;
    TimerListObject* nearest = NULL;

    while(tobj) {
        if (!nearest || tobj->when_sec < nearest->when_sec ||
                (tobj->when_sec == nearest->when_sec &&
                 tobj->when_ms < nearest->when_ms))
            nearest = tobj;
        tobj = tobj->next;
    }
    return nearest;
}

int TimerManager::AddTimer(long long milliseconds, TimerCallBack* te) {
    long long id = GenNextTimerId();
    long now_sec, now_ms;

    AddMilliSecondsToNow(milliseconds, &now_sec, &now_ms);
    if (te == NULL) return -1;
    TimerListObject* tobj = new TimerListObject(id, now_sec, now_ms, te, timer_head_);
    if (tobj == NULL) return -3;
    timer_head_ = tobj;

    return id;
}

int TimerManager::DelTimer(long long id) {
    TimerListObject* tobj, *prev = NULL;

    tobj = timer_head_;
    while (tobj) {
        if (tobj->id == id) {
            if (prev == NULL)
                timer_head_ = tobj->next;
            else
                prev->next = tobj->next;
            //TODO: TimeDead Callback
            //te->TimerEnd()
            delete tobj;
            return 0;
        }
        prev = tobj;
        tobj = tobj->next;
    }
    //Not found the timer event with id
    return -1;
}

int TimerManager::ProcessTimerEvent() {
    int processed = 0;
    TimerListObject* tobj;
    long long maxid;

    tobj = timer_head_;
    maxid = next_timer_id_-1;
    while (tobj) {
        long nowsec, nowms;
        long long id;
        if (tobj->id > maxid) {
            tobj = tobj->next;
            continue;
        }
        GetTime(&nowsec, &nowms);
        //need process this timer
        if (nowsec > tobj->when_sec || 
            (nowsec == tobj->when_sec && nowms > tobj->when_ms)) {
            int retval;

            id = tobj->id;
            assert(tobj->te != NULL);
            retval = tobj->te->HandleTimer(id);
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
                AddMilliSecondsToNow(retval, &tobj->when_sec, &tobj->when_ms);
            } else {
                DelTimer(id);
            }
            tobj = timer_head_;
        } else {
            tobj = tobj->next;
        }
    }
    return processed;
}

} //namespace net
} //namespace yanetlib
