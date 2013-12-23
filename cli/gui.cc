#include <muduo/extra/DStr.h>
#include <vector>
#include <string.h>
#include <glib.h>

#include "gui.hh"
#include "log.hh"
#include "global.hh"
#include "utils.hh"
#include "rainbow_data.hh"
#include "rainbow_global.hh"
#include "../packet_manager.hh"

////////////////////////////////////////////////////////////////
gui_notification_info_t notification_infos[] = {
    E_NOTIFICATION_NORMAL, "<i>%s</i>",
    E_NOTIFICATION_TRACE, "<b>Trace: </b><i>%s</i>",
    E_NOTIFICATION_OK, "<span foreground=\"green\"><b>Success:</b></span> %s",
    E_NOTIFICATION_WARN,"<span foreground=\"yellow\"><b>Warning:</b></span> %s",
    E_NOTIFICATION_ERROR, "<span foreground=\"red\"><b>Error:</b></span> %s",
    E_NOTIFICATION_UPPER_BOUDN, ""
};

bool isNotificationTypeValid(e_notification_type type)
{
    return (type < E_NOTIFICATION_UPPER_BOUDN
            && type >= E_NOTIFICATION_NORMAL);
}

const char* getNotificationFmt(e_notification_type type)
{
    return notification_infos[type].fmt;
}


////////////////////////////////////////////////////////////////
#define MAX_FIXED_SIGNATURE_LENGTH 50
////////////////////////////////////////////////////////////////


static void
updateRainbowGuiByGuiData()
{
    if ( ! g_login_response ) {
        PR_WARN("login response not received yet");
        return;
    }

    // self information
    const std::string& account_name = g_login_response->getName();
    const std::string& account_signature = g_login_response->getSignature();
    PR_TRACE("to update account information, name=%s, signature=%s",
             account_name.c_str(), account_signature.c_str());
    g_rainbow_gui.updateAccountTitle(account_name.c_str(), account_signature.c_str());

    // buddies
    PR_TRACE("to update buddies information for rainbow GUI");
    GList * items = g_gui_data_manager.getDataBuddies();
    while ( items ) {
        g_rainbow_gui.addBuddy(*((BuddyData*)(items->data)), (BuddyData*)(items->data));
        items = g_list_next(items);
    }
    g_rainbow_gui.expandUserBar();

    // groups
    PR_TRACE("to update groups information for rainbow GUI");
    items = g_gui_data_manager.getDataGroups();
    while ( items ) {
        g_rainbow_gui.addGroup(*((GroupData*)(items->data)), (GroupData*)(items->data));
        items = g_list_next(items);
    }
    g_rainbow_gui.expandUserBar();

    // update category info
    PR_TRACE("update rainbow GUI category info");
    g_rainbow_gui.updateCategoryInfoAll();

    // update window title
    PR_TRACE("setting rainbow GUI title");
    std::string window_title = account_name;
    window_title += " - " global_app_name;
    g_rainbow_gui.setWindowTitle(window_title.c_str());
}

static void
createAndInitGuiDataManager()
{
    PR_TRACE("initialize data manager self information");
    // ok
    g_gui_data_manager.storeSelfData( g_login_response->getUserIdStr().c_str(),
                                      g_login_response->getName().c_str(),
                                      g_login_response->getSignature().c_str(),
                                      g_login_response->getStatus().c_str(),
                                      g_login_response->getSessionId().c_str());
    // buddies
    PR_TRACE("initialize data manager buddies");
    const std::vector<buddy_t>& buddies = g_login_response->getBuddies();
    int size = buddies.size();
    for ( int i = 0; i < size; ++i ) {
        BuddyData *data = new BuddyData(buddies[i]);
        g_gui_data_manager.storeBuddyData(data);

        const std::string& category = data->getCategory();
        g_gui_data_manager.handleCategoryInfo_Guard( category, CATEGORY_INFO_ACTION_PLUS_TOTAL, 1);
        if ( data->isOnLine() ) {
            g_gui_data_manager.handleCategoryInfo_Guard(category, CATEGORY_INFO_ACTION_PLUS_ON, 1);
        }
    }
    // groups
    PR_TRACE("to initialize data manager groups");
    const std::vector<group_t>& groups = g_login_response->getGroups();
    size = groups.size();
    for ( int i = 0; i < size; ++i ) {
        GroupData *data = new GroupData(groups[i]);
        g_gui_data_manager.storeGroupData(data);
    }
}

