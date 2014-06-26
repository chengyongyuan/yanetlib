#ifndef YANETLIB_COMM_DATETIME_H
#define YANETLIB_COMM_DATETIME_H

//The file contains the utilty class DateTime.
//this class is a thin wrapper class. It contains
//many usefull time related function. eg.
//GetTimeStr:
//GetDateStr:
//GetYear:
//GetMonth
//GetDay:
//...

#include <unistd.h>
#include <time.h>
#include <string>

namespace yanetlib {
namespace comm {

//NOTE:This class is not thread-safe.
class DateTime {
 public:
     //Get full date string representation of time t
     static std::string GetDateStr(time_t t);

     //GetDateStr(now());
     static std::string GetDateStr();

     //Get full time string representation of time t
     static std::string GetTimeStr(time_t t);

     //GetTimeStr(now())
     static std::string GetTimeStr();

     //Give a time, return the year string
     static std::string GetYear(time_t t);

     static std::string GetYear();

     //Give a time, return the month string
     static std::string GetMonth(time_t t);

     static std::string GetMonth();

     //Give a time return the day string
     static std::string GetDay(time_t t);

     static std::string GetDay();

     //Give a time return the hour string
     static std::string GetHour(time_t t);

     static std::string GetHour();
 private:
    DateTime(const DateTime&);
    void operator=(const DateTime&);
};

} //namespace comm
} //namespace yanetlib
#endif //datetime.h
