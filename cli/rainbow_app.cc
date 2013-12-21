#include "rainbow_app.hh"
#include "rainbow_global.hh"
#include "log.hh"

RainbowApp::RainbowApp(const std::string& app_name)
    : App(app_name) {}

bool RainbowApp::do_init(int argc, char *argv[])
{
    if ( argc != 3 ) {
        PR_WARN("invalid command line argument, usage:%s server_ip server_port", argv[0]);
        printf("Usage:%s server_ip server_port\n", argv[0]);
        return false;
    }
    // GUI initialization
    gtk_init (&argc, &argv);
    g_login_gui.create();

    // init tcp client
    m_server_port = static_cast<uint16_t>(atoi(argv[2]));
    m_server_ip = argv[1];

    return true;
}

bool RainbowApp::do_start()
{
    // one thread to create epoll loop, and main thread to run app gui
    muduo::net::EventLoopThread loopThread;
    muduo::net::InetAddress serverAddr(m_server_ip.c_str(), m_server_port);

    g_rainbow_client = new RainbowClient(loopThread.startLoop(), serverAddr);
    // main thread show and run gtk
    g_login_gui.show();
    // return when gtk_main_quit() called
    gtk_main ();
    return true;
}

bool RainbowApp::do_stop()
{
    return do_finish();
}

bool RainbowApp::do_finish()
{
    return true;
}

