#include <muduo/extra/DStr.h>
#include <string.h>
#include <glib.h>

#include "log.hh"
#include "global.hh"
#include "utils.hh"
#include "rainbow_data.hh"
#include "rainbow_global.hh"
#include "rainbow_gui.hh"



static void
onRainbowUserbarRowActivated(GtkTreeView *tree_view,
                             GtkTreePath *path,
                             GtkTreeViewColumn  *col,
                             gpointer user_data)
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *model;
    BuddyData *buddy_data;

    buddy_data = NULL;
    selection = gtk_tree_view_get_selection(tree_view);
    // handle selection, to get information about selection
    if ( ! gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),
                                           &model, &iter)) {
        PR_WARN("foaled to call gtk_tree_selection_get_selected on rainbow user bar");
        return;
    }
    // handle parent row
    if ( gtk_tree_model_iter_has_child(model, &iter) ) {
        bool expanded = gtk_tree_view_row_expanded(tree_view, path);
        bool success;
        if ( expanded ) {
            success = gtk_tree_view_collapse_row(tree_view, path);
        } else {
            success = gtk_tree_view_expand_row(tree_view, path, true);
        }
        PR_TRACE("%s %s current row",
                 success ? "successfully to" : "failed to",
                 expanded ? "collapse" : "expand");
        return;
    }
    // handle normal row
    gtk_tree_model_get(model, &iter,
                       COL_GUI_DATA_POINTER, &buddy_data,
                       -1);
    if ( NULL == buddy_data ) {
        PR_WARN("buddy data not found");
        return;
    }
    // show real buddy data
    PR_TRACE("real buddy data, name=%s, signature=%s, user id=%s, status=%s",
             buddy_data->getName().c_str(),
             buddy_data->getSignature().c_str(),
             buddy_data->getUserIdStr().c_str(),
             buddy_data->isOnLine() ? "on-line" : "off-line");

    const std::string& buddy_common_id = buddy_data->getUserIdStr();
    const std::string& buddy_name = buddy_data->getName();

    ////////////////////////////////////////////////////////////////
    if ( ! g_gui_data_manager.isChatWindowCreated()) {
        PR_TRACE("first time to create chat window");
        g_chat_gui.create();
        g_gui_data_manager.setChatWindowCreated();
    } else {
        PR_TRACE("to use already created chat window");
    }
    // TODO: [2013-12-03] to recover, always to show
    PR_TRACE("*ALWAYS* to show chat window");
    g_chat_gui.show();

    ////////////////////////////////////////////////////////////////
    // add current a new(user) sidebar item into chat window
    // in do -> while, only check and add, has nothing to do
    // showing
    do {
        if ( buddy_data->isShowingChat() ) {
            PR_TRACE("this user's chat utilities already created");
            break;
        }
        // add buddy into chat sidebar, and the pair is created and
        // stored in data manager
        bool rc = g_chat_gui.addBuddy(buddy_data);
        if ( ! rc ) {
            PR_ERROR("failed to add buddy=[%s] to chat window side bar",
                     buddy_name.c_str());
            return;
        }
        buddy_data->setChatShowing(true);
        PR_TRACE("is user showed in chat window? [%s]",
                 buddy_data->isShowingChat() ? "yes" : "no");
    } while ( 0 );

    // NOTICE: make sure always present chat window,
    // when this function is invoked.
    PR_TRACE("*ALWAYS* set the row selected, id=%s", buddy_common_id.c_str());
    g_chat_gui.setSidebarRowSelected(buddy_common_id.c_str());
    PR_TRACE("*ALWAYS* to present the chat window");
    g_chat_gui.present();
}

static void
onRainbowGroupbarRowActivated(GtkTreeView *tree_view,
                              GtkTreePath *path,
                              GtkTreeViewColumn  *col,
                              gpointer user_data)
{
    GtkTreeSelection * selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeIter iter;
    GtkTreeModel *model;

    // TODO: [2013-12-02]
    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),
                                        &model, &iter)) {
        gchar* name;
        gchar* signature;
        gtk_tree_model_get(model, &iter,
                           COL_NAME, &name,
                           COL_SIGNATURE, &signature, -1);
        PR_TRACE("current name:%s, signature=%s",
                 name, signature);
        g_free(name);
        g_free(signature);
    }
}

// static void
// onRainbowGuiToggleButtonAccoutToggled(void* args)
// {
//     PR_TRACE("account toggled");
// }

