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

namespace yanetlib {
namespace net {

#define YANET_MAX_POLL_SIZE 4*1024
#define YANET_NONE          0
#define YANET_READABLE      1
#define YANET_WRITABLE      2

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

} //namespace net
} //namespace yanetlib

#endif //poller.h
