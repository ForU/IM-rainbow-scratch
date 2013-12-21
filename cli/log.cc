#include "log.hh"
#include <string.h>
#include <muduo/extra/utils.h>

#define LOG_BUFFER_SIZE 1000
static struct level_info {
    enum e_level level;
    const char *str;
} levels[] = {
    { PR_LEVEL_TRACE, "TRACE" },
    { PR_LEVEL_INFO, "INFO " },
    { PR_LEVEL_DEBUG, "DEBUG" },
    { PR_LEVEL_WARN, "WARN " },
    { PR_LEVEL_ERROR, "ERROR" },
    { PR_LEVEL_FATAL, "FATAL" }
};

static const char* getLevel(enum e_level level)
{
    /* no check */
    return levels[level].str;
}

static const char *base_file(const char* file)
{
    if ( ! file || ! *file ) {
        return "";
    }
    const char* p_start = file + strlen(file);
    do {
        if ( *p_start == '/' )
            return p_start+1;
    } while( --p_start);
    return "";
}

void xlog(e_level level, const char* file, int line,  const char* func, const char *fmt, ...)
{
    char buf[LOG_BUFFER_SIZE];
    va_list args; va_start(args, fmt);
    vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);
    printf("%5d %s [%s] %s - %s:%d\n", (int)getTid(), getLevel(level), func, buf, base_file(file), line);
}
