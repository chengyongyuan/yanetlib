#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <comm/common.h>

namespace yanetlib {
namespace comm {

//empty callback function
void DoNothing() { }

void DefaultLogHandler(LogLevel level, const char* filename,
        int line, const std::string& message) {
    static const char* level_names[] = {"INFO", "WARNNING", "ERROR", "FATAL"};

    fprintf(stderr, "[YANETLIB %s:%s:%d] %s\n",
            level_names[level], filename, line, message.c_str());
    fflush(stderr);
}

void NullLogHandler(LogLevel level, const char* filename,
        int line, const std::string& message) {
    //Do nothing.
}

static LogHandler* log_handler_ = &DefaultLogHandler;
LogMessage::LogMessage(LogLevel level, const char* filename, int line) 
    : level_(level), filename_(filename), line_(line) { }

LogMessage::~LogMessage() { }

LogMessage& LogMessage::operator<<(const std::string& message) {
    message_ += message;
    return *this;
}

LogMessage& LogMessage::operator<<(const char* message) {
    message_ += message;
    return *this;
}

#undef  DEFINE_STREAM_OPERATOR
#define DEFINE_STREAM_OPERATOR(TYPE, FORMAT)            \
    LogMessage& LogMessage::operator<<(TYPE value) {    \
        /*128 bytes is far enough for inner type.*/     \
        char tmpbuf[128];                               \
        snprintf(tmpbuf, sizeof(tmpbuf), FORMAT, value);\
        message_ += tmpbuf;                             \
        return *this;                                   \
    }
DEFINE_STREAM_OPERATOR(char,            "%c" )
DEFINE_STREAM_OPERATOR(int,             "%d" )
DEFINE_STREAM_OPERATOR(uint,            "%u" )
DEFINE_STREAM_OPERATOR(long,            "%ld")
DEFINE_STREAM_OPERATOR(unsigned long,   "%lu")
DEFINE_STREAM_OPERATOR(double,          "%g" )

void LogMessage::Finish() {
    log_handler_(level_, filename_, line_, message_);

    //abort or throw exceptions?
    //FATAL ERROR, we now just abort for safety
    if (level_ == LOGLEVEL_FATAL) {
        abort();
    }
}

void LogFinisher::operator=(LogMessage& msg) {
    msg.Finish();
}

//Mutex inner struct.
struct Mutex::Internal {
    pthread_mutex_t mutex;
};

Mutex::Mutex() : internal_(new Internal) {
    pthread_mutex_init(&internal_->mutex, NULL);
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&internal_->mutex);
    delete internal_;
}

void Mutex::Lock() {
    int ret = pthread_mutex_lock(&internal_->mutex);
    //TODO: CHANGE THIS
    assert(ret == 0);
}

void Mutex::UnLock() {
    int ret = pthread_mutex_unlock(&internal_->mutex);
    //TODO: CHANGE THIS
    assert(ret == 0);
}

pthread_mutex_t* Mutex::GetRaw() {
    return &internal_->mutex;
}

} //namespace comm
} //namespace yanetlib
