#include <muduo/base/Logging.h>
#include "server_app.hh"
#include "packet_manager.hh"

using namespace muduo;
using namespace muduo::net;

RainbowServerApp::RainbowServerApp(const InetAddress& server_addr,
                                   const std::string& app_name)
    : App(app_name), m_server_core(&m_loop, server_addr)
{
    LOG_TRACE << "constructing server=" << app_name;
}

bool RainbowServerApp::do_init(int argc, char *argv[])
{
    LOG_TRACE << "to load server configure";
    // init user manager
    // TODO: [2013-12-08] the right method is to load data from database
    LOG_TRACE << "to initialize user manager";
    G_user_manager.init();
    LOG_TRACE << "to initialize server core";
    m_server_core.init();
    return true;
}

bool RainbowServerApp::do_start()
{
    // TODO: [2013-12-08] load from configure;
    LOG_TRACE << "creating process threads";
    int process_thread_num = 2;
    pthread_t tid;
    MutexLock tid_mutex;
    for ( int i = 0; i < process_thread_num; ++i ) {
        MutexLockGuard lock(tid_mutex);
        pthread_create(&tid, NULL, &RainbowServerApp::processThreadLoop, NULL);
        m_process_threads.push_back(tid);
        LOG_TRACE << "tid=" << (int)tid << "created";
    }

    // start server core
    m_server_core.start();
    m_loop.loop();

    // never comes here
    return true;
}

void* RainbowServerApp::processThreadLoop(void* args)
{
    LOG_TRACE << "start thread=" << (int)getTid();
    // get packet from packet manager and handle
    while ( ! app_to_exist ) {
        LOG_TRACE << "packet manager start to process in thread="
                  << (int)getTid();
        G_packet_manager.process();
    }
    return NULL;
}

bool RainbowServerApp::do_stop()
{
    return do_finish();
}
bool RainbowServerApp::do_finish()
{
    // TODO: [2013-12-08] to get thread num from configure
    int process_thread_num = 2;
    for ( int i = 0; i < process_thread_num; ++i ) {
        pthread_join(m_process_threads[i], NULL);
    }
    return true;
}
