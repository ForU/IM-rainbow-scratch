#include <gtk/gtk.h>
#include <muduo/extra/Json.h>

#include "rainbow_global.hh"
#include "log.hh"
#include "utils.hh"

pthread_cond_t g_condtion_login_res_received = PTHREAD_COND_INITIALIZER;
pthread_mutex_t g_mutex_login_response = PTHREAD_MUTEX_INITIALIZER;
LoginResponse* g_login_response = NULL;
RainbowClient *g_rainbow_client = NULL;

void onRainbowGuiWindowDestroy(void* args)
{
    PR_TRACE("rainbow window destroy button clicked, %s terminates", g_login_gui.title().c_str());
    gtk_main_quit();
}

void onRainbowChatGuiWindowDestroy(void* args)
{
    g_chat_gui.hide();
}
