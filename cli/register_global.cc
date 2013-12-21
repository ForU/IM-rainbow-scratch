#include <gtk/gtk.h>
#include <muduo/extra/Json.h>

#include "../packet_manager.hh"
#include "register_global.hh"
#include "log.hh"

RegisterGui *global_register_gui = NULL;
RegisterClient *global_register_client = NULL;

void onRegisterGuiWindowDestroy(void *args)
{
    PR_TRACE("register window destroy button clicked, %s terminates", global_register_gui->title().c_str());
    // it's bad when using new/delete to manage resource
    if (global_register_gui) {
        PR_TRACE("clear register gui");
        delete global_register_gui;
    }
    if (global_register_client) {
        PR_TRACE("clear register client");
        delete global_register_client;
    }
    gtk_main_quit();
}

void onRegisterGuiCancelButtonClicked(void *args)
{
    PR_TRACE("register GUI [cancel] button clicked, %s terminates", global_register_gui->title().c_str());
    // it's bad when using new/delete to manage resource
    if (global_register_gui) {
        PR_TRACE("clear register gui");
        delete global_register_gui;
    }
    if (global_register_client) {
        PR_TRACE("clear register client");
        delete global_register_client;
    }
    gtk_main_quit();
}

void onRegisterGuiRegisterButtonClicked(void *args)
{
    PR_TRACE("register button clicked");
    // first of all, reset notification
    global_register_gui->setNotification("registering ...", E_NOTIFICATION_NORMAL);
    // get text from entry
    std::string name, email, password, repassword, signature, error_info;
    bool error_occurred =false;
    do {
        if ( ! global_register_gui->getInput(name, "name", error_info) ||
             ! global_register_gui->getInput(email, "email", error_info) ||
             ! global_register_gui->getInput(password, "password", error_info) ||
             ! global_register_gui->getInput(repassword, "repassword", error_info) ||
             ! global_register_gui->getInput(signature, "signature", error_info) ) {
            PR_WARN("error occurred when get widget input, error info=[%s]", error_info.c_str());
            error_occurred = true; break;
        }
        // only check whether password here is same or not
        if ( password != repassword ) {
            error_info = "password not matched";
            PR_WARN("error occurred when match password, error info=[%s]", error_info.c_str());
            error_occurred = true; break;
        }
    } while ( 0 );
    if ( error_occurred ) {
        PR_TRACE("to set notification for error=%s", error_info.c_str());
        global_register_gui->setNotification(error_info.c_str(), E_NOTIFICATION_ERROR);
        return;
    }
    RegisterRequest request(name.c_str(), email.c_str(), password.c_str(), signature.c_str());

    // now input is ok, to compose register request
    std::string message;
    request.compose(message);
    // message = request.requestStr();
    // check if register client is ok
    if ( ! global_register_client ) {
        PR_ERROR("global register client not create yet");
        return;
    }
    // try to connected to server
    global_register_gui->setNotification("sends request to server, and waits for server to response ...",
                                         E_NOTIFICATION_NORMAL);
    global_register_client->connect();              // to get m_connection
    int timeout = 21, step = 1;
    bool connected = false;
    do {
        PR_TRACE("waiting connection, sleeping %d secs", step);
        sleep(step);
        if ( global_register_client->connection() ) {
            PR_TRACE("connection gotten");
            connected = true;
            break;
        }
        PR_WARN("connection not gotten");
    } while ( timeout -= step );
    if ( ! connected ) {
        PR_TRACE("cant connected to server, try again later");
        global_register_gui->setNotification("cant connected to server, try again later", E_NOTIFICATION_WARN);
        return;
    }

    Packet register_packet(message, (e_message_type)1);
    register_packet.sendTo(global_register_client->connection());
}
