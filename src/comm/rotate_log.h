#ifndef YANETLIB_COMM_ROTATELOG_H
#define YANETLIB_COMM_ROTATELOG_H

#include <string>

//This file implement a simple rotating log
//Currently Support Rotate By: SIZE | DAY | HOUR
//Currently not thread safe -:

namespace yanetlib {
namespace comm {

#define LOG(log, level, fmt, args...) do {                 \
    if (level >= log.Level()) {                        \
        log.Log(RotateLog::DATESTR_LONG, "[%s:%d]"fmt,   \
                __FILE__, __LINE__ , ##args);          \
    }                                                  \
} while(0)

#define LOG_ERROR(log, fmt, args...) LOG(log, RotateLog::ERROR, fmt, ##args)
#define LOG_WARN(log, fmt, args...) LOG(log, RotateLog::WARN, fmt, ##args)
#define LOG_INFO(log, fmt, args...) LOG(log, RotateLog::INFO, fmt, ##args)
#define LOG_DEBUG(log, fmt, args...) LOG(log, RotateLog::DEBUG, fmt, ##args)

class RotateLog {
 public:

     enum LogLevel {
         ERROR = 1,
         WARN  = 2,
         INFO  = 3,
         DEBUG = 4,
     };

     enum RotateType {
         ROTATE_BY_SIZE,  //rotate by file size
         ROTATE_BY_DAY,   //rotate by day
         ROTATE_BY_HOUR,  //rotate by hour
     };

     enum DateFormat {
         DATESTR_SHORT,
         DATESTR_LONG,
     };

     //construtor
     RotateLog();

     //Destructor
     ~RotateLog();

     //Init the log
     //name is absolute path of log file.(start with '/')
     //rtype specify RotateType: ROTATE_BY_SIZE, 
     //ROTATE_BY_DAY, ROTATE_BY_HOUR
     //if the log is not rotate size, then maxsize simply 
     //has no effect.
     bool Init(LogLevel level, const std::string& name, RotateType rtype, 
               int maxsize, int maxcnt = 30);

     //Log to file. first three parm ususally wrap by
     void Log(DateFormat dformat, const char* fmt, ...);

     LogLevel Level() const { return loglevel_; }
 private:
     //disable copy
     RotateLog(const RotateLog&);
     void operator=(const RotateLog&);

     //roate the low file!
     void DoRoate();

     //log level
     LogLevel loglevel_;
     //base log name not have .log
     std::string log_basename_;
     //log full name *.log
     std::string log_fullname_;
     //rotate type
     RotateType  rtype_;
     //max file size
     int max_filesz_;
     //max log filecountu
     int max_file_cnt_;
     //last rotate time
     time_t last_rotate_time_;
     //log file handle
     FILE* fp_;
};

} // namespace comm
} // namespace yanetlib
#endif //rotate_log.h
