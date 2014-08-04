#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include "net_common.h"
#include "net_handler.h"

namespace yanetlib {
namespace net {

UdpHandler::UdpHandler() : _rbuf(NULL), _wbuf(NULL) {
    memset(&_caddr, 0, sizeof(_caddr));
    _rbuf = new Buffer;
    _wbuf = new Buffer;
}

UdpHandler::~UdpHandler() {
    delete _rbuf;
    delete _wbuf;
}

void UdpHandler::HandleRead(EventLoop* ev, int fd) {
    socklen_t socklen = sizeof(_caddr);
    int bread = recvfrom(fd, _rbuf->ReadPtr(), _rbuf->LeftSize(), 0,
                (struct sockaddr*)&_caddr, &socklen);
    if (bread <= 0) {
        if (errno == EINTR || errno == EAGAIN) 
            return ;
        HandleError(ev, fd);
    }
    _rbuf->Push(bread);
    if (HandlePkg(ev, fd, _rbuf->DataPtr(), bread) == 0) {
        _rbuf->Pull(bread);
        if (_rbuf->LeftSize() == 0) {
            ev->DelEvent(fd, YANET_READABLE);
        }
    } else {
        HandleError(ev, fd);
    }
     
}

void UdpHandler::HandleWrite(EventLoop* ev, int fd) {
    int bwrite = sendto(fd, _wbuf->DataPtr(), _wbuf->DataSize(), 0, 
                 (struct sockaddr*)&_caddr, sizeof(_caddr));
    if (bwrite <= 0) {
        if (errno == EINTR || errno == EAGAIN)
            return ;
        HandleError(ev, fd);
    }
    _wbuf->Pull(bwrite);
    if (_wbuf->DataSize() == 0) {
        ev->DelEvent(fd, YANET_WRITABLE);
    }
}

TcpHandler::TcpHandler() : _rbuf(NULL), _wbuf(NULL) {
    _rbuf = new Buffer;
    _wbuf = new Buffer;
}

TcpHandler::~TcpHandler() {
    delete _rbuf;
    delete _wbuf;
}

void TcpHandler::HandleRead(EventLoop* ev, int fd) {
    int bread = YanetReadN(fd, _rbuf->ReadPtr(), _rbuf->LeftSize());
    if (bread < 0) {
        if (errno == EINTR || errno == EAGAIN) 
            return ;
        HandleError(ev, fd);
        ev->DelEvent(fd, YANET_READABLE|YANET_WRITABLE);
        close(fd);
    } else if(bread == 0) {
        HandleClose(ev, fd);
        ev->DelEvent(fd, YANET_READABLE|YANET_READABLE);
        close(fd);
    }
    _rbuf->Push(bread);

    int real_datalen = HandleInput(ev, fd, _rbuf->DataPtr(), _rbuf->DataSize());
    if (real_datalen > 0) {
        int ret = HandlePkg(ev, fd, _rbuf->DataPtr(), real_datalen);
        if (ret < 0) {
            HandleError(ev, fd);
            ev->DelEvent(fd, YANET_READABLE|YANET_WRITABLE);
            close(fd);
        }
        _rbuf->Pull(real_datalen);
    } else if (real_datalen == 0) { //need more
        if (_rbuf->LeftSize() == 0) { //need enlarge buff
            _rbuf->Extend();
        }
    }
}

void TcpHandler::HandleWrite(EventLoop* ev, int fd) {
    int bwrite = YanetWriteN(fd, _wbuf->DataPtr(), _wbuf->DataSize());
    if (bwrite <= 0) {
        if (errno == EINTR || errno == EAGAIN)
            return ;
        HandleError(ev, fd);
        ev->DelEvent(fd, YANET_READABLE|YANET_WRITABLE);
        close(fd);
    }
    _wbuf->Pull(bwrite);
    if (_wbuf->DataSize() == 0) {
        ev->DelEvent(fd, YANET_WRITABLE);
    }
}

} // net
} // yanetlib