////////////////////////////////////////////////////////////////
RainbowGui::RainbowGui()
{
    m_window_show_titlebar = false;
    // m_window_width = 300;
    // m_window_height = 400;
    // m_window_center = true;

    // user bar already created
    m_user_bar.setBarAsTree();

    // NOTICE: make sure widgets on which following actions operate
    //         are composed
    // init account as toggle button
    m_account_title.setAccoutAsButton(true);
    // set callback
    g_signal_connect(G_OBJECT(m_user_bar.getTreeView()),
                     "row-activated",
                     G_CALLBACK(onRainbowUserbarRowActivated),
                     NULL);
    g_signal_connect(G_OBJECT(m_group_bar.getTreeView()),
                     "row-activated",
                     G_CALLBACK(onRainbowGroupbarRowActivated),
                     NULL);
}

void RainbowGui::handleSearchResultInSearchBar(const FriendsManagementResponse& response)
{
    if ( response.isErrorMessage() ) {
        m_search_widget.initShowForAddMenu();
        m_search_widget.setNotification( response.getErrorInfo().c_str(),
                                         E_NOTIFICATION_ERROR);
        return;
    }

    // normal response
    const std::string& target_user_id = response.getTargetIdStr();
    const std::string& target_name = response.getTargetName();
    const std::string& target_signature = response.getTargetSignature();

    std::string message_to_show;
    message_to_show =  "<b><tt>  User Id: </tt></b>" + target_user_id + "\n";
    message_to_show += "<b><tt>     Name: </tt></b>" + target_name + "\n";
    message_to_show += "<b><tt>Signature: </tt></b>" + target_signature + "\n";
    if ( response.isAlreadyFriend() ) {
        message_to_show += "<b><tt>     Info: </tt></b>"
            + target_name + " is already your friend, "
            "[Delete] button for deleting";
    } else {
        message_to_show += "<b><tt>     Info: </tt></b>"
            + target_name + " is not your friend yet, "
            "[Add] button for adding";
    }

    m_search_widget.setNotification(message_to_show.c_str(),
                                    E_NOTIFICATION_NORMAL);
    // widget showing control part
    m_search_widget.showAll();
    if ( response.isAlreadyFriend() ) {
        m_search_widget.hideAddButton();
    } else {
        m_search_widget.hideDeleteButton();
    }
}

bool RainbowGui::addBuddy( const BuddyData& buddy, BuddyData* data_pointer)
{
    const char* icon_path;
    const std::string& status = buddy.getStatus();
    if ( OFF_LINE == status  ) {
        icon_path = getIconPath(ICON_TYPE_USER_INACTIVE_MIDDLE);
    } else if ( ON_LINE == status ) {
        icon_path = getIconPath(ICON_TYPE_USER_ACTIVE_MIDDLE);
    } else {
        PR_WARN("unknown status=%s", status.c_str());
        icon_path = getIconPath(ICON_TYPE_USER_INACTIVE_MIDDLE);
    }

    const std::string& category = buddy.getCategory();
    const std::string& user_id = buddy.getUserIdStr();
    const std::string name = buddy.getName();
    DStr markup_name;
    markup_name.AssignFmt("<b>%s</b>\n(<i>%s</i>)", name.c_str(), user_id.c_str());
    return m_user_bar.add(markup_name.Str(),
                          icon_path,
                          buddy.getSignature().c_str(),
                          buddy.getUserIdStr().c_str(),
                          data_pointer,
                          NULL,
                          category.c_str());
}

bool RainbowGui::addGroup( const GroupData& group, GroupData* data_pointer)
{
    const std::string name = group.getName();
    const std::string group_id = group.getGroupIdStr();
    DStr markup_name;
    markup_name.AssignFmt("<b>%s</b>\n(<i>%s</i>)", name.c_str(), group_id.c_str());
    return m_group_bar.add(markup_name.Str(),
                           getIconPath(ICON_TYPE_GROUP),
                           group.getSignature().c_str(),
                           group.getGroupIdStr().c_str(),
                           (void*)&data_pointer);
}

