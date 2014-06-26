/* MD5.H - header file for MD5C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.
These notices must be retained in any copies of any part of this
documentation and/or software.
 */

/* GLOBAL.H - RSAREF types and constants
 */

/* PROTOTYPES should be set to one if and only if the compiler supports
  function argument prototyping.
The following makes PROTOTYPES default to 0 if it has not already
  been defined with C compiler flags.
 */
//Personally, I think this file is borrowed from openssl implementation.

#ifndef YANETLIB_COMM_MD5_H
#define YANETLIB_COMM_MD5_H

#include <stdint.h>

namespace yanetlib {
namespace comm {


//Thin wrapper class of C functions.
class Md5 {
 public:
    /* POINTER defines a generic pointer type */
    typedef unsigned char* POINTER;

    /* UINT2 defines a two byte word */
    typedef unsigned short int UINT2;

    /* UINT4 defines a four byte word */
    typedef uint32_t UINT4;

    /* MD5 context. */
    typedef struct {
      UINT4 state[4];                                   /* state (ABCD) */
      UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
      unsigned char buffer[64];                         /* input buffer */
    } MD5_CTX;

    //public interface
    //given a input bufer, generate md5 byte string. put it in outbuffer
    static void Md5HashBuffer( char *outBuffer, const char *inBuffer, int length);

    //generate human readable md5string. if you are lasy, you can
    //just like outout of shell "md5sum" cmd
    static std::string Md5HashString(const char* inBuffer, int length);
 private:
    //disable copy
    Md5(const Md5&);
    void operator=(const Md5&);

    static void MD5Init (MD5_CTX *);
    static void MD5Update (MD5_CTX *, unsigned char *, unsigned int);
    static void MD5Final (unsigned char [16], MD5_CTX *);
    static void MD5Transform(UINT4 [4], unsigned char [64]);
    static void Encode(unsigned char *, UINT4 *, unsigned int);
    static void Decode(UINT4 *, unsigned char *, unsigned int);
    static void MD5_memcpy(POINTER, POINTER, unsigned int);
    static void MD5_memset(POINTER, int, unsigned int);

};

}
}
#endif //md5.h
