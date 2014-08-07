#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include "comm/common.h"
#include "net_common.h"
#include "eventloop.h"
#include "net_handler.h"
#include "gtest/gtest.h"

using namespace std;
using namespace yanetlib::net;
using namespace yanetlib::comm;

class MyUdpHandler : public UdpHandler {
 public:
     virtual  int HandleInput(EventLoop* ev, int cfd, char* buf, int blen) {
         return blen;
     }
     
     virtual  int HandlePkg(EventLoop* ev, int cfd, char* buf, int blen) {
         if (_wbuf->LeftSize() < blen) {
             COLIN_LOG(ERROR) << "write buffer space not enough.\n";
             return 0;
         }
         memcpy(_wbuf->ReadPtr(), buf, blen);
         _wbuf->Push(blen);
         ev->AddEvent(cfd, YANET_WRITABLE, this);
         return 0;
     }
     virtual void HandleError(EventLoop* ev, int fd) {
         COLIN_LOG(ERROR) << "UDP ERROR!\n";
     }
};

class MyTcpHandler : public TcpHandler {
 public:
     //default 60 sec
     explicit MyTcpHandler(long expire_sec = 20) : TcpHandler(expire_sec) { }
     
     virtual int HandleInput(EventLoop* ev, int cfd, char* buf, int blen) {
         return blen;
     }

     virtual  int HandlePkg(EventLoop* ev, int cfd, char* buf, int blen) {
         char tmp[512];
         if (blen+1 > (int)sizeof(tmp)) {
             fprintf(stderr, "client content too large!\n");
             return -1;
         }
         memcpy(tmp, buf, blen);
         tmp[blen] = '\0';
         fprintf(stdout, "Recv: %s\n", tmp);

         if (_wbuf->LeftSize() < blen) {
             COLIN_LOG(ERROR) << "tcp write buffer space not enough.\n";
             return 0;
         }
         memcpy(_wbuf->ReadPtr(), buf, blen);
         _wbuf->Push(blen);
         ev->AddEvent(cfd, YANET_WRITABLE, this);
         return 0;
     }

     virtual void HandleError(EventLoop* ev, int cfd) {
         COLIN_LOG(ERROR) << "error happen!\n";
     }

     virtual void HandleClose(EventLoop* ev, int cfd) {
         COLIN_LOG(INFO) << "client close.\n";
     }
};

class MyListenHandler : public ListenHandler {
 public:
     virtual void SetNetHandler(EventLoop* ev, int fd) {
         ev->AddEvent(fd, YANET_READABLE, new MyTcpHandler);
     }
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
    ev.AddEvent(sfd, YANET_READABLE, new MyUdpHandler);
    ev.AddEvent(listenfd, YANET_READABLE, new MyListenHandler);
    ev.Run(100);
}
