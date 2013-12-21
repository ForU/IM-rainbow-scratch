#ifndef INCLUDE_WORKING_CONTEXT_HPP
#define INCLUDE_WORKING_CONTEXT_HPP

#include <muduo/base/Singleton.h>
#include "user_manager.hh"

class ServerWorkingContext
{
public:
    ServerWorkingContext() {}
    ServerWorkingContext& instance() {
        return muduo::Singleton<ServerWorkingContext>::instance();
    }
    virtual ~ServerWorkingContext();
    UserManager& getUserManager() {
        return muduo::Singleton<UserManager>::instance();
    }
private:
    std::string m_configure_file;
    // to add
};
#endif /* INCLUDE_WORKING_CONTEXT_HPP */
