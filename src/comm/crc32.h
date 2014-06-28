#ifndef YANETLIB_COMM_CRC32_H
#define YANETLIB_COMM_CRC32_

#include <stdint.h>

namespace yanetlib {
namespace comm {

//open source code from:
//http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/bsd/libkern/crc32.c
//This is code is beautiful, so I dont want put it in a C++ class  wrapper...
//NOTE: CRC32 have at most six different implementation, with the different 
//plynomial. this one is the mostly used. which is the same with zip/gzip/..
//polynomial $edb88320
//PARAM: crc usuall choose 0
uint32_t
crc32(uint32_t crc, const void *buf, size_t size);

} //namespace comm
} //namespace yanetlib

#endif //crc32.h
