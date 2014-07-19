#include "thr_pool.h"

namespace yanetlib {
namespace comm {

static void* WorkerFunc(void *arg) {
    ThrPool* pool = (ThrPool*) arg;
    while (true) {
        ThrPool::job_t j;
        if (!pool->TakeJob(&j))
            break;
        (void)(j.f)(j.a);
    }
    pthread_exit(NULL);

    return NULL;
}

ThrPool::ThrPool(int sz, bool blocking) :
    nthreads_(sz), blockadd_(blocking) {
    VERIFY(pthread_attr_init(&attr_) == 0);
    VERIFY(pthread_attr_setstacksize(&attr_, 128<<10) == 0);
    pthread_t tid;
    for (int i = 0; i < nthreads_; ++i) {
        VERIFY(pthread_create(&tid, &attr_, &WorkerFunc, (void*)this) == 0);
        vtid_.push_back(tid);
    }
}

//all thread must stop before we release res.
ThrPool::~ThrPool() {
    for (size_t i = 0; i < vtid_.size(); ++i) {
        //posion job, make thread die
        job_t j;
        j.f = (void* (*)(void *))NULL;
        jobq_.Enqueue(j);
    }
    for (size_t i = 0; i < vtid_.size(); ++i) {
        VERIFY(pthread_join(vtid_[i], NULL) == 0);
    }
    VERIFY(pthread_attr_destroy(&attr_) == 0);
}

bool ThrPool::AddJob(void* (*f)(void *), void* a) {
    job_t job;
    job.f = f;
    job.a = a;
    return jobq_.Enqueue(job, blockadd_);
}

bool ThrPool::TakeJob(job_t *job) {
    jobq_.Dequeue(job);
    return job->f != NULL;
}

} //namespace comm
} //namespace yyanetlib
