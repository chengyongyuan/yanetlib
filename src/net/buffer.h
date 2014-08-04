#ifndef YANETLIB_NET_BUFFER_H
#define YANETLIB_NET_BUFFER_H

//all network program need some sort of buffer,
//here we implemnt a simple one. more complex one
//include freelist / buffer bool etc.
//

namespace yanetlib {
namespace net {
    
class Buffer {
 public:
     //default 64K buffer
     Buffer(int sz = 65536);
     
     ~Buffer();

     bool Init();

     char* DataPtr() {
         return _buf + _rpos;
     }

     int   DataSize() {
         return _wpos - _rpos;
     }

     char* ReadPtr() {
         return _buf + _wpos;
     }

     int   LeftSize() {
         return _sz - _wpos;
     }

     void Push(int sz) {
         _wpos += sz;
         //can not large than max buf size
         if (_wpos > _sz)
             _wpos = _sz;
     }

     void Pull(int sz) {
         _rpos += sz;
         //rset
         if (_rpos == _wpos) {
             _rpos = _wpos = 0;
         }
     }

     bool Extend();
 private:
     //disable evil
     Buffer(const Buffer&);
     void operator=(const Buffer&);

 private:
     char* _buf;
     //max size of buffer
     int  _sz;
     //buffer read pos
     int  _rpos;
     //buffer write pos
     int  _wpos;
};

} // net
} // yanetlib
#endif //buffer.h
