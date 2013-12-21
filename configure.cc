#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "log.hh"
#include "configure.hh"

char Configure::comment_delimiter = '#';

static void
int2str(std::string& rst_string, int int_val)
{
#define SIZE 12
    char buf[SIZE];
    memset(buf, sizeof buf, 0);
    snprintf(buf, sizeof buf, "%d", int_val);
    rst_string = buf;
}


void Configure::checkConfigureFile(const char* configure_file)
{
    if ( ! configure_file ) {
        PR_ERROR("invalid argument, configure file is NULL");
        return;
    }
    FILE *fd;
    fd = fopen(configure_file, "r");
    if ( ! fd ) {
        PR_ERROR("failed to call fopen on %s, errno=%d",
                 configure_file, errno);
        return;
    }
    // OK now
#ifndef LINE_SIZE_MAX
#define LINE_SIZE_MAX 1024
#endif
    int cur_linum = 1;
    char line[LINE_SIZE_MAX];
    memset(line, sizeof line, '0');
    while ( NULL != fgets(line, sizeof line, fd) ) {
        PR_TRACE("parsing line=%d", cur_linum);
        std::string key, value, error_info;
        if (! parseLine(line, key, value, error_info)) {
            PR_ERROR("failed to parse line=%d, for \"%s\"",
                     cur_linum, error_info.c_str());
        }
        ++cur_linum;
        memset(line, sizeof line, '0');
    }
    fclose(fd), fd = NULL;
}

bool Configure::loadConfig(const char* configure_file)
{
    if ( ! configure_file ) {
        PR_ERROR("invalid argument, configure file is NULL");
        return false;
    }
    FILE *fd;
    fd = fopen(configure_file, "r");
    if ( ! fd ) {
        PR_ERROR("failed to call fopen on %s, errno=%d",
                 configure_file, errno);
        return false;
    }
    // OK now
#ifndef LINE_SIZE_MAX
#define LINE_SIZE_MAX 1024
#endif
    int rc = true;
    int cur_linum = 1;
    char line[LINE_SIZE_MAX];
    memset(line, sizeof line, '0');
    while ( NULL != fgets(line, sizeof line, fd) ) {
        PR_TRACE("parsing line=%d", cur_linum);
        std::string key, value, error_info;
        if (! parseLine(line, key, value, error_info)) {
            PR_ERROR("failed to parse line=%d, for \"%s\"",
                     cur_linum, error_info.c_str());
            rc = false;
            break;
        }
        PR_TRACE("to insert pair:key=[%s], value=[%s]",
                 key.c_str(), value.c_str());
        m_configs.insert(std::make_pair(key, value));
        ++cur_linum;
        memset(line, sizeof line, '0');
    }
    fclose(fd), fd = NULL;
    return rc;
}

static bool isWhiteSpace(char c)
{
    return (' ' == c || '\t' == c || '\n' == c || '\r' == c);
}

static bool isKeyChar(char c)
{
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            (c == '_'));
}

