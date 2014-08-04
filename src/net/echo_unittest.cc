#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include "net_common.h"
#include "eventloop.h"
#include "gtest/gtest.h"

using namespace std;
using namespace yanetlib::net;

#define MAX_BUF_SIZE (1<<16)

class TcpHandler : public EventCallBack {
 public:
     TcpHandler() : rpos(0), wpos(0) { }
     ~TcpHandler() {}

     void HandleRead(EventLoop* ev, int fd) {
         int bread = YanetReadN(fd, rbuf+wpos, MAX_BUF_SIZE-wpos);
         assert(bread != -1);
         wpos += bread;
         if (rpos != wpos) {
             ev->AddEvent(fd, YANET_WRITABLE, this); 
         }
         if (wpos == MAX_BUF_SIZE) {
             ev->DelEvent(fd, YANET_READABLE);
         }
     }

     void HandleWrite(EventLoop* ev, int fd) {
         int bwrite = YanetWriteN(fd, rbuf+rpos, wpos-rpos);
         assert(bwrite != -1);
         rpos += bwrite;
         if (rpos == wpos) { 
             rpos = wpos = 0;
             ev->DelEvent(fd, YANET_WRITABLE);
         }
         if (wpos != MAX_BUF_SIZE) {
             ev->AddEvent(fd, YANET_READABLE, this);
         }
     }

 private:
     char rbuf[MAX_BUF_SIZE];
     int  rpos;
     int  wpos;
     struct sockaddr_in caddr;
};

class ListenHandler : public EventCallBack {
 public:
     ListenHandler() { }
     ~ListenHandler() { }

     void HandleRead(EventLoop* ev, int fd) {
         int cfd = YanetAccept(NULL, fd, NULL, NULL);
         assert(cfd != YANET_ERR);
         assert(YANET_OK == YanetNonBlock(NULL, cfd));
         ev->AddEvent(cfd, YANET_READABLE, new TcpHandler());
     }

     void HandleWrite(EventLoop* ev, int fd) {
     }
};

class UdpHandler : public EventCallBack {
 public:
     UdpHandler() : rpos(0), wpos(0) { }
     ~UdpHandler() {}

     void HandleRead(EventLoop* ev, int fd) {
         //int bread = YanetReadN(fd, rbuf+wpos, MAX_BUF_SIZE-wpos);
         socklen_t clen = sizeof(caddr);
         int bread = recvfrom(fd, rbuf+wpos, MAX_BUF_SIZE-wpos,0, (struct sockaddr*)&caddr,&clen);
         assert(bread != -1);
         wpos += bread;
         if (rpos != wpos) {
             ev->AddEvent(fd, YANET_WRITABLE, this); 
         }
         if (wpos == MAX_BUF_SIZE) {
             ev->DelEvent(fd, YANET_READABLE);
         }
     }

     void HandleWrite(EventLoop* ev, int fd) {
         //int bwrite = YanetWriteN(fd, rbuf+rpos, wpos-rpos);
         int bwrite = sendto(fd, rbuf+rpos, wpos - rpos, 0, (struct sockaddr*)&caddr, sizeof(caddr));
         assert(bwrite != -1);
         rpos += bwrite;
         if (rpos == wpos) { 
             rpos = wpos = 0;
             ev->DelEvent(fd, YANET_WRITABLE);
         }
         if (wpos != MAX_BUF_SIZE) {
             ev->AddEvent(fd, YANET_READABLE, this);
         }
     }

 private:
     char rbuf[MAX_BUF_SIZE];
     int  rpos;
     int  wpos;
     struct sockaddr_in caddr;
};

int main(int argc, char **argv)
{
    char errbuf[512];
    int sfd = YanetUdpServer(NULL, "*", 9099);
    int listenfd = YanetTcpServer(errbuf, "192.168.1.110", 9099);
    assert(sfd != YANET_ERR);
    assert(listenfd != YANET_ERR);
    EventLoop ev;
    assert(ev.InitEventLoop() == 0);
    ev.AddEvent(sfd, YANET_READABLE, new UdpHandler());
    ev.AddEvent(listenfd, YANET_READABLE, new ListenHandler()); 
    ev.Run();
}
