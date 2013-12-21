#ifndef INCLUDE_LOG_HPP
#define INCLUDE_LOG_HPP

#include <stdarg.h>             /* for vsnprintf, va_list, va_start */
#include <stdio.h>              /* for printf */
#include <pthread.h>

enum e_level
{
    PR_LEVEL_TRACE,
    PR_LEVEL_INFO,
    PR_LEVEL_DEBUG,
    PR_LEVEL_WARN,
    PR_LEVEL_ERROR,
    PR_LEVEL_FATAL
};

extern void xlog(e_level level, const char* file, int line,  const char* func, const char *fmt, ...);

#define PR_TRACE(fmt, ...) xlog(PR_LEVEL_TRACE, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define PR_INFO(fmt, ...)  xlog(PR_LEVEL_INFO,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define PR_DEBUG(fmt, ...) xlog(PR_LEVEL_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define PR_WARN(fmt, ...)  xlog(PR_LEVEL_WARN,  __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define PR_ERROR(fmt, ...) xlog(PR_LEVEL_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define PR_FATAL(fmt, ...) xlog(PR_LEVEL_FATAL, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#endif /* INCLUDE_LOG_HPP */
