#ifndef YANETLIB_NET_SRV_FRAMEWORK_H
#define YANETLIB_NET_SRV_FRAMEWORK_H

//Define a event based srv framework.
//
//colincheng @ 2014/08/04
//

#include "comm/common.h"
#include "comm/rotate_log.h"
#include "eventloop.h"
#include <string>
#include <vector>
#include <map>

namespace yanetlib {
namespace net {

struct SockAddr {
    std::string ip;
    int port;
};

enum SrvType {
    SrvType_UDP  = 1,
    SrvType_TCP  = 2,
    SrvType_UNIX = 3,
};

struct OneSrvConf {
    SrvType type;
    SockAddr addr;
    EventCallBack* handler;
};

struct SrvLogConf {
    std::string path;
    yanetlib::comm::RotateLog::RotateType  type;    
    //only in effect if type==ROTATE_BY_SIZE
    int maxsz;
    yanetlib::comm::RotateLog::LogLevel loglevel;
};

struct SrvConf {
    //different srv/port/handler config
    std::vector<OneSrvConf> srvlist;
    //srv log config
    SrvLogConf logconf;
    //max idle link in sec. 
    int max_idle_sec;
    //max accept client count
    int max_clt_count;
    //max resource fd count(setrlimit.)
    int max_fd_count;
    //daemon mode
    bool is_daemon;
};

struct SockCtx {
    //sockfd
    int fd;
    //address, for srv socket, addr is srv local addr,
    //for clt socket is remote addr
    SockAddr addr;
    //last active time
    time_t last_active_time;
    //create time
    time_t creat_time;
};

class SrvFramework {
 public:
     //typedef
     typedef std::map<int, SockCtx> SockMap;

     enum {
         MAX_SRV_SOCKET = 20,
         MIN_FD_LIMIT   = 1024,
     };

 public:
     //constructor
     SrvFramework(SrvConf srvconf);

     //destructor
     ~SrvFramework();

     //initialization
     bool Init();

     //run srv
     void Run();
 private:
     //disable evil
     SrvFramework(const SrvFramework& );
     void operator=(const SrvFramework& );

     bool InitSrvSocket();
     bool InitOneSrvSock(const OneSrvConf& srvconf);
     bool InitRlimit();

 private:
     EventLoop* _ev;
     SrvConf   _srv_conf;
     yanetlib::comm::RotateLog _srv_log;
     SockMap   _srv_sock_map;
     SockMap   _clt_sock_map;
};

} //net
} //yanetlib

#endif // srv_framework.h
