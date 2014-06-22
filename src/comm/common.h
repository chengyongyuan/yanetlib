#ifndef YANETLIB_BASE_COMMON_H_
#define YANETLIB_BASE_COMMON_H_

//This files contains some most basic function and classes
//of all the proj. most of them can found in many C++ Proj.
//Some of them inspired by Goole Protocol Buffer src, POCO
//networking library.Of course we remove some platform 
//dependent implementations. Because We mainly focuse on
//Linux or Unix like systems.
//
//Author: colincheng 334089103@qq.com

#include <stdint.h>
#include <string>

namespace yanetlib {
namespace comm {

//use consistant typedef for primitive type
typedef unsigned int uint;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

//USEFULL MACROS...
template <bool>
struct CompileAssert {
};
#undef  COMPILE_ASSERT
#define COMPILE_ASSERT(expr, msg)       \
    typedef ::yanetlib::comm::CompileAssert<(bool(expr))>   \
        msg[bool(expr) ? 1 : -1]

#undef ARRAY_SIZE
#define ARRAY_SIZE(a)   \
    (sizeof(a) / sizeof(*a) / \
    static_cast<size_t>(!(sizeof(a) % sizeof(*a))))

#undef GOOGLE_DISALLOW_EVIL_CONSTRUCTORS
#define GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
    TypeName(const TypeName&);                         \
    void operator=(const TypeName&);                   \

#undef PREDICT_TRUE
#undef PREDICT_FALSE
#ifdef __GNUC__
#define PREDICT_TRUE(x)   __builtin_expect(!!(x), 1)
#define PREDICT_FALSE(x)  __builtin_expect(!!(x), 0)
#else
#define PREDICT_TRUE
#define PREDICT_FALSE
#endif

#undef ATTRIBUTE_ALWAYS_INLINE
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
//force inline gcc >= 3.1
#define ATTRIBUTE_ALWAYS_INLINE __attribute__ ((always_inline))
#else
#define ATTRIBUTE_ALWAYS_INLINE
#endif

//LOG FOR INNER USE. CURRENTLY SIMPLEY OUTPUT TO STRERR AND QUIT.
//STILL STEAL FROM GOOGLE CODE.
enum LogLevel {
    LOGLEVEL_INFO,
    LOGLEVEL_WARNNING,
    LOGLEVEL_ERROR,
    LOGLEVEL_FATAL,
};

class LogFinisher;

class LogMessage {
 public:
    LogMessage(LogLevel level, const char* filename, int line);
    ~LogMessage();

    //overloading for different type
    LogMessage& operator<<(const std::string& val);
    LogMessage& operator<<(const char *val);
    LogMessage& operator<<(char val);
    LogMessage& operator<<(int val);
    LogMessage& operator<<(uint val);
    LogMessage& operator<<(long val);
    LogMessage& operator<<(unsigned long val);
    LogMessage& operator<<(double val);

 private:
    friend class LogFinisher;
    void Finish();

    LogLevel level_;
    const char* filename_;
    int line_;
    std::string message_;
    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(LogMessage);
};

class LogFinisher {
 public:
     void operator=(LogMessage&);
};

typedef void LogHandler(LogLevel level, const char* filename, int line,
                        const std::string& message);

#undef COLIN_LOG_IF
#undef COLIN_LOG

#undef COLIN_CHECK
#undef COLIN_CHECK_EQ
#undef COLIN_CHECK_NE
#undef COLIN_CHECK_LT
#undef COLIN_CHECK_LE
#undef COLIN_CHECK_GT
#undef COLIN_CHECK_GE

#define COLIN_LOG(LEVEL)   \
    ::yanetlib::comm::LogFinisher() =                 \
      ::yanetlib::comm::LogMessage(                   \
        ::yanetlib::comm::LOGLEVEL_##LEVEL, __FILE__, __LINE__)
#define COLIN_LOG_IF(LEVEL, COND)   \
    (!COND) ? void(0) : COLIN_LOG(LEVEL)

#define COLIN_CHECK(EXPRESSION)   \
    COLIN_LOG_IF(FATAL, !(EXPRESSION)) << "CHECK FAILED:" #EXPRESSION ": "
#define COLIN_CHECK_EQ(A, B) COLIN_CHECK((A) == (B))
#define COLIN_CHECK_NE(A, B) COLIN_CHECK((A) != (B))
#define COLIN_CHECK_LT(A, B) COLIN_CHECK((A) <  (B))
#define COLIN_CHECK_LE(A, B) COLIN_CHECK((A) <= (B))
#define COLIN_CHECK_GT(A, B) COLIN_CHECK((A) >  (B))
#define COLIN_CHECK_GE(A, B) COLIN_CHECK((A) >= (B))



//Mutex wrapper class.
//Usually used with ScopedLock template class.
class Mutex {
 public:
     //Create a mutex not held by anyone.
     Mutex();

     //Destructor
    ~Mutex();

    //Block if necessary util the lock if free.
    void Lock();

    //Release the lock. caller must hold this lock.
    void UnLock();
 private:
    struct Internal;
    Internal* internal_;
    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Mutex);
};

//Not using mutex lock.
class NullMutex {
 public:
     NullMutex() {}

     ~NullMutex() {}

     //Do nothing.
     void Lock() {}

     //Do nothing.
     void UnLock() {}
 private:
    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(NullMutex);
};

template <typename M>
class ScopedLock {
 public:
    explicit ScopedLock(M* m) : m_(m) {
        m_->Lock();
    }

    ~ScopedLock() {
        m_->UnLock();
    }

 private:
    M*const m_;
    GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ScopedLock);
};

//ScopedPtr is a typical C++ RAII Object.
template <typename C>
class ScopedPtr {
 public:
     typedef C ElementType;

     //Constructor, default recv a null ptr.
     //The ptr must be created by new.
     explicit ScopedPtr(C* p = NULL) : ptr_(p) { }

     //Destructors.If there is ptr owned by this object.
     //Deleted it
     ~ScopedPtr() {
         delete ptr_;
     }

     //reset the underlying ownder object.
     //if we try to reset it to itselft. simply
     //ingore it.
     void reset(C* p = NULL) {
         if (p != ptr_) {
            delete ptr_;
            ptr_ = p;
         }
     }

     //Release a pointer.
     //release the ownership of the underlying object.
     //if this object hold a NULL pointer, this return value is NULL
     //After this operation, this object will hold a NULL pointer
     //the caller take the ownership of the unerlying object.
     C* release() {
         C* retVal = ptr_;
         ptr_ = NULL;
         return retVal;
     }

     C* get() const {
         return ptr_;
     }

     //Accessors to get the owned object.
     C& operator*() const{
         assert(ptr_ != NULL);
         return *ptr_;
     }
     C* operator->() const{
         assert(ptr_ != NULL);
         return ptr_;
     }

     //Comparion operators. 
     bool operator=(C* p) const { return ptr_ == p; } 
     bool operator!=(C* p) const { return ptr_ != p; }

 private:
     //owned object.
     C* ptr_;
    
     //Disable compare two ScopedPtr object.
     //Because If C2 != C2 it make no sense. If C2 == C,
     //it also make no sense, because you can never make
     //two ScopedPtr own one same ptr.
     template <typename C2> bool operator=(const ScopedPtr<C2>& p2) const;
     template <typename C2> bool operator!=(const ScopedPtr<C2>& p2) const;

     GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(ScopedPtr);
};

} //namespace comm
} //namespace yanetlib

#endif //common.h
