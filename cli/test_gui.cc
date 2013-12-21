#include "gui.hh"
#include "register_global.hh"

#include <stdlib.h>

int main(int argc, char *argv[])
{
    gtk_init (&argc, &argv);

    global_register_gui = new RegisterGui();
    global_register_gui->create();

    // callback
    global_register_gui->setCallbackWindowDestroy(onRegisterGuiWindowDestroy, NULL);
    global_register_gui->setCallbackButtonRegisterClicked(onRegisterGuiRegisterButtonClicked, NULL);
    global_register_gui->setCallbackButtonCancelClicked(onRegisterGuiCancelButtonClicked, NULL);

    global_register_gui->show();

    gtk_main ();

    return 0;
}
