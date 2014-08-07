#ifndef YANETLIB_NET_NET_HANDLER_H
#define YANETLIB_NET_NET_HANDLER_H

//This file define different proto handler
//currently define: udp/tcp
//
//colincheng 2014/08/02
//

#include <arpa/inet.h>
#include "buffer.h"
#include "eventloop.h"

namespace yanetlib {
namespace net {

struct RemoteInfo {
    std::string rmt_addr; //remote ip
    int         rmt_port; //remote port
};

class UdpHandler;
class TcpHandler;
//interface for proto handler
class NetHandler : public EventCallBack {
 public:
     //interface from EventCallBack
     virtual void HandleRead(EventLoop* ev,  int fd) = 0;

     //interface from EventCallBack
     virtual void HandleWrite(EventLoop* ev, int fd) = 0;

     //interface from EventCallBack
     virtual void HandleLoop(EventLoop* ev, int fd)  = 0;

     //invoke when error happenen
     virtual void HandleError(EventLoop* ev, int fd) = 0;

     //invoke when socket first get some data.the function
     //return the real length of pkg
     //RETURN: <0: error; ==0: not yet. >0: got full pkg
     virtual int  HandleInput(EventLoop* ev, int fd, char* buf, int blen) = 0;

     //invoke after got full proto pkg.
     //RETURN: <0: error. ==0: ok
     virtual int  HandlePkg  (EventLoop* ev, int fd, char* buf, int blen) = 0;

     virtual ~NetHandler() { }
};


//handler for procssing tcp server socket
class ListenHandler : public EventCallBack {
 public:
     ListenHandler() {}

     void HandleRead(EventLoop* ev, int fd) {
         RemoteInfo rmt;
         int cfd = YanetAccept(NULL, fd, &rmt.rmt_addr, &rmt.rmt_port);
         if (cfd <= 0) { //ERROR
             COLIN_LOG(ERROR) << "Accept client error!\n";
             return ;
         }
         YanetNonBlock(NULL, cfd);
         //ev->AddEvent(cfd, YANET_READABLE, _handler);
         SetNetHandler(ev, cfd);
         HandleAccept(ev, cfd, rmt);
     }

     //nothing to do
     void HandleWrite(EventLoop* ev, int fd) { }

     //currently do nothings
     virtual void HandleLoop(EventLoop* ev,  int fd) { }

     //template method, invoke when successfully accept a client
     //default do nothing..
     virtual void HandleAccept(EventLoop* ev, int cfd, RemoteInfo rmt) { }

     //must implement to specify user define TcpHandler to
     //handler logic.
     virtual void SetNetHandler(EventLoop* ev, int fd) = 0;

 private:
     //disable evil
     ListenHandler(const ListenHandler& );
     void operator=(const ListenHandler& );
};

//proto handler for udp. user should inherit from this class
//when implement a udp server
class UdpHandler : public NetHandler {
 public:
     UdpHandler();
    
     ~UdpHandler();

     void HandleRead(EventLoop* ev, int fd);

     void HandleWrite(EventLoop* ev, int fd);

     //do nothing
     virtual void HandleLoop(EventLoop* ev, int fd) { }
 private:
     UdpHandler(const UdpHandler& );
     void operator=(const UdpHandler&);
 protected:
     struct sockaddr_in _caddr;
     Buffer* _rbuf;
     Buffer* _wbuf;
};

//proto handler for tcp. user should inhert from this class
//when implement a tcp server
class TcpHandler : public NetHandler {
 public:
      explicit TcpHandler(long expire_sec);

     ~TcpHandler();

     void HandleRead(EventLoop* ev, int fd);

     void HandleWrite(EventLoop* ev, int fd);

     void HandleLoop(EventLoop* ev, int fd);

     //tcp handler's interface.invoke when perr socket close
     virtual void HandleClose(EventLoop* ev, int fd) = 0;

 private:
     TcpHandler(const TcpHandler&);
     void operator=(const TcpHandler&);
 private:
     time_t creat_time_;
     time_t last_access_time_;
     long expire_sec_;
 protected:
     Buffer* _rbuf;
     Buffer* _wbuf;
};

} //net
} //yanetlib
#endif //net_handler.h
