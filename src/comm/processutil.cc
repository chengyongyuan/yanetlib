extern "C" {
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
}
#include <comm/processutil.h>

namespace yanetlib {
namespace comm {

bool MySystem(const char *format, ...)
{
    if (format == NULL)
        return false;
    char _buf[512]; // default buf if cmd is not too long
    char *buf = _buf;
    va_list va;
    va_start(va, format);
    if (vsnprintf(_buf, sizeof(_buf), format, va) >= (int)sizeof(_buf)) {
        // _buf is too small, so alloc 1M heap mem and try again
        va_start(va, format);
        buf = new char[1024 * 1024];
        if (vsnprintf(_buf, sizeof(_buf), format, va) >= 1024 * 1024) {
            va_end(va);
            delete [] buf;
            return false;
        }
    }
    va_end(va);

    sighandler_t old_handler = signal(SIGCHLD, SIG_DFL); 
    system(buf); 
    signal(SIGCHLD, old_handler); 
    if (buf != _buf)
        delete [] buf;
    return true;
}

} //namespace comm
} //namespace yanetlib
