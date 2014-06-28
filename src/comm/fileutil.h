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

// Test if a path is a directory.
// If error occured, false will be returned
bool IsDirectory(const char *path);

// Get file size.
// If error occur (e.g. file does not exist), 0 will be returned.
// If file_path specifies a directory, size of directory recursively will be returned
unsigned long GetFileSize(const char *file_path);

// Get file base name. Just return the part after the last slash "/"
// Similar to basename but not modify content of file_path
// e.g.: "/data/log.log" -> "log.log" ; "log.log" -> "log.log"
//       "/data" -> "data" ; "/" -> "" ; "" -> "" ; NULL -> NULL
const char *GetBaseName(const char *file_path);

}//namespace comm 
}//namespace yanetlib

#endif //fileutil.h
