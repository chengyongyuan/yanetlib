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

#define MAX_BUF_SIZE (1<<16)

#if 0
class TcpHandler : public EventCallBack {
 public:
     TcpHandler() : rpos(0), wpos(0) { }
     ~TcpHandler() {}

     void HandleRead(EventLoop* ev, int fd) {
         printf("HandleRead...\n");
         int bread = YanetReadN(fd, rbuf+wpos, MAX_BUF_SIZE-wpos);
         if (bread == 0) {
             printf("Peer Close!\n");
             ev->DelEvent(fd, YANET_WRITABLE | YANET_READABLE );
             close(fd);
             return ;
         } else if (bread == -1) {
             printf("Peer Error, close!!\n");
             ev->DelEvent(fd, YANET_WRITABLE | YANET_READABLE );
             close(fd);
             return ;
         }
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
         printf("client fd:%d\n", cfd);
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
#endif

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
    ev.AddEvent(listenfd, YANET_READABLE, new ListenHandler(new MyTcpHandler)); 
    ev.Run();
}
