#include <sys/types.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "net_common.h"
#include "poller.h"
#include "comm/config_parser.h"
#include "comm/common.h"
#include "eventloop.h"

#include <string>
#include <vector>
#include <set>

using namespace std;
using namespace yanetlib::net;
using namespace yanetlib::comm;

//implement a simple echo srv(udp/tcp)
//to verify those network common function.
//

struct SockAddr {
    string addr;
    int   port;
    SockAddr(string a, int p) :
        addr(a), port(p) {}
};

bool ParseAddrFromConf(const string& conf, vector<SockAddr>* vaddr, bool is_tcp) {
    SimpleConf config;
    vector<string> tcp_addr, udp_addr;
    vector<int>    tcp_port, udp_port;

    if (!config.Init(conf)) return false;
    if (is_tcp) {
        config.GetArray("TCP_IP", tcp_addr);
        config.GetIntArray("TCP_PORT", tcp_port);
        if (tcp_addr.size() != tcp_port.size()) return false;
        for (size_t i = 0; i < tcp_addr.size(); ++i) {
            vaddr->push_back(SockAddr(tcp_addr[i], tcp_port[i]));
        }
    } else {
        config.GetArray("UDP_IP", udp_addr);
        config.GetIntArray("UDP_PORT", udp_port);
        if (udp_addr.size() != udp_port.size()) return false;
        for (size_t i = 0; i < udp_addr.size(); ++i) {
            vaddr->push_back(SockAddr(udp_addr[i], udp_port[i]));
        }
    }

    return true;
}

bool CreateSock(const vector<SockAddr>& vaddr, set<int>* vfd, bool is_tcp) {
    if (is_tcp) {
        for (size_t i = 0; i < vaddr.size(); ++i) {
            int fd = -1;
            if ((fd = YanetTcpServer(NULL, vaddr[i].addr.c_str(), vaddr[i].port)) < 0) 
                return false;
            vfd->insert(fd);
        }
    } else {
        for (size_t i = 0; i < vaddr.size(); ++i) {
            int fd = -1;
            if ((fd = YanetUdpServer(NULL, vaddr[i].addr.c_str(), vaddr[i].port)) < 0) 
                return false;
            vfd->insert(fd);
        }
    }
    return true;
}

class EchoSrv : public EventCallBack {
 public:
     EchoSrv() {}

     ~EchoSrv() {}
     
     bool InitSrv(const string& conf);

     //interface
     void HandleRead(int fd)  {};
     void HandleWrite(int fd) {};

 private:
     vector<SockAddr> tcp_addr_;
     vector<SockAddr> udp_addr_;
     set<int> allfd_;
};

bool EchoSrv::InitSrv(const string& conf) {
    assert(ParseAddrFromConf(conf, &tcp_addr_, true));
    assert(ParseAddrFromConf(conf, &udp_addr_, false));
    assert(CreateSock(tcp_addr_, &allfd_, true));
    assert(CreateSock(udp_addr_, &allfd_, false));
    return true;
}

EventLoop evloop;

void MySigHandler() {
    static int cnt;
    fprintf(stderr, "this is a test!\n");
    if (cnt++ == 2) { evloop.DelSigEvent(SIGINT); }
    
}

class Dummy {
 public:
    void MySigHandler() {
        fprintf(stderr, "this is a method function\n");
        if (cnt++ == 2) { evloop.DelSigEvent(SIGTERM); }
    }
    static int cnt;
};
int Dummy::cnt;

int main(int argc, char **argv)
{
    assert(0 == YanetDaemonize(1));
    printf("should not seen any message!\n");
    EchoSrv srv;
    Dummy* dummy = new Dummy;
    assert(srv.InitSrv("srv.conf"));
    assert(0 == evloop.InitEventLoop());
    assert(0 == evloop.AddSigEvent(SIGINT, NewPermanentCallback(MySigHandler)));
    assert(0 == evloop.AddSigEvent(SIGTERM, NewPermanentCallback(dummy, &Dummy::MySigHandler)));
    evloop.Run();
    return 0;
}

