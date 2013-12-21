#ifndef INCLUDE_GUI_HPP
#define INCLUDE_GUI_HPP

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gio/gio.h>
#include <string>
#include <map>

#include <muduo/base/Mutex.h>
#include "global.hh"

#include "rainbow_data.hh"

class BuddyData;
class GroupData;

typedef void (*callback_ptr)(void*);
extern void defaultCallBack(void* args);
extern void defaultWindowDestoryCallBack(void* args);

// for GUI notification
enum e_notification_type
{
    E_NOTIFICATION_NORMAL,
    E_NOTIFICATION_TRACE,
    E_NOTIFICATION_OK,
    E_NOTIFICATION_WARN,
    E_NOTIFICATION_ERROR,
    // to add
    E_NOTIFICATION_UPPER_BOUDN,
};

struct gui_notification_info_t {
    e_notification_type type;
    const char *fmt;
};

extern gui_notification_info_t notification_infos[];
extern bool isNotificationTypeValid(e_notification_type type);
extern const char* getNotificationFmt(e_notification_type type);

class Gui
{
public:
    Gui(callback_ptr win_destroy_cb=defaultWindowDestoryCallBack);
    ~Gui() {}

public:
    // create a window, but not show
    Gui* create();
    virtual void compose() {}
    virtual void setSettings() {}
    const std::string title() const { return m_window_title; }
    virtual void setWindowTitle(const char* title);
    virtual void show();
    virtual void hide();
    void present() { gtk_window_present(GTK_WINDOW(m_window)); }

    void setCallbackWindowDestroy(callback_ptr callback, void* args=NULL);
    void setDefaultFocus(GtkWidget* widget);
    void setDefaultWidget(GtkWidget* widget);

protected:
    // basic window information
    GtkWidget * m_window;
	gint m_window_width;
	gint m_window_height;

    bool m_window_destroy_with_parent;
	bool m_window_accessible;
    bool m_window_resizable;
    bool m_window_show_titlebar;
    bool m_window_center;

    callback_ptr m_window_destroy_callback;
    std::string m_window_title;
};

// sidebar
enum {
    COL_ICON = 0,
    COL_NAME,
    COL_SIGNATURE,
    COL_COMMON_ID,
    COL_GUI_DATA_POINTER,
    COL_GUI_UTILITIES_POINTER,
    COL_UPPER_BOUND
};

class InfoListBar
{
public:
    InfoListBar();
    ~InfoListBar() {}
    void doActivatedOnSingleClick();
    GtkWidget* container();
    GtkWidget* getTreeView() { return m_tree_view; }
    bool add(const char* name,
             const char* icon_path="",
             const char* signature="",
             const char* common_id="",
             void *data_ptr=NULL,
             void* gui_utilities=NULL,
             const char* category="");

    bool update(GtkTreeIter& iter,
                bool to_update_icon=false, const char* icon_path="",
                bool to_update_name=false, const char* name="",
                bool to_update_signature=false, const char*signature="");

    // bool update(GtkTreeIter& iter, const char* icon_path, bool signature_changed=false, const char*signature="");

    void hide();
    void show();

    void setBarAsTree() { m_bar_as_tree = true; }
    void ExpandTreeViewAll();

    bool findRowCategory(const char* category, GtkTreeIter& iter);
    bool findRow(const char* common_id, GtkTreeIter& iter, const char* category="");

    bool setRowSelected(const char* common_id);
    void setSelectionFunction(GtkTreeSelectionFunc func);

private:
    GtkWidget* compose();

private:
    int m_width;
    GtkWidget* m_container;
    GtkWidget* m_scrolled_window;
    GtkWidget* m_tree_view;
    // data
    GtkTreeStore* m_tree_store;
    GtkTreeIter m_iter;
    GtkTreeIter m_parent_iter;

    enum detail_mode {
        MODE_SIMPLE,
        MODE_DETAIL,
    };
    detail_mode m_detail_mode;
    bool m_bar_as_tree;
};

class RainbowLoginGui : public Gui
{
public:
    RainbowLoginGui();
    virtual ~RainbowLoginGui() {}

    void compose();
    bool getInput(std::string& rst_widget_text, const char *widget_name, std::string& rst_error_info);
    void setCallbackButtonLoginClicked(callback_ptr callback, void* args);
    void setNotification(const char* notification, e_notification_type type);
    void show();

private:
    GtkWidget* m_entry_name;
	GtkWidget* m_entry_password;
    GtkWidget* m_label_notification;
    GtkWidget* m_button_login;
};


// for showing account user's data
class UserInfoBar
{
public:
    UserInfoBar(const std::string& name="name here", const std::string& signature="signature here");
    virtual ~UserInfoBar() {}
    GtkWidget* container();
    void setAccoutAsButton(bool);
    void show();
    void updateAccountName(const char* name);
    void updateAccountSigature(const char* signature);
    const char* accountName() const;

    GtkWidget* compose();

private:
    GtkWidget* m_container;
    GtkWidget* m_icon_user;
    GtkWidget* m_label_name;
    GtkWidget* m_label_signature;
    bool m_accout_as_button;
};


#endif /* INCLUDE_GUI_HPP */