void onLoginGuiButtonLoginClicked(void* args)
{
    static muduo::MutexLock local_mutex;
    muduo::MutexLockGuard _lock(local_mutex);
    PR_TRACE("LOGIN GUI's login button clicked");

    if ( ! g_rainbow_client ) {
        PR_ERROR("global rainbow client not create yet");
        return;
    }

    std::string name, password, error_info;
    if ( ! g_login_gui.getInput(name, "name", error_info) ||
         ! g_login_gui.getInput(password, "password", error_info)) {
        PR_WARN("error occurred when get widget input, error info=[%s]",
                error_info.c_str());
        PR_TRACE("to set notification for error=%s", error_info.c_str());
        g_login_gui.setNotification(error_info.c_str(), E_NOTIFICATION_ERROR);
        return;
    }

    // try to connected to server
    // g_rainbow_client->connect();
    int timeout = 3, step = 1;
    bool connected = false;
    while ( timeout ) {
        if ( g_rainbow_client->connection() ) {
            PR_TRACE("connection gotten");
            connected = true;
            break;
        }
        g_rainbow_client->connect();
        PR_TRACE("waiting connection for %d secs", timeout);
        sleep(step);
        timeout -= step;
    }
    if ( ! connected ) {
        PR_TRACE("Can't connect to server, try again later");
        g_login_gui.setNotification("Can't connect to server, try again later",
                                    E_NOTIFICATION_ERROR);
        return;
    }

    // compose login request packet
    LoginRequest login_request(name.c_str(), password.c_str());
    std::string message;
    login_request.compose(message);
    Packet packet(message, login_request.getType());
    packet.sendTo(g_rainbow_client->connection());

    // TODO: [2013-11-29] to use Condition
    PR_TRACE("waiting for login response");
    pthread_mutex_lock(&g_mutex_login_response);
    while ( NULL == g_login_response ) {
        pthread_cond_wait(&g_condtion_login_res_received, &g_mutex_login_response);
    }
    PR_TRACE("successfully to receive login response received here");
    pthread_mutex_unlock(&g_mutex_login_response);

    // check response error code
    if (g_login_response->isErrorMessage()) {
        PR_DEBUG("response is error message");
        error_info = g_login_response->getErrorInfo();
        g_login_gui.setNotification(error_info.c_str(), E_NOTIFICATION_WARN);
        return;
    }

    // now message successfully received
    createAndInitGuiDataManager();
    g_login_gui.hide();

    g_rainbow_gui.create();
    // update rainbow_gui with response
    updateRainbowGuiByGuiData();
    g_rainbow_gui.show();
}
////////////////////////////////////////////////////////////////

Gui::Gui(callback_ptr win_destroy_cb)
{
    PR_TRACE("constructing base GUI");

    m_window_destroy_callback = win_destroy_cb;
    m_window_title = global_app_name;

    m_window_width = 600;
    m_window_height = 400;

    m_window_destroy_with_parent = false;
    m_window_resizable = true;
    m_window_accessible = true;
    m_window_show_titlebar = true;
    m_window_center = false;
    // NOTICE: to show image in button
    GtkSettings *default_settings = gtk_settings_get_default();
    g_object_set(default_settings, "gtk-button-images", TRUE, NULL);
}

Gui* Gui::create()
{
    PR_TRACE("creating GUI=[%s]", title().c_str());
    m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    // compose specifically
    PR_TRACE("to compose GUI");
    compose();

    // setting m_window property after compose, Caz compose() may change the
    // property of window
    gtk_window_set_title(GTK_WINDOW(m_window), m_window_title.c_str());
    gtk_widget_set_size_request(GTK_WIDGET(m_window), m_window_width, m_window_height);
    gtk_window_set_resizable(GTK_WINDOW(m_window), m_window_resizable);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(m_window), m_window_destroy_with_parent);
    g_signal_connect(G_OBJECT(m_window), "destroy", G_CALLBACK(m_window_destroy_callback), NULL);
    gtk_window_set_decorated(GTK_WINDOW(m_window), m_window_show_titlebar);
    if (m_window_center) {
        gtk_window_set_position(GTK_WINDOW(m_window), GTK_WIN_POS_CENTER);
    } else {
        gtk_window_set_position(GTK_WINDOW(m_window), GTK_WIN_POS_MOUSE);
    }
    // to overwrite above default setting
    PR_TRACE("to set GUI settings");

    return this;
}

