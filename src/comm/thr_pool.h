#ifndef YANETLIB_COMM_THR_POOL_H
#define YANETLIB_COMM_THR_POOL_H

//thread pool, currenly not support auto-resizing
//
//TODO: auto resizing

#include <vector>
#include "common.h"
#include "fifo.hpp"

namespace yanetlib {
namespace comm {

class ThrPool {
 public:
     struct job_t {
         void* (*f) (void *); //work function 
         void* a;
     };

     ThrPool(int sz, bool blocking = true);

     ~ThrPool();

     bool AddJob(void* (*f)(void *), void* a);
     
     template<class C, class A>
     bool AddJob(C* o, void (C::*m)(A), A a);

     bool TakeJob(job_t *j);

 private:
    pthread_attr_t attr_;
    int nthreads_;
    bool blockadd_;

    Fifo<job_t> jobq_;
    std::vector<pthread_t> vtid_;
};

template<class C, class A>
bool ThrPool::AddJob(C* o, void (C::*m)(A), A a) {
    class ObjectWrapper {
     public:
         C* o;
         void (C::*m)(A);
         A a;
         static void* func(void* arg) {
            ObjectWrapper* wrap = (ObjectWrapper*)arg;
            C* oo = wrap->o;
            void (C::*mm)(A) = wrap->m;
            A aa = wrap->a;
            //invoke it
            (oo->*mm)(aa);

            delete wrap;
            return 0;
         }
    };
    ObjectWrapper* obj = new ObjectWrapper;
    obj->o = o;
    obj->m = m;
    obj->a = a;
    return  AddJob(&ObjectWrapper::func, (void*)obj);
}

} // namespace comm
} // namespace yanetlib
#endif //thr_pool.h
