#ifndef INCLUDE_SERVER_APP_HPP
#define INCLUDE_SERVER_APP_HOP

#include "app.hh"
#include "server_core.hh"

#include <vector>
#include <pthread.h>

using namespace muduo;
using namespace muduo::net;

class RainbowServerApp: public App
{
public:
    RainbowServerApp(const InetAddress& server_addr,
                     const std::string& app_name="rainbow server");
    virtual ~RainbowServerApp() {}

protected:
    bool do_init(int argc, char *argv[]);
    bool do_start();
    bool do_stop();
    bool do_finish();

private:
    // use static to avoid the default "this" argument
    static void* processThreadLoop(void* args);

private:
    ServerCore m_server_core;
    std::vector<pthread_t> m_process_threads;
    EventLoop m_loop;
};

#endif /* INCLUDE_SERVER_APP_HPP */