bool Configure::parseLine(const char* line, std::string& key, std::string& value, std::string& error_info)
{
    if ( !line ) {
        PR_ERROR("invalid argument, line is NULL");
        return false;
    }

    // [ \t]*key[ \t]*=[ \t]*value[ \t]*\n
    // [ \t]*# comment delimiter can be anywhere

    int key_len, value_len;
    const char* key_base;
    const char *value_base;
    const char* cur_pos = line;
    std::string err_pos_str;

    // skip leading white space
    while ( *cur_pos && isWhiteSpace(*cur_pos) )
        ++cur_pos;

    // expecting comment char:'#' or normal ASCII character
    if ( comment_delimiter == *cur_pos ) {
        return true;
    }
    // now, key expected
    key_base = cur_pos;
    while ( *cur_pos && isKeyChar(*cur_pos) )
        ++cur_pos;
    if ((key_len = (cur_pos - key_base)) <= 0) {
        ::int2str(err_pos_str, cur_pos-line);
        error_info="no key found, at position:" + err_pos_str;
        return false;
    }
    key.assign(key_base, key_len);
    PR_TRACE("key=%s", key.c_str());

    // then,  white space, comment_delimiter, '=' expected
    while ( *cur_pos && (isWhiteSpace(*cur_pos) ||
                         comment_delimiter == *cur_pos)) {
        if ( comment_delimiter == *cur_pos ) {
            ::int2str(err_pos_str, cur_pos-line);
            error_info = "hit comment, only key given, at position:" + err_pos_str;
            return false;
        }
        ++cur_pos;
    }
    // now, '=' expected
    if ( '=' != *cur_pos ) {
        ::int2str(err_pos_str, cur_pos-line);
        error_info = "= expected at position:" + err_pos_str;
        return false;
    }
    // head forward for '='
    ++cur_pos;

    ////////////////////////////////////////////////////////////////
    // following content is after '=', so if no value is given, we
    // treat position:it as valid
    ////////////////////////////////////////////////////////////////
    // now, white space expected
    while ( *cur_pos && isWhiteSpace(*cur_pos) )
        ++cur_pos;
    // now, value and comment_delimiter expected
    value_base = cur_pos;
    while ( *cur_pos || comment_delimiter == *cur_pos ) {
        if ( comment_delimiter == *cur_pos ) {
            PR_WARN("hit comment");
            break;
        }
        // white-space
        if ( isWhiteSpace(*cur_pos)) {
            PR_TRACE("hit white space in value");
            break;
        }
        // '"' special handle
        const char* quote_end_pos = NULL;
        const char* comment_pos = NULL;
        if ( '"' == *cur_pos ) {
            ++cur_pos;          // skip '"'
            value_base = cur_pos; // reset value_base

            comment_pos = strchr(cur_pos, comment_delimiter);
            quote_end_pos = strchr(cur_pos, '"');
            // no end quotes, error
            if ( ! quote_end_pos ) {
                ::int2str(err_pos_str, cur_pos-line);
                error_info = "invalid value starting with \" has no ending \", at position:" + err_pos_str;
                return false;
            } else {
                // has end quotes, check if comment delimiter in quotes
                if ( comment_pos && comment_pos < quote_end_pos) {
                    ::int2str(err_pos_str, comment_pos-line);
                    error_info = "comment delimiter between quotes, at position:" + err_pos_str;
                    return false;
                }
                cur_pos = quote_end_pos;
                break;
            }

            if ( ! quote_end_pos ) {
                ::int2str(err_pos_str, cur_pos-line);
                error_info = "invalid value starting with \" has no ending \", at position:" + err_pos_str;
                return false;
            } else {
                cur_pos = quote_end_pos;
                break;
            }
        }
        // white space
        ++cur_pos;
    }
    // now, value end
    if ( (value_len = (cur_pos - value_base)) <= 0 ) {
        ::int2str(err_pos_str, cur_pos-line);
        error_info = "empty content after '=', at position:" + err_pos_str;
        return m_allow_empty_value;
    }
    value.assign(value_base, value_len);
    return true;
}

const std::string& Configure::getStrValue(const char* key, const std::string& default_val)
{
    ConfigMapConstIterator it = m_configs.find(key);
    if ( it == m_configs.end() ) {
        return default_val;
    }
    return it->second;
}

int Configure::getIntValue(const char* key, int default_val)
{
    ConfigMapConstIterator it = m_configs.find(key);
    if ( it == m_configs.end() ) {
        return default_val;
    }
    return atoi(it->second.c_str());
}

double Configure::getDoubleValue(const char* key, double default_val)
{
    ConfigMapConstIterator it = m_configs.find(key);
    if ( it == m_configs.end() ) {
        return default_val;
    }
    return strtod(it->second.c_str(), NULL);
}

bool Configure::getBoolValue(const char* key, bool default_val)
{
    ConfigMapConstIterator it = m_configs.find(key);
    if ( it == m_configs.end() ) {
        return default_val;
    }
    // anything not false(case insensitive) or 0 is true
    return ( 0 != strcasecmp(it->second.c_str(), "false") && it->second != "0");
}
