#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h> //for gethostby_xxx
#include <string.h>
#include <stdarg.h>
#include "net_common.h"

namespace yanetlib {
namespace net {

static void YanetSetError(char* err, const char*fmt, ...) {
    va_list ap;

    if (!err) return ;

    va_start(ap, fmt);
    vsnprintf(err, YANET_MAX_ERR_SIZE, fmt, ap);
    va_end(ap);
}

int YanetNonBlock(char* err, int fd) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        YanetSetError(err, "fcntl(F_GETFL): %s\n", strerror(errno));
        return YANET_ERR;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        YanetSetError(err, "fcntl(F_SETFL): %s\n", strerror(errno));
        return YANET_ERR;
    }
    return YANET_OK;
}

int YanetNoDelay(char* err, int fd) {
    int yes = 1;

    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1) {
        YanetSetError(err, "setsockopt(TCP_NODELAY): %s\n", strerror(errno));
        return YANET_ERR;
    }
    return YANET_OK;
}

int YanetKeepAlive(char* err, int fd) {
    int yes = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1) {
        YanetSetError(err, "setsockopt(SO_KEEPALIVE): %s\n", strerror(errno));
        return YANET_ERR;
    }
    return YANET_OK;
}

int YanetSetSendBuffer(char* err, int fd, int bufsize) {
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize)) == -1) {
        YanetSetError(err, "setsockopt(SO_SNDBUF): %s\n", strerror(errno));
        return YANET_ERR;
    }
    return YANET_OK;
}

int YanetSetRecvBuffer(char* err, int fd, int bufsize) {
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize)) == -1) {
        YanetSetError(err, "setsockopt(SO_RCVBUF): %s\n", strerror(errno));
        return YANET_ERR;
    }
    return YANET_OK;
}

int YanetReadN(int fd, char* buff, int count) {
    int nread, totlen = 0; 

    while (totlen != count) {
        nread = read(fd, buff, count-totlen);
        if (nread == 0) return totlen;
        if (nread == -1) return -1;
        totlen += nread;
        buff += nread;
    }
    return totlen;
}

int YanetWriteN(int fd, char* buff, int count) {
    int nwrite, totlen = 0;

    while (totlen != count) {
        nwrite = write(fd, buff, count-totlen);
        if (nwrite == 0) return totlen;
        if (nwrite == -1) return -1;
        totlen += nwrite;
        buff += nwrite;
    }
    return totlen;
}

int YanetAccept(char* err, int serversock, char* ip, int* port) {
    int fd;
    struct sockaddr_in sa;
    unsigned int sa_len;

    while (1) {
        sa_len = sizeof(sa);
        fd = accept(serversock, (struct sockaddr*)&sa, &sa_len);
        if (fd == -1) {
            if (errno == EINTR)
                continue;
            else {
                YanetSetError(err, "accept: %s\n", strerror(errno));
                return YANET_ERR;
            }
        }
        break;
    }
    if (ip) strcpy(ip, inet_ntoa(sa.sin_addr));
    if (port) *port = ntohs(sa.sin_port);
    return YANET_OK;
}

int YanetTcpServer(char* err, const char* bindaddr, int port) {
    int s, reuse = 1;
    struct sockaddr_in sa;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        YanetSetError(err, "socket: %s\n", strerror(errno));
        return YANET_ERR;
    }
    //tcp server reuse addr
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        YanetSetError(err, "setsockopt(SO_REUSEADDR): %s\n", strerror(errno));
        close(s);
        return YANET_ERR;
    }
    
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(INADDR_ANY);
    sa.sin_port = htons(port);
    if (bindaddr) {
        if (inet_aton(bindaddr, &sa.sin_addr) == 0) {
            YanetSetError(err, "Invalid network address!\n");
            close(s);
            return YANET_ERR;
        }
    }
    if (bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        YanetSetError(err, "bind: %s\n", strerror(errno));
        close(s);
        return YANET_ERR;
    }
    //magic 511 come from nginx!
    if (listen(s, 511) == -1) {
        YanetSetError(err, "listen: %s\n", strerror(errno));
        close(s);
        return YANET_ERR;
    }
    return s;
}

int YanetResolve(char* err, const char* host, char* ip) {
    struct sockaddr_in sa;

    if (inet_aton(host, &sa.sin_addr) == 0) {
        struct hostent *he;
        if ((he = gethostbyname(host)) == NULL) {
            YanetSetError(err, "Can't Resolve: %s\n", host);
            return YANET_ERR;
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(in_addr));
    }
    strcpy(ip, inet_ntoa(sa.sin_addr));
    return YANET_OK;
}

#define YANET_CONNECT_FLAG_NONE 0
#define YANET_CONNECT_FLAG_NONBLOCK 1
static int YanetGenericConnect(char* err, const char* ip, int port, int flags) {
    int s, reuse = 1;
    struct sockaddr_in sa;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        YanetSetError(err, "socket: %s\n", strerror(errno));
        return YANET_ERR;
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        YanetSetError(err, "setsockopt(SO_REUSEADDR): %s\n", strerror(errno));
        close(s);
        return YANET_ERR;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    if (inet_aton(ip, &sa.sin_addr) == 0) {
        struct hostent *he;

        if ((he = gethostbyname(ip)) == NULL) {
            YanetSetError(err, "Can't Resolve: %s\n", ip);
            close(s);
            return YANET_ERR;
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(in_addr));
    }
    if (flags & YANET_CONNECT_FLAG_NONBLOCK) {
        if (YanetNonBlock(err, s) != YANET_OK) {
            close(s);
            return YANET_ERR;
        }
    }
    if (connect(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        if (errno == EINPROGRESS && 
            flags & YANET_CONNECT_FLAG_NONBLOCK)
            return s;
        YanetSetError(err, "connect: %s\n", strerror(errno));
        close(s);
        return YANET_ERR;
    }
    return s;
}

int YanetConnect(char* err, const char* ip, int port) {
    return YanetGenericConnect(err, ip, port, YANET_CONNECT_FLAG_NONE);
}

int YanetNonBlockConnect(char* err, const char* ip, int port) {
    return YanetGenericConnect(err, ip, port, YANET_CONNECT_FLAG_NONBLOCK);
}

} //namespace net
} //namespace yanetlib
