#include <muduo/extra/DStr.h>
#include <string.h>
#include <glib.h>

#include "log.hh"
#include "global.hh"
#include "utils.hh"
#include "rainbow_data.hh"
#include "rainbow_global.hh"
#include "register_gui.hh"

RegisterGui::RegisterGui()
{
    m_window_title = global_app_name " register window";
    m_window_resizable = TRUE;
    m_button_register_clicked_callback = defaultCallBack;
    m_button_cancel_clicked_callback = defaultCallBack;
}

void RegisterGui::compose()
{
    GtkWidget * container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(m_window), container);
    // notebook
    GtkWidget * notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(container), notebook, true, true, 0);

    // setting notebook
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    // pages
    GtkWidget * register_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), register_page, newLabel("register tab", NULL));
    GtkWidget * response_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), response_page, newLabel("response tab", NULL));

    // setting register page
    gtk_container_set_border_width(GTK_CONTAINER(container), s_container_spacing);
    gtk_box_set_spacing(GTK_BOX(container), s_container_spacing);
    gtk_container_set_border_width(GTK_CONTAINER(register_page), s_container_spacing);
    gtk_box_set_spacing(GTK_BOX(register_page), 10);

    ////////////////////////////////////////////////////////////////
    PR_TRACE("create register GUI response page");
    // compose response page
    GtkWidget* scrolled_window;
    m_res_page_textview_response = newTextView(&scrolled_window,
                                               GTK_SHADOW_NONE,
                                               "", -1, -1, TRUE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(m_res_page_textview_response), FALSE);
    
    gtk_box_pack_start(GTK_BOX(response_page), scrolled_window, true, true, 0);

    ////////////////////////////////////////////////////////////////
    PR_TRACE("create register page");
    // register name
    GtkWidget* label;
    GtkWidget* hbox;

    // register name
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(register_page), hbox, FALSE, TRUE, 0);
    label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label), "<tt>  Register Name: </tt>");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
    m_reg_page_entry_registername = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), m_reg_page_entry_registername, TRUE, TRUE, 0);

    // password
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(register_page), hbox, FALSE, TRUE, 0);
    label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label), "<tt>  Your Password: </tt>");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
    m_reg_page_entry_password = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), m_reg_page_entry_password, TRUE, TRUE, 0);
    gtk_entry_set_visibility(GTK_ENTRY(m_reg_page_entry_password), FALSE);

    // re-password
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(register_page), hbox, FALSE, TRUE, 0);
    label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label), "<tt>Confirm Password: </tt>");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
    m_reg_page_entry_repassword = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), m_reg_page_entry_repassword, TRUE, TRUE, 0);
    gtk_entry_set_visibility(GTK_ENTRY(m_reg_page_entry_repassword), FALSE);

    // email
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(register_page), hbox, FALSE, TRUE, 0);
    label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label), "<tt>          Email: </tt>");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
    m_reg_page_entry_email = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), m_reg_page_entry_email, TRUE, TRUE, 0);
    // signature
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(register_page), hbox, TRUE, TRUE, 0);
    label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label), "<tt>      Signature: </tt>");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
    GtkWidget *sw = NULL;
    const char* init_text ="share your motion with your friends, you can write 100 characters";
    m_reg_page_textview_signature = newTextView(&sw, GTK_SHADOW_ETCHED_IN, init_text, -1, -1);
    gtk_box_pack_start(GTK_BOX(hbox), sw, TRUE, TRUE, 0);

    // notification label
    m_reg_page_label_notification = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(register_page), m_reg_page_label_notification, TRUE, TRUE, 0);

    // cancel/register button
    m_button_register = gtk_button_new_with_mnemonic("    _Register    ");
    m_button_cancel = gtk_button_new_with_mnemonic("    _Cancel    ");

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(container), hbox, false, false, 0);
    gtk_box_pack_start(GTK_BOX(hbox), m_button_cancel, false, false, 0);
    gtk_box_pack_end(GTK_BOX(hbox), m_button_register, false, false, 0);
    // setting
    g_signal_connect(G_OBJECT(m_button_register), "clicked",
                     G_CALLBACK(m_button_register_clicked_callback), NULL);

    g_signal_connect(G_OBJECT(m_button_cancel), "clicked",
                     G_CALLBACK(m_button_cancel_clicked_callback), NULL);

    // default setting
    setDefaultFocus(m_reg_page_entry_registername);
    setDefaultWidget(m_button_register);
}

void RegisterGui::show()
{
    Gui::show();
    gtk_widget_hide(m_reg_page_label_notification);
}

void RegisterGui::setCallbackButtonCancelClicked(callback_ptr callback, void* args)
{
    g_signal_connect(G_OBJECT(m_button_cancel), "clicked", G_CALLBACK(callback), args);
}

void RegisterGui::setCallbackButtonRegisterClicked(callback_ptr callback, void* args)
{
    g_signal_connect(G_OBJECT(m_button_register), "clicked", G_CALLBACK(callback), args);
}

void RegisterGui::setNotification(const char* notification, e_notification_type type)
{
    gtk_widget_show_all(m_reg_page_label_notification);
    if ( ! isNotificationTypeValid(type)) {
        PR_ERROR("invalid notification type=%d",(int)type);
        return;
    }
    DStr notification_to_show;
    notification_to_show.AssignFmt(getNotificationFmt(type), notification);
    gtk_label_set_markup(GTK_LABEL(m_reg_page_label_notification),
                         notification_to_show.Str());
}

