#ifndef YANETLIB_COMM_FILEUTIL_H
#define YANETLIB_COMM_FILEUTIL_H

//This Files Contains Some Disk/File Utilility Functions.
//Author: sbtdkj1017@tom.com
//

#include <sys/stat.h>

namespace yanetlib {
namespace comm {

// Test if a file/directory exists.
// If file_path is NULL or error occured when accessing file, false will be returned
bool IsFileExist(const char *file_path);

}//namespace comm 
}//namespace yanetlib

#endif //fileutil.h