void Gui::show()
{
    PR_TRACE("showing GUI=[%s]", title().c_str());
    gtk_widget_show_all(m_window);
}

void Gui::hide()
{
    PR_TRACE("hiding GUI=[%s]", title().c_str());
    gtk_widget_hide(m_window);
}

void Gui::setWindowTitle(const char* title_str)
{
    if ( ! title_str ) {
        title_str = "";
    }
    m_window_title = title_str;
    m_window_title += " - " global_app_name;
    gtk_window_set_title(GTK_WINDOW(m_window),
                         m_window_title.c_str());
}

void Gui::setDefaultFocus(GtkWidget* widget)
{
    gtk_window_set_focus (GTK_WINDOW (m_window), widget);
}

void Gui::setCallbackWindowDestroy(callback_ptr callback, void* args)
{
    PR_TRACE("setting window destroy callback");
    g_signal_connect(G_OBJECT(m_window), "destroy", G_CALLBACK(callback), args);
}

void Gui::setDefaultWidget(GtkWidget* widget)
{
    gtk_widget_set_can_default(widget, true);
    gtk_window_set_default (GTK_WINDOW (m_window), widget);
}

InfoListBar::InfoListBar()
{
    m_width = 200;
    m_bar_as_tree = false;

    compose();

    // setting widget after compose
    g_signal_connect(G_OBJECT(m_tree_view), "row-activated",
                     G_CALLBACK(defaultCallBack), NULL);
}

void InfoListBar::doActivatedOnSingleClick()
{
    gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(m_tree_view), true);
}

GtkWidget* InfoListBar::compose()
{
    // container(scrolled_window(text_view), hbox(button_add, button_delete))
    m_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(m_container, m_width, -1);

    // scrolled_window(text_view)
    m_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(m_container), m_scrolled_window, true, true, 0);

    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(m_scrolled_window),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    // store
    m_tree_store = gtk_tree_store_new(COL_UPPER_BOUND,
                                      GDK_TYPE_PIXBUF,  // COL_ICON
                                      G_TYPE_STRING,    // COL_NAME
                                      G_TYPE_STRING,    // COL_SIGNATURE/topic
                                      G_TYPE_STRING,    // COL_COMMON_ID/(row id)
                                      G_TYPE_POINTER,   // COL_GUI_DATA_POINTER
                                      G_TYPE_POINTER ); // COL_GUI_UTILITIES_POINTER
    GtkTreeModel* model = GTK_TREE_MODEL(m_tree_store);

    m_tree_view  = gtk_tree_view_new_with_model(model);
    gtk_container_add(GTK_CONTAINER(m_scrolled_window), m_tree_view);
    // construct treeview
    GtkTreeViewColumn *col;
    GtkCellRenderer* renderer;

    col = gtk_tree_view_column_new();
    // icon cell
    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start(col, renderer, false);
    gtk_tree_view_column_set_attributes(col, renderer, "pixbuf", COL_ICON, NULL);
    // name cell
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start(col, renderer, true);
    gtk_tree_view_column_set_attributes(col, renderer, "markup", COL_NAME, NULL);
    // signature cell
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start(col, renderer, true);
    gtk_tree_view_column_set_attributes(col, renderer, "markup", COL_SIGNATURE, NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(m_tree_view), col);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_tree_view), false);
    return m_container;
}

static const char* fix_signature(const char*signature)
{
    if ( ! signature ) {
        PR_TRACE("invalid argument, signature is NULL");
        return NULL;
    }
    bool need_to_fix = (strlen(signature) > MAX_FIXED_SIGNATURE_LENGTH);
    if ( ! need_to_fix ) {
        return signature;
    }

    static DStr fixed_signature;

    fixed_signature.Empty();
    fixed_signature.Assign(signature, MAX_FIXED_SIGNATURE_LENGTH);
    fixed_signature += "...";
    return fixed_signature.Str();
}

