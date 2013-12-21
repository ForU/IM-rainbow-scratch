#include "gui.hh"

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    RainbowGui *gui = new RainbowGui();
    gui->create();
    gui->show();

    gtk_main();

    return 0;
}
