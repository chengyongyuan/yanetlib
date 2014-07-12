#ifndef YANETLIB_NET_NETCOMMON_H
#define YANETLIB_NET_NETCOMMON_H

//This file implemnt many network specific function.
//these general purpose function are widely used in
//most of the network program. many of the utility
//functions are inspired by REDIS:a popular open-
//sourced Key-Value Database system. (thx to open-
//source code :)
//
//colincheng 2014/6/29

namespace yanetlib {
namespace net {

enum {
    YANET_OK =   0,
    YANET_ERR = -1,
    YANET_MAX_ERR_SIZE = 256,
};

//set fd to close on exit. if err is not NULL
//put error msg in err buffer
//RETURN: YANET_OK if ok, YANET_ERR if fail
int YanetCloseOnExec(char* err, int fd);

//set fd to nonblocking mode. if err is not NULL
//put error msg in err buffer
//RETURN: YANET_OK if ok, YANET_ERR if fail
int YanetNonBlock(char* err, int fd);

//set fd to turn on no-delay . if err is not NULL
//put error msg in err buffer
//RETURN: YANET_OK if ok, YANET_ERR if fail
int YanetNoDelay(char* err, int fd);

//set tcp keep alive mode if err is not NULL
//put err msg in err buffer
//RETURN: YANET_OK if ok, YANET_ERR if fail
int YanetKeepAlive(char* err, int fd);

//set tcp send/recv buffer length. if err is 
//not NULL, put err msg in err buffer.
//RETURN: YANET_OK if ok, YANET_ERR if fail
int YanetSetSendBuffer(char* err, int fd, int bufsize);
int YanetSetRecvBuffer(char* err, int fd, int bufsize);

//just like read(2), except
//read count bytes unless eof or error reached
//caller must make sure buffer is at least 
//count bytes long. 
//RETURN: bytes read if ok, -1 if fail. errno indicate error
int YanetReadN(int fd, char* buff, int count);

//just like write(2), except
//read count bytes unless error reached
//caller must make sure buffer is at least 
//count bytes long.
//RETURN: bytes write if ok, -1 if fail. errno indicate error
int YanetWriteN(int fd, char* buff, int count);

//accpet a client connection until INTR or a error
//happen. if ip/port is not NULL, put client ip and
//port in 'ip' and 'port'. if err is not NULL, put
//err msg in err buf.
//RETURN: client sockfd if ok, YANET_ERR if fail
int YanetAccept(char* err, int serversock, char* ip, int* port);

//Create a tcp server socket. if bindaddr is not NULL, bind
//on this addr, or else bind on *, if err is not NULL, put
//err msg in err buf.
//RETURN: serverfd if ok, YANET_ERR if fail
int YanetTcpServer(char* err, const char* bindaddr, int port);

//Create a tcp server socket. if bindaddr is not NULL, bind
//on this addr, or else bind on *, if err is not NULL, put
//err msg in err buf.
//RETURN: serverfd if ok, YANET_ERR if fail
int YanetUdpServer(char* err, const char* bindaddr, int port);

//Do a DNS query. caller must make sure ip buffer is longer
//enough to hold resulting ip address. if err is no NULL,put
//err msg in err buf.
//RETURN: YANET_OK if ok, YANET_ERR if fail
int YanetResolve(char* err, const char* host, char* ip);

//Tcp connect. blocking & nonblocking mode. if err is not NULL
//put err msg in err buf.
//RETURN: YANET_OK if ok, YANET_ERR if fail
int YanetConnect(char* err, const char* ip, int port);
int YanetNonBlockConnect(char* err, const char* ip, int port);

//Make a process become daemon proces. if nochdir is  zero,
//change the calling process's working directory to "/',
//if noclose is zero, close STDIN, STDOUT, STDERR.
//RETURN: YANET_OK if ok, YANET_ERR if fail
int YanetDaemonize(int nochdir, int noclose = 1);

} //namespace net
} //namespace yanetlib

#endif //net_common.h