bool InfoListBar::add(const char* name, const char* icon_path,
                      const char* signature, const char* common_id,
                      void* sidebar_item, void* chat_pair, const char* category)
{
    if ( ! name || ! common_id) {
        PR_WARN("invalid argument, name or common_id is NULL");
        return false;
    }
    if ( ! icon_path || ! *icon_path ) {
        icon_path = getIconDefaultPath();
    }
    GdkPixbuf* icon_pixbuf = newIconFromPath(icon_path);
    if ( ! icon_pixbuf ) {
        PR_ERROR("failed call newIconFromPath, icon_path=%s", icon_path);
        return false;
    }
    // check if category need to add
    GtkTreeIter* p_parent_iter;
    PR_TRACE("to check if need to handle category");
    if ( m_bar_as_tree ) {
        PR_TRACE("handle category=%s", category);
        bool category_found = findRowCategory(category, m_parent_iter);
        if ( ! category_found ) {
            PR_TRACE("to create category=%s", category);
            gtk_tree_store_append(GTK_TREE_STORE(m_tree_store), &m_parent_iter, NULL);
            gtk_tree_store_set(GTK_TREE_STORE(m_tree_store), &m_parent_iter,
                               COL_NAME, category, // for showing
                               COL_COMMON_ID, category, // for filter/find
                               -1);
        } else {
            PR_TRACE("category already created");
        }
        p_parent_iter = &m_parent_iter;
    } else {
        p_parent_iter = NULL;
    }

    // add buddy info
    gtk_tree_store_append(GTK_TREE_STORE(m_tree_store), &m_iter, p_parent_iter);
    gtk_tree_store_set(GTK_TREE_STORE(m_tree_store), &m_iter,
                       COL_ICON, icon_pixbuf,
                       COL_NAME, name,
                       COL_SIGNATURE, fix_signature(signature),
                       COL_COMMON_ID, common_id,
                       COL_GUI_DATA_POINTER, sidebar_item,
                       COL_GUI_UTILITIES_POINTER, chat_pair,
                       -1);
    return true;
}

bool InfoListBar::update(GtkTreeIter& iter,
                         bool to_update_icon, const char* icon_path,
                         bool to_update_name, const char* name,
                         bool to_update_signature, const char*signature)
{
    if ( (to_update_icon && ! icon_path )
         || (to_update_name && ! name)
         || (to_update_signature && ! signature) ) {
        PR_ERROR("invalid argument, %s %s %s",
                 to_update_icon ? ", icon is NULL," : "",
                 to_update_name ? ", name is NULL," : "",
                 to_update_signature ? ", signature is NULL," : "");
        return false;
    }
    if ( to_update_icon ) {
        PR_TRACE("updating user icon");
        GdkPixbuf* icon_pixbuf = newIconFromPath(icon_path);
        if ( ! icon_pixbuf ) {
            PR_ERROR("failed call newIconFromPath, icon_path=%s", icon_path);
            return false;
        }
        PR_TRACE("updating icon=%s", icon_path);
        gtk_tree_store_set(GTK_TREE_STORE(m_tree_store), &iter,
                           COL_ICON, icon_pixbuf, -1);
    }
    if ( to_update_name ) {
        PR_TRACE("updating changed name=%s", name);
        gtk_tree_store_set(GTK_TREE_STORE(m_tree_store), &iter,
                           COL_NAME, name, -1);
    }
    if ( to_update_signature ) {
        PR_TRACE("updating changed signature, raw=%s", signature);
        gtk_tree_store_set(GTK_TREE_STORE(m_tree_store), &iter,
                           COL_SIGNATURE, fix_signature(signature), -1);
    }
    return true;
}

bool InfoListBar::findRowCategory(const char* category, GtkTreeIter& iter)
{
    if ( ! m_bar_as_tree ) {
        PR_WARN("the info list bar is not used as tree, [%s] do nothing", __func__);
        return false;
    }
    if ( ! category ) {
        PR_WARN("invalid argument, category id is NULL");
        return false;
    }
    if (! gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_tree_store), &iter) ) {
        PR_WARN("the tree store is empty");
        return false;
    }

    gchar *category_name;
    bool has_child;
    bool found;
    do {
        has_child = gtk_tree_model_iter_has_child(GTK_TREE_MODEL(m_tree_store), &iter);
        if ( ! has_child ) {
            PR_WARN("current iterator has no children");
            continue;
        }
        gtk_tree_model_get(GTK_TREE_MODEL(m_tree_store), &iter,
                           COL_COMMON_ID, &category_name, // for find
                           -1);
        PR_TRACE("category_name=%s", category_name);

        found = (! strcmp(category, category_name));
        g_free(category_name);  // to free memory
        if ( found ) { return true; }
    } while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(m_tree_store), &iter) );
    return false;
}

