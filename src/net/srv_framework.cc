#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "net_common.h"
#include "srv_framework.h"

using namespace yanetlib::comm;

namespace yanetlib {
namespace net {

SrvFramework::SrvFramework(SrvConf srvconf) : _ev(NULL),
    _srv_conf(srvconf) {
}

SrvFramework::~SrvFramework() {
    if (_ev != NULL) {
        delete _ev;
        _ev = NULL;
    }
}

bool SrvFramework::Init() {
    //first daemonize, then other initialzation
    if (_srv_conf.is_daemon) {
        if (YanetDaemonize(0) != 0) 
            return false;
    }
    //init event loop
    if ((_ev = new EventLoop) == NULL) return false;

    //init srv log
    SrvLogConf lconf = _srv_conf.logconf;
    if (!_srv_log.Init(lconf.loglevel, lconf.path, \
                lconf.type, lconf.maxsz)) return false;

    //init resource
    if (!InitRlimit()) return false;

    //init socket
    if (!InitSrvSocket()) return false;

    printf(ANSI_COLOR_GREEN "Srv Start!\n" ANSI_COLOR_RESET);
    LOG_INFO(_srv_log, "Srv Start!");
    return true;
}

bool SrvFramework::InitRlimit() {
    struct rlimit flimit;
    int max_fd_count = (_srv_conf.max_fd_count < MIN_FD_LIMIT) ? \
                       MIN_FD_LIMIT : _srv_conf.max_fd_count;
    flimit.rlim_cur = max_fd_count;
    flimit.rlim_max = max_fd_count;
    if (setrlimit(RLIMIT_NOFILE, &flimit) != 0) return false;
    return true;
}

bool SrvFramework::InitSrvSocket() {
    if (_srv_conf.srvlist.size() > MAX_SRV_SOCKET) {
        LOG_ERROR(_srv_log, "too many server socket.");
        return false;
    }
    for (size_t i = 0; i < _srv_conf.srvlist.size(); ++i) {
        if (!InitOneSrvSock(_srv_conf.srvlist[i])) {
            //LOG_ERROR(_srv_log, "init srv(%s) fail.", Addr2Str(_srv_conf.srvlist[i].addr));
            return false;
        }
    }
    return true;
}

bool SrvFramework::InitOneSrvSock(const OneSrvConf& srvconf) {
    //char errbuf[256];
    //int fd;
    /*TODO: problem
    switch(srvconf.type) {
        case SrvType_UDP:
            fd = YanetUdpServer(errbuf, srvconf.addr.ip.c_str(),
                                    srvconf.addr.port);
            if (fd < 0) {
                LOG_ERROR(_srv_log, "init udp socket(%s:%d) fail.",
                          srvconf.addr.ip.c_str(), srvconf.addr.port);
                return false;
            }
            break;
        case SrvType_TCP:
            fd = YanetTcpServer(errbuf, srvconf.addr.ip.c_str(),
                                    srvconf.addr.port);
            if (fd < 0) {
                LOG_ERROR(_srv_log, "init tcp socket(%s:%d) fail.",
                          srvconf.addr.ip.c_str(), srvconf.addr.port);
                return false;
            }
            break;
        case SrvType_UNIX:
            LOG_ERROR(_srv_log, "unix socket currently not implemented!");
            return false;
        default:
            LOG_ERROR(_srv_log, "unkown socket type!");
            return false;
    };
    if (!srvconf.handler || 0 > _ev->AddEvent(fd, YANET_READABLE, srvconf.handler)) {
        return false;
    }
    */
    return true;
}

void SrvFramework::Run() {
}

} // net
} //yanetlib