bool RainbowGui::updateBuddy(const char *user_id_str, bool online,
                             bool signature_changed, const char* signature)
{
    if ( ! user_id_str ) {
        PR_ERROR("invalid argument, user id is NULL");
        return false;
    }
    // find buddy_data pointer by user_id_str from data manager
    BuddyData* buddy_data = g_gui_data_manager.findBuddy_Guard(user_id_str);
    if ( ! buddy_data ) {
        PR_WARN("user not found by data manager, user id=%d", user_id_str);
        return false;
    }
    // set online status into data
    buddy_data->setOnLineStatus(online);
    PR_TRACE("buddy status=%s", buddy_data->isOnLine()? "on":"off");
    if ( signature_changed ) {
        buddy_data->setSignature(signature);
    }

    // find user from user bar (tree store)
    PR_TRACE("finding user from user bar");
    const std::string& category = buddy_data->getCategory();
    GtkTreeIter iter;
    bool rc = m_user_bar.findRow(user_id_str, iter, category.c_str());
    if ( ! rc ) {
        PR_WARN("user not found in user bar, common id", user_id_str);
        return true;
    }
    // now, set the icon
    const char* icon_path = NULL;
    if ( online ) {
        icon_path = getIconPath(ICON_TYPE_USER_ACTIVE_MIDDLE);
    } else {
        icon_path = getIconPath(ICON_TYPE_USER_INACTIVE_MIDDLE);
    }
    PR_TRACE("to update user icon according to online status=%s, icon path=%s",
             online? ON_LINE : OFF_LINE, icon_path);
    return m_user_bar.update(iter, true, icon_path, signature_changed, signature);
}

bool RainbowGui::updateGroup(const char *group_id_str, bool signature_changed, const char* signature)
{
    // TODO: [2013-12-02]
    return false;
}

GtkWidget* RainbowGui::getButtonTogglePreference()
{
    return m_toggle_button_preference;
}

GtkWidget* RainbowGui::getButtonToggleUser()
{
    return m_toggle_button_user;
}

GtkWidget* RainbowGui::getButtonTogglegroup()
{
    return m_toggle_button_group;
}

void RainbowGui::hideUserBar()
{
    m_user_bar.hide();
}

void RainbowGui::hideGroupBar()
{
    m_group_bar.hide();
}

void RainbowGui::showUserBar()
{
    m_user_bar.show();
}

void RainbowGui::showGroupBar()
{
    m_group_bar.show();
}

GtkWidget* RainbowGui::getUserBar()
{
    return m_user_bar.container();
}

GtkWidget* RainbowGui::getGroupBar()
{
    return m_group_bar.container();
}

static void
onRainbowGuiToggleButtonUserorGroupToggled(GtkWidget *togglebutton, void* args)
{
    RainbowGui* gui = (RainbowGui*)args;
    bool isUserToggleButton = (togglebutton == gui->getButtonToggleUser());
    gboolean pressed = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton));

    PR_TRACE("toggled! toggle button=[%s], pressed status=[%s]",
             isUserToggleButton ? "user button": "group button",
             pressed ? "pressed": "not pressed");

    // handle
    if ( isUserToggleButton ) {
        if ( pressed ) {
            gui->showUserBar();
            gui->hideGroupBar();
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui->getButtonTogglegroup()), false);
        } else {
            gui->showGroupBar();
            gui->hideUserBar();
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui->getButtonTogglegroup()), true);
        }
    } else {
        if ( pressed ) {
            gui->showGroupBar();
            gui->hideUserBar();
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui->getButtonToggleUser()),false);
        } else {
            gui->showUserBar();
            gui->hideGroupBar();
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui->getButtonToggleUser()), true);
        }
    }
}

SearchWidgetBar::SearchWidgetBar()
{
    compose();
}

void SearchWidgetBar::initShowForAddMenu()
{
    gtk_widget_show_all(m_container);
    gtk_widget_hide(m_label_user_info);
    gtk_widget_hide(m_button_add);
    gtk_widget_hide(m_button_delete);
}