bool InfoListBar::findRow(const char* common_id, GtkTreeIter& iter, const char* category)
{
    if ( ! common_id ) {
        PR_WARN("invalid argument, common id is NULL");
        return false;
    }
    bool rc;
    if ( m_bar_as_tree ) {
        if ( ! category ) {
            PR_WARN("invalid argument, category is NULL");
            return false;
        }

        GtkTreeIter parent_iter;
        rc = findRowCategory(category, parent_iter);
        PR_TRACE("%s to call findRowCategory", rc ? "successfully" : "failed");
        if ( ! rc  ) return false;
        rc = gtk_tree_model_iter_children(GTK_TREE_MODEL(m_tree_store), &iter, &parent_iter);
    } else {
        rc = gtk_tree_model_iter_children(GTK_TREE_MODEL(m_tree_store), &iter, NULL);
    }
    PR_TRACE("%s to call gtk_tree_model_iter_children", rc ? "successfully" : "failed");

    // only run on this level
    do {
        char *tmp_common_id = NULL;
        gtk_tree_model_get(GTK_TREE_MODEL(m_tree_store), &iter,
                           COL_COMMON_ID, &tmp_common_id,
                           -1);
        bool equal = (! strcmp(tmp_common_id, common_id) );
        PR_TRACE("arg:%s, found:%s", common_id, tmp_common_id);
        g_free(tmp_common_id);
        if (equal ){ // equal
            return true;
        }
    } while ( gtk_tree_model_iter_next(GTK_TREE_MODEL(m_tree_store), &iter) );
    return false;
}

bool InfoListBar::setRowSelected(const char* common_id)
{
    if ( !common_id ) {
        PR_ERROR("invalid argument, common id NULL");
        return false;
    }
    GtkTreeIter iter;
    bool rc = findRow(common_id, iter);
    if ( ! rc ) {
        PR_DEBUG("not found for common id=%s", common_id);
        return false;
    }
    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_tree_view));
    gtk_tree_selection_select_iter(selection, &iter);

    ////////////////////////////////////////////////////////////////
    // check if already selected
    bool selected = false;
    selected = gtk_tree_selection_iter_is_selected(selection, &iter);
    PR_TRACE("common id=%s row selected? [%s]",
             common_id, selected ? "yes" : "no");

    ////////////////////////////////////////////////////////////////
    PR_TRACE("to get pair from selection");
    gchar *showing_name;
    ChatPair* chat_pair = NULL;
    ChatSideBarItem* sidebar_item = NULL;
    gtk_tree_model_get(GTK_TREE_MODEL(m_tree_store), &iter,
                       COL_NAME, &showing_name,
                       COL_GUI_DATA_POINTER, &sidebar_item,
                       COL_GUI_UTILITIES_POINTER, &chat_pair,
                       -1);

    // TODO to return the sidebar_item, then put following code in chat_gui.cc
    PR_TRACE("to update input entry");
    g_chat_gui.updateInputEntry(sidebar_item->getType(), sidebar_item->getData());

    PR_TRACE("to show the output pair");
    gtk_widget_show_all(chat_pair->container());

    PR_TRACE("to update the chat window's title");
    g_chat_gui.setWindowTitle(showing_name);

    g_free(showing_name);

    return true;
}

void InfoListBar::ExpandTreeViewAll()
{
    if ( ! m_bar_as_tree ) {
        PR_WARN("current bar is not initialized as tree, "
                "used as list instead");
    }
    gtk_tree_view_expand_all(GTK_TREE_VIEW(m_tree_view));
}

GtkWidget* InfoListBar::container()
{
    return m_container;
}

void InfoListBar::hide()
{
    gtk_widget_hide(m_container);
}

void InfoListBar::setSelectionFunction(GtkTreeSelectionFunc func)
{
    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_tree_view));
    gtk_tree_selection_set_select_function(selection, func, NULL, NULL);
}

