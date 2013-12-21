#ifndef INCLUDE_REGISTERAPP_HPP
#define INCLUDE_REGISTERAPP_HPP

#include "../app.hh"
#include <stdint.h>

class RegisterApp : public App
{
public:
    RegisterApp(const std::string& app_name="register");
    ~RegisterApp() {}

    virtual bool do_init(int argc, char *argv[]);
    virtual bool do_start();
    virtual bool do_stop();
    virtual bool do_finish();

private:
    uint16_t m_server_port;
    std::string m_server_ip;
    // tcpclient
};

#endif /* INCLUDE_REGISTERAPP_HPP */