static void
onRainbowSearchWidgetEntryActivate(GtkWidget* self_entry, void* args)
{
    PR_TRACE("calling entry activate");
    SearchWidgetBar* search_widget_bar = (SearchWidgetBar*)args;

    std::string entry_content = gtk_entry_get_text(GTK_ENTRY(self_entry));
    search_widget_bar->setLatestEntryContent(entry_content);

    int text_length;
    text_length = gtk_entry_get_text_length(GTK_ENTRY(self_entry));

    if ( 0 == text_length ) {
        PR_TRACE("Empty input, hide into label");
        search_widget_bar->hideLabelInfo();
        return;
    }

#ifndef USER_ID_LENGTH_MIN
#define USER_ID_LENGTH_MIN (5)
#endif

#ifndef USER_ID_LENGTH_MAX
#define USER_ID_LENGTH_MAX (12)
#endif

#ifndef USER_NAME_LENGTH_MIN
#define USER_NAME_LENGTH_MIN (4)
#endif

#ifndef USER_NAME_LENGTH_MAX
#define USER_NAME_LENGTH_MAX (32)
#endif

    int max_length, min_length;
    max_length = max( USER_ID_LENGTH_MAX, USER_NAME_LENGTH_MAX);
    min_length = min( USER_ID_LENGTH_MIN, USER_NAME_LENGTH_MIN );
    if ( text_length > max_length ||
         text_length < min_length ) {
        std::string input_size_str, min_str, max_str;
        int2str(input_size_str, (int)text_length);
        int2str(min_str, (int)min_length);
        int2str(max_str, (int)max_length);

        std::string err_notification;
        err_notification = "wrong size of search content: <b><span foreground='red'>"
            + input_size_str + "</span></b>, you can type in " + "[" + min_str
            + " - " + max_str + "] " + "charactors\n";

        PR_WARN("%s", err_notification.c_str());
        search_widget_bar->setNotification(err_notification.c_str(),
                                           E_NOTIFICATION_ERROR);
        return;
    }
    search_widget_bar->setNotification("", E_NOTIFICATION_NORMAL);
    // clear entry buffer
    GtkEntryBuffer* entry_buffer;
    entry_buffer = gtk_entry_get_buffer(GTK_ENTRY(self_entry));
    gtk_entry_buffer_delete_text(entry_buffer, 0, text_length);
    PR_TRACE("input content=[%s]", entry_content.c_str());

    // now, to compose and send
    search_widget_bar->composeAndSendRequest(entry_content,
                                             FRIENDS_MANAGE_ACTION_SEARCH);
}

void SearchWidgetBar::composeAndSendRequest(const std::string& target_id_str,
                                            int action,
                                            const std::string& category)
{
    PR_TRACE("composing search request");
    static const std::string& requester_id = g_gui_data_manager.getSelfUserIdStr();
    static const std::string& session_id = g_gui_data_manager.getSelfSessionId();

    PR_TRACE("requester=[%s], target=[%s], session id=[%s]",
             requester_id.c_str(), target_id_str.c_str(), session_id.c_str());

    e_friends_manage_action manage_action = (e_friends_manage_action)action;
    if ( ! isFriendsManageActionValid(manage_action)) {
        PR_ERROR("invalid argument, managed action=%d", (int)manage_action);
        setNotification("Error occur in client, this message won't' be sent",
                        E_NOTIFICATION_ERROR);
        return;
    }

    FriendsManagementMessage req_msg( manage_action,
                                      requester_id,
                                      target_id_str,
                                      category,
                                      session_id);
    PR_TRACE("request body:%s", req_msg.bodyStr().c_str());

    PR_TRACE("to compose and send packet");
    Packet packet(req_msg.bodyStr(), req_msg.getType());
    packet.sendTo(g_rainbow_client->connection());
}

void SearchWidgetBar::setNotification(const char* notification,
                                      e_notification_type type)
{
    if ( ! isNotificationTypeValid(type)) {
        PR_ERROR("invalid notification type=%d",(int)type);
        return;
    }
    DStr notification_to_show;
    notification_to_show.AssignFmt(getNotificationFmt(type), notification);
    gtk_label_set_markup(GTK_LABEL(m_label_user_info),
                         notification_to_show.Str());
    gtk_widget_show_all(m_label_user_info);
}

static void
onSearchWidgetBarButtonCancelClicked(GtkWidget* button, void* args)
{
    SearchWidgetBar *self_bar = (SearchWidgetBar*)args;
    self_bar->hide();
}

static void
onSearchWidgetBarButtonDeleteClicked(GtkWidget* button, void* args)
{
    // TODO: [2013-12-17]
    PR_TRACE("delete button clicked, TODO");
}

static void
onSearchWidgetBarButtonAddClicked(GtkWidget* button, void* args)
{
    SearchWidgetBar *self_bar = (SearchWidgetBar*)args;
    const std::string& target_id_str = self_bar->getLatestEntryContent();
    self_bar->composeAndSendRequest(target_id_str,
                                    FRIENDS_MANAGE_ACTION_ADD);
}

