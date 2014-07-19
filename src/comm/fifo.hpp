#ifndef YANETLIB_COMM_FIFO_HPP
#define YANETLIB_COMM_FIFO_HPP

//A simple fifo queue template

#include <errno.h>
#include <list>
#include "common.h"

namespace yanetlib {
namespace comm {

template <class T>
class Fifo {
 public:
     Fifo(int maxsz = 0);

     ~Fifo();

     bool Enqueue(T, bool blocking = true);

     void Dequeue(T*);

     int size();
 private:
     std::list<T> q_;
     //maximun capacity of queue, block Enqueue if threads exceeds this limit.
     int maxsz_; 
     Mutex m_;
     //signal not empty
     pthread_cond_t not_empty_c_;
     //singal not full
     pthread_cond_t has_space_c_;
};

template <class T>
Fifo<T>::Fifo(int maxsz) : maxsz_(maxsz) {
    VERIFY(pthread_cond_init(&not_empty_c_, NULL) == 0);
    VERIFY(pthread_cond_init(&has_space_c_, NULL) == 0);
}

template <class T>
Fifo<T>::~Fifo() {
    //fifo is delete when there is no threads are using it
    VERIFY(pthread_cond_destroy(&not_empty_c_) == 0);
    VERIFY(pthread_cond_destroy(&has_space_c_) == 0);
}

template <class T>
int Fifo<T>::size() {
    ScopedLock<Mutex> sl(&m_);
    return q_.size();
}

template <class T>
bool Fifo<T>::Enqueue(T t, bool blocking) {
    ScopedLock<Mutex> sl(&m_);
    while (1) {
        if (!maxsz_ || q_.size() < (size_t)maxsz_) {
            q_.push_back(t);
            break;
        }
        if (blocking)
            VERIFY(pthread_cond_wait(&has_space_c_, m_.GetRaw()) == 0);
        else
            return false;
    }
    VERIFY(pthread_cond_signal(&not_empty_c_) == 0);
    return true;
}

template <class T>
void Fifo<T>::Dequeue(T* t) {
    ScopedLock<Mutex> sl(&m_);
    while (1) {
        if (!q_.empty()) {
            *t = q_.front();
            q_.pop_front();
            if (maxsz_ && q_.size() < (size_t)maxsz_) {
                VERIFY(pthread_cond_signal(&has_space_c_) == 0);
            }
            break;
        } else {
            VERIFY(pthread_cond_wait(&not_empty_c_, m_.GetRaw()) == 0);
        }
    }
    return ;
}

} // namespace comm
} // namespace yanetlib
#endif //fifo.hpp
