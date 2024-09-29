#pragma once 

#include "defines.h"

#define LOG_WARNING_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

#if KRELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

enum log_level{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
};
bool intialize_logging();
void shutdown_logging();

EXP void log_output(log_level level,const char*message,...);

#define DFATAL(message,...) log_output(LOG_LEVEL_FATAL,message,##__VA_ARGS__);

#define DERROR(message,...) log_output(LOG_LEVEL_ERROR,message,##__VA_ARGS__);

#if LOG_WARNING_ENABLED == 1
#define DWARNING(message,...) log_output(LOG_LEVEL_WARNING,message,##__VA_ARGS__);
#else
#define DWARNING(message,...)
#endif

#if LOG_INFO_ENABLED == 1
#define DINFO(message,...) log_output(LOG_LEVEL_INFO,message,##__VA_ARGS__);
#else
#define DINFO(message,...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define DDEBUG(message,...) log_output(LOG_LEVEL_DEBUG,message,##__VA_ARGS__);
#else
#define DDEBUG(message,...)
#endif

#if LOG_TRACE_ENABLED == 1
#define DTRACE(message,...) log_output(LOG_LEVEL_TRACE,message,##__VA_ARGS__);
#else
#define DTRACE(message,...)
#endif



