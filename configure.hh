#ifndef INCLUDE_CONFIGURE_HPP
#define INCLUDE_CONFIGURE_HPP

#include <map>
#include <string>

class Configure
{
public:
    Configure() : m_allow_empty_value(true) {}
    ~Configure() {}

    int getIntValue(const char* key, int default_val=-1);
    double getDoubleValue(const char* key, double default_val=-1.0);
    bool getBoolValue(const char* key, bool default_val=false);

    const std::string& getStrValue(const char* key, const std::string& default_val="");

    void checkConfigureFile(const char* configure_file);
    bool loadConfig(const char* configure_file);
private:
    bool parseLine(const char* line, std::string& key, std::string& value, std::string& error_info);

    void setAllowEmptyValue(bool allow) { m_allow_empty_value = allow; }

private:
    // simple key = value pair
    typedef std::map<std::string, std::string> ConfigMap;
    typedef ConfigMap::iterator ConfigMapIterator;
    typedef ConfigMap::const_iterator ConfigMapConstIterator;
    typedef std::pair<bool, ConfigMap> ConfigMapPair;

    ConfigMap m_configs;
    bool m_allow_empty_value;
    static char comment_delimiter;
};

#endif /* INCLUDE_CONFIGURE_HPP */
