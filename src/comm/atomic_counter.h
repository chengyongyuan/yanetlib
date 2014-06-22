#ifndef YANETLIB_COMM_ATOMIC_COUNTER_H
#define YANETLIB_COMM_ATOMIC_COUNTER_H

#include "common.h"

namespace yanetlib {
namespace comm {

#if (__GNUC__ == 4 && __GNUC__MINOR__ >= 2) || (__GUNC__ > 4)
#define GCC_HAVE_ATOMICS
#endif

//Class that implement a atomic counter
//a atomic counter is usefull in many multi-threading 
//programs. NOTE:GCC >= 4.1.2 provide atomic operations
//otherwise, we use Mutex
class AtomicCounter {
 public:
     typedef int ValueType;

     AtomicCounter();

     explicit AtomicCounter(ValueType init_val);

     ~AtomicCounter();

     //copy cons
     AtomicCounter(const AtomicCounter& counter);

     //copy asignment.
     AtomicCounter& operator=(const AtomicCounter& counter);

     //copy ValueType to AtomicCounter
     AtomicCounter& operator=(ValueType value);

     //implicit typedef change to ValueType
     operator ValueType()const ;

     //return the underying val
     ValueType value() const ;

     //++/-- operators, notice we return ValueType
     //instread of AtomicCounter
     ValueType operator++();      //prefix ++

     ValueType operator++(int);   //postfix ++

     ValueType operator--();      //prefix --

     ValueType  operator--(int);  //postfix --

     bool operator!() const ;

 private:
#ifdef GCC_HAVE_ATOMICS
     typedef ValueType AtomicImpl;
#else
     struct AtomicImpl {
         mutable Mutex mutex_;
         int val_;
     };
#endif
     AtomicImpl counter_;
};

#ifdef GCC_HAVE_ATOMICS
inline AtomicCounter::ValueType AtomicCounter::operator++() {
    return __sync_add_and_fetch(&counter_, 1);
}

inline AtomicCounter::ValueType AtomicCounter::operator++(int) {
    return __sync_fetch_and_add(&counter, 1);
}

inline AtomicCounter::ValueType AtomicCounter::operator--() {
    return __sync_sub_and_fetch(&counter_, 1);
}

inline AtomicCounter::ValueType AtomicCounter::operator--(int) {
    return __sync_fetch_and_sub(&counter_, 1);
}

inline  AtomicCounter::operator AtomicCounter::ValueType() const {
    return counter_;
}

inline AtomicCounter::ValueType AtomicCounter::value() const {
    return counter_;
}

bool AtomicCounter::operator!() const {
    return counter__ == 0;
}
#else
inline AtomicCounter::ValueType AtomicCounter::operator++() {
    ValueType result;
    {
        ScopedLock<Mutex> lock(&counter_.mutex_);
        ++counter_.val_;
        result = counter_.val_;
    }
    return result;
}

inline AtomicCounter::ValueType AtomicCounter::operator++(int) {
    ValueType result;
    {
        ScopedLock<Mutex> lock(&counter_.mutex_);
        result = counter_.val_;
        ++counter_.val_;
    }
    return result;
}

inline AtomicCounter::ValueType AtomicCounter::operator--() {
    ValueType result;
    {
        ScopedLock<Mutex> lock(&counter_.mutex_);
        --counter_.val_;
        result = counter_.val_;
    }
    return result;
}

inline AtomicCounter::ValueType AtomicCounter::operator--(int) {
    ValueType result;
    {
        ScopedLock<Mutex> lock(&counter_.mutex_);
        result = counter_.val_;
        --counter_.val_;
    }
    return result;
}

inline AtomicCounter::operator AtomicCounter::ValueType() const {
    ValueType result;
    {
        ScopedLock<Mutex> lock(&counter_.mutex_);
        result = counter_.val_;
    }
    return result;
}

inline AtomicCounter::ValueType AtomicCounter::value() const {
    ValueType result;
    {
        ScopedLock<Mutex> lock(&counter_.mutex_);
        result = counter_.val_;
    }
    return result;
}

inline bool AtomicCounter::operator!() const {
    bool result;
    {
        ScopedLock<Mutex> lock(&counter_.mutex_);
        result = counter_.val_ == 0;
    }
    return result;
}
#endif
} // namespace comm 
} // namespace yanetlib
#endif //aotmic_counter.h
