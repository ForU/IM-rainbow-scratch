#ifndef INCLUDE_CALLBACK_HPP
#define INCLUDE_CALLBACK_HPP

#include "register_gui.hh"
#include "register_client.hh"
#include "../singleton.hh"

// TODO: [2013-12-12] to singleton

extern RegisterGui *global_register_gui;
extern RegisterClient *global_register_client;

// this is the default callback
extern void onRegisterGuiWindowDestroy(void *args);
extern void onRegisterGuiCancelButtonClicked(void *args);
extern void onRegisterGuiRegisterButtonClicked(void *args);

#endif /* INCLUDE_CALLBACK_HPP */