void InfoListBar::show()
{
    gtk_widget_show_all(m_container);
}

RainbowLoginGui::RainbowLoginGui()
{
    m_window_width = 100;
    m_window_height = 80;
    m_window_title = global_app_name " login";
    m_window_resizable = false;
    m_window_center = true;
}

void RainbowLoginGui::setNotification(const char* notification, e_notification_type type)
{
    if ( ! isNotificationTypeValid(type)) {
        PR_ERROR("invalid notification type=%d",(int)type);
        return;
    }
    DStr notification_to_show;
    notification_to_show.AssignFmt(getNotificationFmt(type), notification);
    gtk_widget_show_all(m_label_notification);
    gtk_label_set_markup(GTK_LABEL(m_label_notification),
                         notification_to_show.Str());
}

void RainbowLoginGui::show()
{
    Gui::show();
    gtk_widget_hide(m_label_notification);
}

void RainbowLoginGui::compose()
{
    // gui container
    GtkWidget* gui_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(m_window), gui_container);
    gtk_container_set_border_width(GTK_CONTAINER(gui_container), s_container_spacing);
    gtk_box_set_spacing(GTK_BOX(gui_container), 10);

    ////////////////////////////////////////////////////////////////
    // notification
    m_label_notification = newLabel("", true);
    gtk_box_pack_start(GTK_BOX(gui_container), m_label_notification, FALSE, TRUE, 0);

    // name, password
    GtkEntryIconPosition icon_pos = GTK_ENTRY_ICON_PRIMARY;
    // name entry
    m_entry_name = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(gui_container), m_entry_name, FALSE, TRUE, 0);
    std::string name_icon_tooltip = "Please fill in your User Id";
    GdkPixbuf* name_pixbuf = newIconFromPath(getIconDefaultPath());
    gtk_entry_set_icon_tooltip_markup(GTK_ENTRY(m_entry_name),
                                      icon_pos,
                                      name_icon_tooltip.c_str());
    gtk_entry_set_icon_from_pixbuf(GTK_ENTRY(m_entry_name),
                                   icon_pos,
                                   name_pixbuf);

    ////////////////////////////////////////////////////////////////
    // password entry
    m_entry_password = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(gui_container), m_entry_password, FALSE, TRUE, 0);
    gtk_entry_set_visibility(GTK_ENTRY(m_entry_password), FALSE);
    std::string password_icon_tooltip = "Please fill in your password";
    GdkPixbuf* password_pixbuf = newIconFromPath(getIconPath(ICON_TYPE_PASSWORD));
    gtk_entry_set_icon_tooltip_markup(GTK_ENTRY(m_entry_password),
                                      icon_pos,
                                  password_icon_tooltip.c_str());
    gtk_entry_set_icon_from_pixbuf(GTK_ENTRY(m_entry_password),
                                   icon_pos,
                                   password_pixbuf);

    ////////////////////////////////////////////////////////////////
    // button
    GtkWidget* valign = gtk_alignment_new(0.5, 0.5, 0, 0);
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(valign), vbox);
    gtk_box_pack_end(GTK_BOX(gui_container), valign, false, true, 0);


    m_button_login = gtk_button_new_with_mnemonic("    _Login    ");
    gtk_box_pack_end(GTK_BOX(vbox), m_button_login, false, false, 0);

    // setting focus and default
    setDefaultFocus(m_entry_name);
    setDefaultWidget(m_button_login);

    setCallbackWindowDestroy(onRainbowGuiWindowDestroy, NULL);
    setCallbackButtonLoginClicked(onLoginGuiButtonLoginClicked, NULL);
    // ends here
}


void RainbowLoginGui::setCallbackButtonLoginClicked(callback_ptr callback, void* args)
{
    g_signal_connect(G_OBJECT(m_button_login), "clicked", G_CALLBACK(callback), args);
}