void RegisterGui::logResponse(const char *response)
{
#define LONG_SEP "__________________________________________________________________"
    if ( !response || ! *response ) {
        PR_WARN("invalid argument, empty response");
        return;
    }
    writeTextView(m_res_page_textview_response,
                  "\n" LONG_SEP "\n");
    writeTextView(m_res_page_textview_response, response);
}

void RegisterGui::processResponse(const char* response)
{
    if ( !response || '\0' == *response ) {
        PR_ERROR("invalid argument, response NULL or empty");
        return;
    }
    logResponse(response);
    RegisterResponse resp(response);
    bool rc = resp.parse();
    if ( !rc ) {
        const char* error = "failed to parse register response";
        PR_ERROR("%s", error);
        setNotification(error, E_NOTIFICATION_ERROR);
        return;
    }
    if ( resp.isErrorMessage() ) {
        PR_TRACE("error message=%s", response);
        setNotification(resp.getErrorInfo().c_str(), E_NOTIFICATION_ERROR);
        return;
    }
    PR_TRACE("successfully show response=%s", response);
    std::string formatted_resp = "successfully registered, you can login with the user id now";
    formatted_resp += "\n<tt>    Name: </tt>" + resp.getName();
    formatted_resp += "\n<tt> User Id: </tt>" + resp.getUserIdStr();
    formatted_resp += "\n<tt>Password: </tt>" + resp.getPassword();

    setNotification(formatted_resp.c_str(), E_NOTIFICATION_OK);

}

bool RegisterGui::getInput(std::string& rst_widget_text, const char *widget_name, std::string& rst_error_info)
{
    // TODO: [2013-11-26] check char set x
    static struct {
        const char *name;
        bool allow_empty;
        int lease_length;
        int max_length;
        GtkWidget *widget;
    } gui_input_restriction [] = {
        { "name", false, 4, 20, m_reg_page_entry_registername },
        { "email", true, 0, 64, m_reg_page_entry_email },
        { "password", false, 4, 32, m_reg_page_entry_password },
        { "repassword", false, 4, 32, m_reg_page_entry_repassword },
        { NULL, true, 0, 0, NULL }
    };

    DStr local_error_info;
    ////////////////////////////////////////////////////////////////
    // handle text view, signature separately
    if ( 0 == strcmp("signature", widget_name)) {
        PR_TRACE("to handle signature ..");
        rst_widget_text = "to handle signature";
        GtkTextBuffer* buffer;
        GtkTextIter start, end;
        buffer = gtk_text_view_get_buffer(
            GTK_TEXT_VIEW( m_reg_page_textview_signature ));
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        rst_widget_text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

#ifndef SIGNATURE_LENGTH_MAX
#define SIGNATURE_LENGTH_MAX 100
#endif
        PR_TRACE("signature gotten, %s", rst_widget_text.c_str());
        if ( rst_widget_text.size() > SIGNATURE_LENGTH_MAX ) {
            local_error_info.AssignFmt("signature exceeds %d characters", SIGNATURE_LENGTH_MAX);
            rst_error_info = local_error_info.Str();
            return false;
        }
        return true;
    }
    ////////////////////////////////////////////////////////////////
    // now handle GUI entry
    GtkWidget *widget;
    bool allow_empty;
    int lease_length, max_length;
    int i = -1; bool found = false;
    while ( gui_input_restriction[++i].name ) {
        if ( 0 == strcmp(gui_input_restriction[i].name, widget_name )) {
            widget = gui_input_restriction[i].widget;
            allow_empty = gui_input_restriction[i].allow_empty;
            lease_length = gui_input_restriction[i].lease_length;
            max_length = gui_input_restriction[i].max_length;
            found = true;
            break;
        }
    }
    if ( ! found ) {
        PR_ERROR("widget not found for name=%s", widget_name);
        rst_error_info = "widget not found for name" + std::string(widget_name);
        return false;
    }

#define INPUT_EXCEEDS_MAX_LENGTH "exceeds the max length"
#define INPUT_DECEEDS_LEASE_LENGTH "deceeds the lease length"

    int input_len = gtk_entry_get_text_length(GTK_ENTRY(widget));
    if ( input_len > max_length) {
        local_error_info.AssignFmt("%s %s %d, %d-%d characters request",
                             widget_name, INPUT_EXCEEDS_MAX_LENGTH,
                             max_length, lease_length, max_length);
        rst_error_info = local_error_info.Str();
        PR_ERROR("widget=[%s], error info=[%s]", widget_name, rst_error_info.c_str());
        return false;
    }
    if ( input_len < lease_length && ! allow_empty ) {
        local_error_info.AssignFmt("%s %s %d, %d-%d characters request",
                             widget_name, INPUT_DECEEDS_LEASE_LENGTH,
                             lease_length, lease_length, max_length);
        rst_error_info = local_error_info.Str();
        PR_ERROR("widget=[%s], error info=[%s]", widget_name, rst_error_info.c_str());
        return false;
    }
    // now get text
    rst_widget_text = gtk_entry_get_text(GTK_ENTRY(widget));
    return true;
}
