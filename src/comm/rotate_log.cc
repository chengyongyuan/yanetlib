#include <sys/time.h>
#include <sys/types.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "rotate_log.h"
#include "datetime.h"

namespace yanetlib {
namespace comm {

RotateLog::RotateLog() : loglevel_(ERROR),fp_(NULL){ }

RotateLog::~RotateLog() {
    if (fp_ != NULL) {
        fclose(fp_);
        fp_ = NULL;
    }
}

bool RotateLog::Init(LogLevel level, const std::string& name, 
                     RotateType rtype, int maxsize, int maxcnt) {
    char logname[512];
    loglevel_ = level;
    max_file_cnt_ = maxcnt;
    max_filesz_ = maxsize;
    rtype_ = rtype;
    log_basename_ = name;
    last_rotate_time_ = time(NULL);
    snprintf(logname, sizeof(logname), "%s.log", log_basename_.c_str());
    log_fullname_ = logname; 

    DoRoate(); 
    //try to open log file
    if ((fp_ = fopen(log_fullname_.c_str(), "a+")) == NULL) {
        return false;
    }

    return true; 
}

void RotateLog::DoRoate() {
    char oldname[512], newname[512];
    struct stat file_stat;
    time_t last_rot_time = last_rotate_time_;
    struct tm last_rot_tm, last_mod_tm;

    if (stat(log_fullname_.c_str(), &file_stat) < 0) return ;
    switch(rtype_) {
        case ROTATE_BY_DAY:
            if (last_rot_time - file_stat.st_mtime > 86400) break;
            memcpy(&last_rot_tm, localtime(&last_rot_time), sizeof(last_rot_tm));
            memcpy(&last_mod_tm, localtime(&file_stat.st_mtime), sizeof(last_mod_tm));
            if (last_rot_tm.tm_mday == last_mod_tm.tm_mday) return ;
            break;
        case ROTATE_BY_HOUR:
            if (last_rot_time - file_stat.st_mtime > 3600) break;
            memcpy(&last_rot_tm, localtime(&last_rot_time), sizeof(last_rot_tm));
            memcpy(&last_mod_tm, localtime(&file_stat.st_mtime), sizeof(last_mod_tm));
            if (last_rot_tm.tm_hour == last_mod_tm.tm_hour) return ;
            break;
        case ROTATE_BY_SIZE:
            if (file_stat.st_size < max_filesz_) return ;
            break;
    }

    //close it first
    if (fp_ != NULL) fclose(fp_);
    fp_ = NULL;

    //begin rotate
    for (int i = max_file_cnt_-2; i >=0; --i) {
        if (i == 0)
            snprintf(oldname, sizeof(oldname), "%s.log", log_basename_.c_str());
        else
            snprintf(oldname, sizeof(oldname), "%s%d.log", log_basename_.c_str(), i);

        if (access(oldname, F_OK) == 0) {
            snprintf(newname, sizeof(newname), "%s%d.log", log_basename_.c_str(), i+1);
            if (rename(oldname, newname) < 0) return ;
        }
    }
    last_rotate_time_ = time(NULL);
    //open newest log
    if ((fp_ = fopen(log_fullname_.c_str(), "a+")) == NULL) {
        fp_ = NULL;
        return ;
    }
}

void RotateLog::Log(DateFormat dformat, const char* fmt, ...) {
    va_list ap;
    struct timeval log_tv;

    //file not open, Roate fail may set fp == NULL
    //we try to open here
    if (fp_ == NULL && (fp_ = fopen(log_fullname_.c_str(), "a+")) == NULL) {
        fp_ = NULL;
        return ;
    }
    if (dformat == DATESTR_SHORT) {
        fprintf(fp_, "[%s]", DateTime::GetDateStr().c_str());
    } else if (dformat == DATESTR_LONG) {
        gettimeofday(&log_tv, NULL);
        fprintf(fp_, "[%s.%6u]", DateTime::GetDateStr(log_tv.tv_sec).c_str(),
                (unsigned int)log_tv.tv_usec);
    }
    va_start(ap, fmt);
    vfprintf(fp_, fmt, ap);
    fprintf(fp_, "\n");
    va_end(ap);

    DoRoate(); 
}

} // namespace comm
} // namespace yanetlib