void SearchWidgetBar::compose()
{
    m_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(m_container), 10);
    gtk_box_set_spacing(GTK_BOX(m_container), 2);

    // entry search
    m_entry_search = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(m_container), m_entry_search, false, true, 0);

    // setting entry
    const char* entry_icon_tooltip = "<i>You can search user or group by the id or name.\n"
        "empty input will not send and \n"
        "press \"Enter\" to start searching</i>";

    GtkEntryIconPosition entry_icon_pos = GTK_ENTRY_ICON_PRIMARY;
    gtk_entry_set_icon_tooltip_markup(GTK_ENTRY(m_entry_search),
                                      entry_icon_pos,
                                      entry_icon_tooltip);
    GdkPixbuf* entry_search_pixbuf = newIconFromPath(getIconPath(ICON_TYPE_SEARCH));
    gtk_entry_set_icon_from_pixbuf(GTK_ENTRY(m_entry_search),
                                   entry_icon_pos,
                                   entry_search_pixbuf);

    // entry activated signal
    g_signal_connect(G_OBJECT(m_entry_search),
                     "activate",
                     G_CALLBACK(onRainbowSearchWidgetEntryActivate),
                     gpointer(this));


    // label info
    m_label_user_info = newLabel("", true);
    gtk_box_pack_start(GTK_BOX(m_container), m_label_user_info, false, true, 0);


    // // button stuff
    // GtkWidget* valign = gtk_alignment_new(0.5, 0.5, 0, 0);
    // gtk_box_pack_start(GTK_BOX(m_container),
    //                  valign, false, true, 0);

    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(m_container), hbox);

    // cancel button
    m_button_cancel = gtk_button_new_with_mnemonic("    _Cancel    ");
    gtk_box_pack_start(GTK_BOX(hbox), m_button_cancel, false, false, 0);
    g_signal_connect(G_OBJECT(m_button_cancel), "clicked",
                     G_CALLBACK(onSearchWidgetBarButtonCancelClicked),
                     gpointer(this));

    // action button
    m_button_add = gtk_button_new_with_mnemonic("    _Add    ");
    gtk_box_pack_end(GTK_BOX(hbox), m_button_add, false, false, 0);
    g_signal_connect(G_OBJECT(m_button_add), "clicked",
                     G_CALLBACK(onSearchWidgetBarButtonAddClicked),
                     gpointer(this));

    // action button
    m_button_delete = gtk_button_new_with_mnemonic("    _Delete    ");
    gtk_box_pack_end(GTK_BOX(hbox), m_button_delete, false, false, 0);
    g_signal_connect(G_OBJECT(m_button_delete), "clicked",
                     G_CALLBACK(onSearchWidgetBarButtonDeleteClicked),
                     gpointer(this));


}


static void
pref_add_user_callback(GtkWidget *widget, RainbowGui* gui)
{
    PR_TRACE("to call add user callback");
    gui->showSearchWidgetBar();
}

static void
pref_search_user_callback(GtkWidget *widget, RainbowGui* gui)
{
    PR_TRACE("to call search user callback");
    gui->showSearchWidgetBar();
}

static void
pref_del_user_callback(GtkWidget *widget, RainbowGui* gui)

{
    PR_TRACE("to call add user callback");
    gui->showSearchWidgetBar();
}

