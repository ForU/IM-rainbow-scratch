#include "register_app.hh"
#include "register_global.hh"
#include "log.hh"

RegisterApp::RegisterApp(const std::string& app_name)
    : App(app_name) {}

bool RegisterApp::do_init(int argc, char *argv[])
{
    if ( argc != 3 ) {
        PR_WARN("invalid command line argument, usage:%s server_ip server_port", argv[0]);
        printf("Usage:%s server_ip server_port\n", argv[0]);
        return false;
    }

    // create global register gui
    gtk_init (&argc, &argv);
    global_register_gui = new RegisterGui();
    if ( NULL == global_register_gui ) {
        PR_ERROR("create register gui failed");
        return false;
    }
    global_register_gui->create();
    // callback
    global_register_gui->setCallbackWindowDestroy(onRegisterGuiWindowDestroy, NULL);
    global_register_gui->setCallbackButtonRegisterClicked(onRegisterGuiRegisterButtonClicked, NULL);
    global_register_gui->setCallbackButtonCancelClicked(onRegisterGuiCancelButtonClicked, NULL);

    // init tcp client
    m_server_port = static_cast<uint16_t>(atoi(argv[2]));
    m_server_ip = argv[1];

    return true;
}

bool RegisterApp::do_start()
{
    // one thread to create epoll loop, and main thread to run app gui
    muduo::net::EventLoopThread loopThread;
    muduo::net::InetAddress serverAddr(m_server_ip.c_str(), m_server_port);
    global_register_client = new RegisterClient(loopThread.startLoop(), serverAddr);
    if ( NULL == global_register_gui ) {
        PR_ERROR("create global register client failed");
        return false;
    }
    // main thread show and run gtk
    global_register_gui->show();
    // return when gtk_main_quit() called
    gtk_main ();
    return true;
}

bool RegisterApp::do_stop()
{
    return do_finish();
}

bool RegisterApp::do_finish()
{
    return true;
}

