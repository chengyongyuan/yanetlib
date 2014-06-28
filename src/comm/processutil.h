#ifndef YANETLIB_COMM_PROCESSUTIL_H
#define YANETLIB_COMM_PROCESSUTIL_H

//This Files Contains Some Utilility Functions about Process.
//Author: sbtdkj1017@tom.com

#include <sys/stat.h>

namespace yanetlib {
namespace comm {

// Similar to system, but support vary aguments like printf.
// Max cmd length is 1M (enough hah)
bool MySystem(const char *format, ...);

}//namespace comm 
}//namespace yanetlib

#endif //processutil.h
