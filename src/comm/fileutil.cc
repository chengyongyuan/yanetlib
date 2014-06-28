#include <unistd.h>
#include <comm/fileutil.h>

namespace yanetlib {
namespace comm {

bool IsFileExist(const char *file_path)
{
    if (file_path)
        return access(file_path, F_OK) == 0;
    else
        return false;
}

} //namespace comm
} //namespace yanetlib
