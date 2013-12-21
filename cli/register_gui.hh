#ifndef INCLUDE_REGISTER_GUI_HPP
#define INCLUDE_REGISTER_GUI_HPP

#include "gui.hh"

class RegisterGui :public Gui
{
public:
    RegisterGui();
    virtual ~RegisterGui() {}
public:
    void compose();
    void setNotification(const char* notification, e_notification_type type);
    void processResponse(const char* response);

    void setCallbackButtonCancelClicked(callback_ptr callback, void* args);
    void setCallbackButtonRegisterClicked(callback_ptr callback, void* args);

    bool getInput(std::string& rst_widget_text, const char *widget_name, std::string& rst_error_info);

    void show();

private:
    void logResponse(const char *response);

private:
    GtkWidget* m_reg_page_label_notification;
	GtkWidget* m_reg_page_entry_registername;
	GtkWidget* m_reg_page_entry_email;
	GtkWidget* m_reg_page_entry_password;
	GtkWidget* m_reg_page_entry_repassword;
    GtkWidget* m_reg_page_textview_signature;
    GtkWidget* m_res_page_textview_response;

    // button
    GtkWidget *m_button_register;
    GtkWidget *m_button_cancel;

    callback_ptr m_button_register_clicked_callback;
    callback_ptr m_button_cancel_clicked_callback;
    muduo::MutexLock m_mutex;
};

#endif /* INCLUDE_REGISTER_GUI_HPP */
