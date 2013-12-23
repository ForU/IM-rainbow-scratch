#include "register_app.hh"

int main(int argc, char *argv[])
{
    RegisterApp register_app("test register application");
    if (! register_app.Init(argc, argv)) {
        return -1;
    }
    if (! register_app.Start()) {
        register_app.Finish();
        return -1;
    }
    if (! register_app.Stop() ) {
        register_app.Finish();
        return -1;
    }
    if ( ! register_app.Finish() ) {
        return -1;
    }
    return 0;
}
