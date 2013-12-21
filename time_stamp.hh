#ifndef INCLUDE_TIME_STAMP_HPP
#define INCLUDE_TIME_STAMP_HPP


#include <muduo/base/Logging.h>

#include <time.h>
#include <stdio.h>
#include <string>

#define DEFAULT_TIME "1900-0-0 0:0:0"
#define BASE_YEAR (1900)

class Time
{
public:
    Time(const std::string& time_str=DEFAULT_TIME)
        : m_time_str(time_str) {}
    ~Time() {}

    void setTimeStr(const std::string& time_str) {
        m_time_str = time_str;
    }
    // format: mon-date h:m
    const std::string& getTimeStr() {
        return m_time_str;
    }

    const std::string& getNowTime() {
        time_t seconds_since_Epoch;
        if ( (time_t)-1 == time(&seconds_since_Epoch) ) {
            LOG_ERROR << "failed to call time";
            m_time_str = DEFAULT_TIME;
            return m_time_str;
        }
        struct tm* p_tm = localtime(&seconds_since_Epoch);
        char buf[100];
        // snprintf(buf, sizeof buf, "%2d-%02d %02d:%02d",
        //          p_tm->tm_mon, p_tm->tm_mday,
        //          p_tm->tm_hour, p_tm->tm_min);
        snprintf(buf, sizeof buf, "%02d:%02d:%02d",
                 p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);

        m_time_str = buf;
        return m_time_str;
    }

private:
    std::string m_time_str;
};

#endif /* INCLUDE_TIME_STAMP_HPP */
