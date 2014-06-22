#include <stdio.h>
#include <comm/datetime.h>

namespace yanetlib {
namespace comm {

using std::string;

string DateTime::GetDateStr(time_t t) {
    static char tmpbuf[64];
    
    struct tm* ptm = localtime(&t);
    if (ptm == NULL) return "";
    sprintf(tmpbuf, "%04d-%02d-%2d %02d:%02d:%02d",
            ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday,
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    return tmpbuf;
}

string DateTime::GetDateStr() {
    return GetDateStr(time(NULL));
}

string DateTime::GetTimeStr(time_t t) {
    static char tmpbuf[64];
    
    struct tm* ptm = localtime(&t);
    if (ptm == NULL) return "";
    sprintf(tmpbuf, "%02d:%02d:%02d",
            ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    return tmpbuf;
}

string DateTime::GetTimeStr() {
    return GetTimeStr(time(NULL));
}

string DateTime::GetYear(time_t t) {
    static char tmpbuf[32];

    struct tm* ptm = localtime(&t);
    if (ptm == NULL) return "";
    sprintf(tmpbuf, "%04d", ptm->tm_year+1900);

    return tmpbuf;
}

string DateTime::GetYear() {
    return GetYear(time(NULL));
}

string DateTime::GetMonth(time_t t) {
    static char tmpbuf[32];

    struct tm* ptm = localtime(&t);
    if (ptm == NULL) return "";
    sprintf(tmpbuf, "%02d", ptm->tm_mon+1);

    return tmpbuf;
}

string DateTime::GetMonth() {
    return GetMonth(time(NULL));
}

string DateTime::GetDay(time_t t) {
    static char tmpbuf[32];

    struct tm* ptm = localtime(&t);
    if (ptm == NULL) return "";
    sprintf(tmpbuf, "%02d", ptm->tm_mday);

    return tmpbuf;
}

string DateTime::GetDay() {
    return GetDay(time(NULL));
}

string DateTime::GetHour(time_t t) {
    static char tmpbuf[32];

    struct tm* ptm = localtime(&t);
    if (ptm == NULL) return "";
    sprintf(tmpbuf, "%02d", ptm->tm_hour);

    return tmpbuf;
}

string DateTime::GetHour() {
    return GetHour(time(NULL));
}

} //namespace comm
} //namespace yanetlib
