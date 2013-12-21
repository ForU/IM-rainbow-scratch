#ifndef INCLUDE_RAINBOW_GLOBAL_HPP
#define INCLUDE_RAINBOW_GLOBAL_HPP

#include <gtk/gtk.h>
#include <pthread.h>

#include "chat_gui.hh"
#include "rainbow_gui.hh"
#include "rainbow_client.hh"
#include "rainbow_data.hh"
#include "../packet_manager.hh" // for login response

#include "../singleton.hh"

// global variables
// extern RainbowLoginGui *g_login_gui;
// extern RainbowChatGui *g_chat_gui;
// extern RainbowGui *g_rainbow_gui;
extern RainbowClient *g_rainbow_client;
// extern RainBowGuiDataManager *g_gui_data_manager;

#define g_login_gui SingletonX<RainbowLoginGui>::instance()
#define g_chat_gui SingletonX<RainbowChatGui>::instance()
#define g_rainbow_gui SingletonX<RainbowGui>::instance()
#define g_gui_data_manager SingletonX<RainBowGuiDataManager>::instance()

extern pthread_cond_t g_condtion_login_res_received;
extern pthread_mutex_t g_mutex_login_response;
extern LoginResponse* g_login_response;

// functions
extern void onLoginGuiButtonLoginClicked(void* args);
extern void onRainbowChatGuiWindowDestroy(void* args);
extern void onRainbowGuiWindowDestroy(void* args);

#endif /* INCLUDE_RAINBOW_GLOBAL_HPP */
