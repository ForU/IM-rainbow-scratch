#include "rainbow_app.hh"

int main(int argc, char *argv[])
{
    RainbowApp rainbow_app("test rainbow application");
    if (! rainbow_app.Init(argc, argv)) {
        return -1;
    }
    if (! rainbow_app.Start()) {
        rainbow_app.Finish();
        return -1;
    }
    if (! rainbow_app.Stop() ) {
        rainbow_app.Finish();
        return -1;
    }
    if ( ! rainbow_app.Finish() ) {
        return -1;
    }
    return 0;
}