void RainbowGui::compose()
{
    // gui_container(header_bar(custom title), bar)
    GtkWidget* gui_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(m_window), gui_container);

    // header bar
    m_header_bar = gtk_header_bar_new();
    gtk_box_pack_start(GTK_BOX(gui_container), m_header_bar, false, true, 0);
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(m_header_bar),
                                         ! m_window_show_titlebar);

    // header_bar/ users button, groups button
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(m_header_bar), hbox);

    // gtk_container_add(GTK_CONTAINER(m_header_bar), hbox);
    gtk_style_context_add_class (gtk_widget_get_style_context (hbox), "linked");
    m_toggle_button_user = gtk_toggle_button_new();
    m_toggle_button_group = gtk_toggle_button_new();
    m_toggle_button_preference = gtk_menu_button_new();
    gtk_header_bar_pack_end (GTK_HEADER_BAR (m_header_bar), m_toggle_button_preference);
    gtk_box_pack_start (GTK_BOX (hbox), m_toggle_button_user, false, true, 0);
    gtk_box_pack_end (GTK_BOX (hbox), m_toggle_button_group, false, true, 0);
    // header_bar/ buttons setting
    GtkWidget* image_users = gtk_image_new_from_file(getIconPath(ICON_TYPE_USERS));
    GtkWidget* image_groups = gtk_image_new_from_file(getIconPath(ICON_TYPE_GROUP));
    GtkWidget* image_preference = gtk_image_new_from_file(getIconPath(ICON_TYPE_PREFERENCE));
    gtk_button_set_image(GTK_BUTTON(m_toggle_button_user), image_users);
    gtk_button_set_image(GTK_BUTTON(m_toggle_button_group), image_groups);
    gtk_button_set_image(GTK_BUTTON(m_toggle_button_preference), image_preference);

    // signal
    g_signal_connect(G_OBJECT(m_toggle_button_user),
                     "toggled",
                     G_CALLBACK(onRainbowGuiToggleButtonUserorGroupToggled),
                     gpointer(this));
    g_signal_connect(G_OBJECT(m_toggle_button_group),
                     "toggled",
                     G_CALLBACK(onRainbowGuiToggleButtonUserorGroupToggled),
                     gpointer(this));
    // header_bar/title
    m_account_title.compose();
    GtkWidget* custom_title = m_account_title.container();
    gtk_header_bar_set_custom_title(GTK_HEADER_BAR(m_header_bar), custom_title);

    // header_bar/preference
    GtkWidget* pref_menu = gtk_menu_new();
    GtkWidget* pref_menu_item;

    pref_menu_item = gtk_menu_item_new_with_label("Add User");
    gtk_menu_attach (GTK_MENU (pref_menu), pref_menu_item, 0, 1, 0, 1);
    g_signal_connect(G_OBJECT (pref_menu_item), "activate",
                     G_CALLBACK(pref_add_user_callback),
                     gpointer(this));

    pref_menu_item = gtk_menu_item_new_with_label("Delete User");
    gtk_menu_attach (GTK_MENU (pref_menu), pref_menu_item, 0, 1, 1, 2);
    g_signal_connect(G_OBJECT (pref_menu_item), "activate",
                     G_CALLBACK(pref_del_user_callback),
                     gpointer(this));

    pref_menu_item = gtk_menu_item_new_with_label("Search User");
    gtk_menu_attach (GTK_MENU (pref_menu), pref_menu_item, 0, 1, 2, 3);
    g_signal_connect(G_OBJECT (pref_menu_item), "activate",
                     G_CALLBACK(pref_search_user_callback),
                     gpointer(this));

    gtk_menu_button_set_popup(GTK_MENU_BUTTON(m_toggle_button_preference),
                              pref_menu);
    gtk_widget_show_all(pref_menu); // to show them, important

    // search widget container
    gtk_box_pack_start(GTK_BOX(gui_container), m_search_widget.container(),
                       false, true, 0);

    // separator, to separate sidebar
    GtkWidget* bar_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(gui_container), bar_separator, false, true, 0);

    // user bar and group bar
    gtk_box_pack_start(GTK_BOX(gui_container), m_user_bar.container(),
                       true, true, 0);
    gtk_box_pack_start(GTK_BOX(gui_container), m_group_bar.container(),
                       true, true, 0);

    // ends here
}

void RainbowGui::show()
{
    Gui::show();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getButtonToggleUser()), true);
    showUserBar();
    hideGroupBar();
    hideSearchWidgetBar();
}

void RainbowGui::updateAccountTitle(const char*name, const char*signature)
{
    m_account_title.updateAccountName(name);
    m_account_title.updateAccountSigature(signature);
}

void RainbowGui::updateCategoryInfoAll()
{
    std::vector<std::string> category_vec;
    g_gui_data_manager.getCategories_Guard(category_vec);
    int size = category_vec.size();
    for ( int i = 0; i < size; ++i ) {
        updateCategoryInfo(category_vec[i].c_str());
    }
}

void RainbowGui::updateCategoryInfo(const char* category)
{
    if ( ! category ) {
        PR_ERROR("invalid argument, category is NULL");
        return;
    }
    GtkTreeIter iter;
    bool rc = m_user_bar.findRowCategory(category, iter);
    if ( ! rc ) {
        PR_ERROR("failed to get category=%s from rainbow GUI user bar", category);
        return;
    }
    // get category data
    int on, total;
    rc = g_gui_data_manager.getCategoryInfo_Guard(category, on, total);
    if ( ! rc ) {
        PR_ERROR("failed to get category info");
        return;
    }
    std::string on_str, total_str, category_statics;

    int2str(on_str, on);
    int2str(total_str, total);

    std::string color = on > 0 ? "green" : "red";
    category_statics = "<span foreground='"+ color +"'><i>("
        + on_str + "|" + total_str + ")</i></span>";
    m_user_bar.update(iter, false, "",
                      true, category, // name
                      true, category_statics.c_str()); // signature
}

