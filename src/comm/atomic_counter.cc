#include "atomic_counter.h"

namespace yanetlib {
namespace comm {

#ifdef GCC_HAVE_ATOMICS 
AtomicCounter::AtomicCounter() : counter_(0) {
}

AtomicCounter::AtomicCounter(ValueType init_val) 
    : counter_(init_val) {
}

AtomicCounter::AtomicCounter(const AtomicCounter& counter) 
    : counter_(counter){
}

AtomicCounter& AtomicCounter::operator=(const AtomicCounter& counter) {
    __sync_test_and_set(&counter, counter.value());
    return *this;
}

AtomicCounter& AtomicCounter::operator=(ValueType value) {
    __sync_test_and_set(&counter, value);
    return *this;
}

AtomicCounter::~AtomicCounter() {
}

#else
AtomicCounter::AtomicCounter() {
    counter_.val_ = 0;
}

AtomicCounter::AtomicCounter(ValueType init_val) {
    counter_.val_ = init_val;
}

AtomicCounter::AtomicCounter(const AtomicCounter& counter) {
    counter_.val_ = counter.value();
}

AtomicCounter& AtomicCounter::operator=(const AtomicCounter& counter) {
    ScopedLock<Mutex> lock(&counter_.mutex_);
    counter_.val_  = counter.value();
    return *this;
}

AtomicCounter& AtomicCounter::operator=(ValueType value) {
    ScopedLock<Mutex> lock(&counter_.mutex_);
    counter_.val_  = value;
    return *this;
}

AtomicCounter::~AtomicCounter() {
}
#endif

} //namespace comm
} //namespace yanetlib
