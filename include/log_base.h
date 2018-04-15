#pragma once


#include <functional>
#include <iomanip>
#include <ctime>
#include <thread>
#include <sstream>
#include <cstring>

using LogFuc = std::function<void(int level, const char* msg)>;

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? std::strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define LOG_FORMAT_MSG(logfuc,level,modulename,msg) \
if(nullptr != logfuc)\
{\
    std::stringstream oss;\
    std::time_t t = std::time(nullptr);\
    std::tm tm = *std::localtime(&t);\
    oss<< std::put_time(&tm,"[%F %T] ");\
    oss<< "[" << std::this_thread::get_id()<<"] "; \
    oss<< "[" <<modulename<<"] "; \
    oss<< "[" << __FILENAME__ <<":" << __LINE__ <<"] "; \
    oss<<msg<<std::endl;\
    logfuc(level, oss.str().c_str()); \
}
