#include "server_app.hh"

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc == 1) {
        printf("Usage: %s port\n", argv[0]);
        return -1;
    }

    // TODO: [2013-12-08] to port from configure.
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress serverAddr(port);
    RainbowServerApp app(serverAddr);

    if (! app.Init(argc, argv)) {
        return -1;
    }
    if (! app.Start()) {
        app.Finish();
        return -1;
    }
    if (! app.Stop() ) {
        app.Finish();
        return -1;
    }
    if ( ! app.Finish() ) {
        return -1;
    }
    return 0;
}