bool RainbowLoginGui::getInput(std::string& rst_widget_text, const char *widget_name, std::string& rst_error_info)
{
    static struct {
        const char *name;
        bool allow_empty;
        int lease_length;
        int max_length;
        GtkWidget *widget;
    } gui_input_restriction [] = {
        { "name", false, 4, 20, m_entry_name },
        { "password", true, 0, 32, m_entry_password },
        { NULL, true, 0, 0, NULL }
    };

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
    DStr error_info;
    int input_len = gtk_entry_get_text_length(GTK_ENTRY(widget));
    if ( input_len > max_length) {
        error_info.AssignFmt("%s %s %d, %d-%d characters request",
                             widget_name, INPUT_EXCEEDS_MAX_LENGTH,
                             max_length, lease_length, max_length);
        rst_error_info = error_info.Str();
        PR_ERROR("widget=[%s], error info=[%s]", widget_name, rst_error_info.c_str());
        return false;
    }
    if ( input_len < lease_length && ! allow_empty ) {
        error_info.AssignFmt("%s %s %d, %d-%d characters request",
                             widget_name, INPUT_DECEEDS_LEASE_LENGTH,
                             lease_length, lease_length, max_length);
        rst_error_info = error_info.Str();
        PR_ERROR("widget=[%s], error info=[%s]", widget_name, rst_error_info.c_str());
        return false;
    }
    // now get text
    rst_widget_text = gtk_entry_get_text(GTK_ENTRY(widget));
    return true;
}

UserInfoBar::UserInfoBar(const std::string& name, const std::string& signature)
{
    PR_TRACE("constructing user info bar");
    setAccoutAsButton(false);
}

void UserInfoBar::show()
{
    gtk_widget_show_all(m_container);
}

void UserInfoBar::updateAccountName(const char* name)
{
    if ( NULL == name ) {
        PR_WARN("invalid argument, name is NULL, do nothing");
        return;
    }
    DStr markup_name; markup_name.AssignFmt("<b>%s</b>", name);
    gtk_label_set_markup(GTK_LABEL(m_label_name), markup_name.Str());
}

void UserInfoBar::updateAccountSigature(const char* signature)
{
    if ( NULL == signature ) {
        PR_WARN("invalid argument, signature is NULL, do nothing");
        return;
    }
    DStr fixed_signature;
    bool to_fix = (strlen(signature) > MAX_FIXED_SIGNATURE_LENGTH);
    if ( to_fix ) {
        fixed_signature.Assign(signature, MAX_FIXED_SIGNATURE_LENGTH);
        fixed_signature += "...";
    }
    gtk_label_set_markup(GTK_LABEL(m_label_signature),
                         to_fix ? fixed_signature.Str() : signature);
}

GtkWidget* UserInfoBar::container()
{
    return m_container;
}

const char* UserInfoBar::accountName() const
{
    const char* text = NULL;
    text = gtk_label_get_text(GTK_LABEL(m_label_name));
    if ( ! text ) {
        PR_WARN("no text gotten from name label, return empty string");
        return "";
    }
    return text;
}

void UserInfoBar::setAccoutAsButton(bool val)
{
    m_accout_as_button = val;
}

GtkWidget* UserInfoBar::compose()
{
    // h_title(icon, vbox(name, signature))
    m_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    // TODO: [2013-11-28] to add icon from data base.
    GtkWidget* accout;
    if ( m_accout_as_button ) {
        accout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        // image
        GtkWidget* image_accout;
        image_accout= gtk_image_new_from_file(getIconPath(ICON_TYPE_USER_ACTIVE_MIDDLE));
        // toggle button
        GtkWidget* toggle_button_accout;
        toggle_button_accout = gtk_toggle_button_new();
        gtk_box_pack_start (GTK_BOX (accout), toggle_button_accout, false, true, 0);
        gtk_button_set_relief(GTK_BUTTON(toggle_button_accout), GTK_RELIEF_NONE);
        gtk_button_set_image(GTK_BUTTON(toggle_button_accout), image_accout);
    } else {
        accout= gtk_image_new_from_file(getIconPath(ICON_TYPE_USER_ACTIVE_MIDDLE));
    }

    gtk_box_pack_start(GTK_BOX(m_container), accout, false, true, 0);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(m_container), vbox, false, true, 0);
    m_label_name = newLabel("", ALIGN_TYPE_LEFT, false);
    gtk_box_pack_start(GTK_BOX(vbox), m_label_name, true, true, 0);
    m_label_signature = newLabel("", ALIGN_TYPE_LEFT);
    gtk_box_pack_end(GTK_BOX(vbox), m_label_signature, true, true, 0);

    return m_container;
}

