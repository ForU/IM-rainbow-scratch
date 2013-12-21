#include "gui.hh"

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    Gui *gui = new ChatGui();
    gui->create();
    gui->show();

    gtk_main();

    return 0;
}
