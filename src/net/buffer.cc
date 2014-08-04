#include <stdlib.h>
#include <assert.h>
#include "buffer.h"

namespace yanetlib {
namespace net {
    
Buffer::Buffer(int sz) : _buf(NULL), _sz(sz), _rpos(0),
    _wpos(0){
    _buf = (char *)malloc(sizeof(char) * _sz);
    assert(_buf != NULL);
}

bool Buffer::Init() {
    _buf = (char *)malloc(sizeof(char) * _sz);
    if (_buf == NULL) return false;
    return true;
}

Buffer::~Buffer() {
    if (_buf != NULL) free(_buf);
    _rpos = _wpos = 0;
    _buf = NULL;
}

bool Buffer::Extend() {
    char* newbuf = (char *)realloc(_buf, _sz<<1);
    if (newbuf == NULL) return false;
    _buf = newbuf;
    _sz  = _sz << 1;
    return true;
}

} // net
} //yanetlib
