extern "C" {
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
}
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

bool IsDirectory(const char *path)
{
    struct stat st;
    if (path && !stat(path, &st))
        return S_ISDIR(st.st_mode);
    return false;
}

unsigned long GetFileSize(const char *file_path)
{
    if (file_path == NULL)
        return 0;

    if (IsDirectory(file_path)) {
        char cmd_buf[PATH_MAX + 10]; // add 10 for "du -sb"
        unsigned long dir_size = 0;
        snprintf(cmd_buf, sizeof(cmd_buf), "du -sb \"%s\"", file_path);
        FILE *fp = popen(cmd_buf, "r");
        if (fp == NULL)
            return 0;
        fscanf(fp, "%lu", &dir_size);
        pclose(fp);
        return dir_size;
    }

    struct stat st;
    if (lstat(file_path, &st) == 0)
        return st.st_size;
    else
        return 0;
}

const char *GetBaseName(const char *file_path)
{
    if (file_path == NULL)
        return NULL;
    const char *p = file_path + strlen(file_path);
    while (p != file_path) {
        if (*--p == '/')
            return p + 1;
    }
    return file_path;
}

} //namespace comm
} //namespace yanetlib
