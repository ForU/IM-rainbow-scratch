#include <string.h>
#include <glib.h>

#include "chat_gui.hh"
#include "log.hh"
#include "global.hh"
#include "utils.hh"
#include "rainbow_data.hh"
#include "rainbow_global.hh"
#include "../packet_manager.hh"

////////////////////////////////////////////////////////////////
static void
onRainbowChatSidebarRowActivated(GtkTreeView *tree_view,
                                 GtkTreePath *path,
                                 GtkTreeViewColumn  *col,
                                 gpointer user_data)
{
    // TODO: [2013-12-03] now just show some information from store
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *model;
    ChatSideBarItem *sidebar_item;
    ChatPair *chat_pair;

    sidebar_item = NULL;
    selection = gtk_tree_view_get_selection(tree_view);

    // handle selection, to get information about selection
    // to get iter
    if (! gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),
                                          &model, &iter)) {
        PR_WARN("failed to call gtk_tree_selection_get_selected");
        return;
    }

    gchar *showing_name;
    gchar *showing_signature;
    gchar *showing_common_id;
    gtk_tree_model_get(model, &iter,
                       COL_NAME, &showing_name,
                       COL_SIGNATURE, &showing_signature,
                       COL_COMMON_ID, &showing_common_id,
                       COL_GUI_DATA_POINTER, &sidebar_item,
                       COL_GUI_UTILITIES_POINTER, &chat_pair,
                       -1);
    PR_TRACE("gui showing name:%s, signature=%s, common_id=%s",
             showing_name, showing_signature, showing_common_id);
    void *item_data = sidebar_item->getData();
    chat_type item_type = sidebar_item->getType();
    const std::string& item_name = sidebar_item->isUser()
        ? ((BuddyData*)item_data)->getName()
        : ((GroupData*)item_data)->getName();

    PR_TRACE("information from sidebar item data, type=%d, name=%s",
             (int)item_type, item_name.c_str());
    // free copy of string
    g_free(showing_name);
    g_free(showing_signature);
    g_free(showing_common_id);

    // update chat entry data
    g_chat_gui.updateInputEntry(item_type, item_data);
    // update chat title
    g_chat_gui.setWindowTitle(item_name.c_str());
    // utilities
    PR_TRACE("to show gui utilities");
    gtk_widget_show_all(chat_pair->container());
}

static void
onRainbowChatGuiEntrySendActivate(GtkWidget* entry, void* args)
{
    PR_TRACE("calling entry activate");
    std::string text_content;
    int text_length;

    text_content = gtk_entry_get_text(GTK_ENTRY(entry));
    text_length = gtk_entry_get_text_length(GTK_ENTRY(entry));
    if ( 0 == text_length ) {
        PR_TRACE("Empty input");
        return;
    }
    if ( text_length > MSG_CHAT_BUF_SIZE ) {
        std::string msg_chat_buf_size_str;
        int2str(msg_chat_buf_size_str, (int)MSG_CHAT_BUF_SIZE);

        std::string err_notification;
        err_notification = "too long chat message, exceeds ";
        err_notification += msg_chat_buf_size_str + "characters, ";
        err_notification += "the message will not be sent\n";

        PR_WARN("%s", err_notification.c_str());
        std::string common_id = (char*)args;
        ChatPair* chat_pair;
        chat_pair = g_gui_data_manager.findChatPair_Guard(common_id.c_str());
        if ( ! chat_pair ) {
            return;
        }
        chat_pair->append(err_notification.c_str(), MSG_ST_ERROR);
        return;
    }
    // clear entry buffer
    GtkEntryBuffer* entry_buffer;
    entry_buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
    gtk_entry_buffer_delete_text(entry_buffer, 0, text_length);
    PR_TRACE("input content=[%s]", text_content.c_str());

    g_chat_gui.send(text_content);
}

gboolean ChatSideBarSelectionFunc (GtkTreeSelection *selection,
                                   GtkTreeModel *model,
                                   GtkTreePath *path,
                                   gboolean path_currently_selected,
                                   gpointer data)
{
    // A function used by gtk_tree_selection_set_select_function()
    // to filter whether or not a row may be selected. It is called
    // whenever a row's state might change. A return value of TRUE
    // indicates to selection that it is okay to change the
    // selection.

    if ( path_currently_selected ) {
        PR_TRACE("current path selected");
        // only to hide current sec
        ChatSideBarItem *sidebar_item = NULL;
        ChatPair *chat_pair = NULL;
        GtkTreeIter iter;
        if (! gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),
                                              &model, &iter)) {
            PR_WARN("failed to call gtk_tree_selection_get_selected on chat sidebar");
            return true;
        }
        gtk_tree_model_get(model, &iter,
                           COL_GUI_DATA_POINTER, &sidebar_item,
                           COL_GUI_UTILITIES_POINTER, &chat_pair,
                           -1);
        PR_TRACE("to hide selected pair");
        gtk_widget_hide(chat_pair->container());
    } else {
        PR_TRACE("current path not selected");
    }
    return true;
}

ChatInputEntry::ChatInputEntry()
{
    PR_TRACE("constructing chat input entry");
    m_type = (chat_type)-1;
    m_data = NULL;
    compose();
    // setting entry

    std::string common_id;
    getCommonIdStr(common_id);

    // TODO: [2013-12-09] define another getCommonIdStr
    // for memory saving
#define COMMON_ID_SIZE 32
    char buf[COMMON_ID_SIZE];
    strncpy(buf, common_id.c_str(), sizeof buf);

    g_signal_connect(G_OBJECT(m_entry_send),
                     "activate",
                     G_CALLBACK(onRainbowChatGuiEntrySendActivate),
                     (void*)buf);
}

void ChatInputEntry::compose()
{
    m_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    // entry send
    m_entry_send = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(m_container), m_entry_send,
                       true, true, 0);


    // entry icon

    const std::string& self_name = g_gui_data_manager.getSelfName();
    std::string entry_icon_tooltip = "<i>I am "+ self_name
        +", write message here and press \"Enter\" to send the message</i>";
    GtkEntryIconPosition icon_pos = GTK_ENTRY_ICON_PRIMARY;
    gtk_entry_set_icon_tooltip_markup(GTK_ENTRY(m_entry_send),
                                      icon_pos,
                                      entry_icon_tooltip.c_str());
    GdkPixbuf* send_pixbuf = newIconFromPath(getIconDefaultPath());
    gtk_entry_set_icon_from_pixbuf(GTK_ENTRY(m_entry_send),
                                   icon_pos,
                                   send_pixbuf);

    // button send
    // m_button_send = gtk_button_new_with_mnemonic("    _Send    ");
    // gtk_box_pack_end(GTK_BOX(m_container), m_button_send,
    //                  false, true, 0);

}

bool ChatInputEntry::update(chat_type type, void* data)
{
    // TODO: [2013-12-04] to mutex this process
    if (! isChatTypeValid(type)) {
        PR_ERROR("failed to update for invalid type=%d", type);
        return false;
    }
    if ( NULL == data ) {
        PR_WARN("data is NULL, no need to upate");
        return false;
    }
    m_type = type;
    m_data = data;
    return true;
}

void ChatInputEntry::getName(std::string& name) const
{
    name = (m_type == CHAT_TYPE_USER)
        ? ((BuddyData*)m_data)->getName()
        : ((GroupData*)m_data)->getName();
}

void ChatInputEntry::getCommonIdStr(std::string& common_id) const
{
    if (m_type == CHAT_TYPE_USER) {
        common_id = ((BuddyData*)m_data)->getUserIdStr();
    } else if (m_type == CHAT_TYPE_USER) {
        common_id = ((GroupData*)m_data)->getGroupIdStr();
    } else {
        common_id = "";
    }
}

////////////////////////////////////////////////////////////////
ChatPair::ChatPair(const std::string& common_id)
    : m_common_id(common_id)
{
    PR_TRACE("constructing chat pair for common id=%s", m_common_id.c_str());
    m_container = NULL;
    m_textview = NULL;

    compose();
}

void ChatPair::compose()
{
    PR_TRACE("composing chat pair for user=%s", m_common_id.c_str());
    m_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    // scrolled_window(text view)
    GtkWidget* scrolled_window;
    m_textview = newTextView(&scrolled_window);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(m_textview), FALSE);
    setTextViewLineWrap(m_textview, true);
    gtk_box_pack_start(GTK_BOX(m_container), scrolled_window, true, true, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled_window),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    // TODO: [2013-12-06] to remove
    std::string tmp = "Hello, his/her id is \"" + m_common_id + "\"\n";
    append(tmp.c_str());
}

void ChatPair::append(const char* content, msg_showing_type type)
{
    muduo::MutexLockGuard lock(m_mutex);
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_textview));
    // for appending text
    gtk_text_buffer_get_end_iter(buffer, &iter);
    // always put cursor at the end of buffer before insert
    gtk_text_buffer_place_cursor(buffer, &iter);

    if ( ! isMsgShowingTypeValid(type) ) {
        PR_WARN("invalid message show type, to use normal instead");
        type = MSG_ST_NORMAL;
    }
    const char *tag_name = getTagName(type);
    PR_TRACE("apply using tag=[%s], content=[%s]", tag_name, content);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &iter,
                                             content, -1, tag_name,
                                             NULL);

    /* Scrolls text_view the minimum distance such that
     * mark is contained within the visible area of the widget. */
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(m_textview),
                                       gtk_text_buffer_get_insert(buffer));
}


////////////////////////////////////////////////////////////////
RainbowChatGui::RainbowChatGui()
{
    PR_TRACE("constructing rainbow chat gui...");
    m_window_width = 750;
    m_window_height = 500;
    m_window_show_titlebar = true;
    m_window_title = global_app_name " chat window";
    // m_window_destroy_callback = onRainbowChatGuiWindowDestroy;

    m_side_bar.doActivatedOnSingleClick();
    m_side_bar.setSelectionFunction(ChatSideBarSelectionFunc);
    g_signal_connect(G_OBJECT(m_side_bar.getTreeView()),
                     "row-activated",
                     G_CALLBACK(onRainbowChatSidebarRowActivated),
                     NULL);
}

bool RainbowChatGui::addSidebarItem(chat_type type,
                                    void* sidebar_item_data,
                                    const char* sidebar_item_name,
                                    const char* sidebar_common_id,
                                    const char* icon_path)
{
    if ( ! isChatTypeValid(type) ) {
        PR_TRACE("invalid chat type=%d", type);
        return false;
    }
    if ( ! sidebar_item_name || ! sidebar_common_id) {
        PR_WARN("invalid argument, side bar name or sidebar common_id is NULL");
        return false;
    }
    if ( ! icon_path ) {
        icon_path = getIconDefaultPath();
    }
    // gui_data (ChatSideBarItem)
    PR_TRACE("creating sidebar item ...");
    ChatSideBarItem *sidebar_item = new ChatSideBarItem(type, sidebar_item_data);
    g_gui_data_manager.storeDataChatSideBarItem(sidebar_item);

    // chat_unities
    PR_TRACE("creating chat pair for [%s], id=[%s]...",
             sidebar_item_name,
             sidebar_common_id);
    ChatPair *chat_pair = new ChatPair(sidebar_common_id);
    // NOTICE: very important, attach new-ed chat pair to chat gui
    gtk_box_pack_start(GTK_BOX(m_io_container), chat_pair->container(),
                       true, true, 0);
    g_gui_data_manager.storeDataChatPairs(chat_pair);
    // now modify chat sidebar (store internal)
    PR_TRACE("adding stuff into sidebar...");
    return m_side_bar.add(sidebar_item_name,
                          icon_path,
                          "",          // do not show signature
                          sidebar_common_id,
                          (void*)sidebar_item,
                          (void*)chat_pair);
    // TODO: [2013-12-03] show chat pair container
    gtk_widget_show_all(chat_pair->container());
}

bool RainbowChatGui::setSidebarRowSelected(const char *common_id)
{
    if ( !common_id ) {
        PR_ERROR("invalid argument, common id NULL");
        return false;
    }
    return m_side_bar.setRowSelected(common_id);
}

// TODO: [2013-12-06] to return newly added sidebar item from argument
// bool RainbowChatGui::addBuddy(ChatSideBarItem** item, BuddyData* buddy_data)
bool RainbowChatGui::addBuddy(BuddyData* buddy_data)
{
    if ( ! buddy_data ) {
        PR_ERROR("invalid argument, buddy_data is NULL");
        return false;
    }
    const std::string& name = buddy_data->getName();
    const std::string& common_id = buddy_data->getUserIdStr();
    const char* icon_path;
    if ( buddy_data->isOnLine() ) {
        icon_path = getIconPath(ICON_TYPE_USER_ACTIVE_MIDDLE);
    } else {
        icon_path = getIconPath(ICON_TYPE_USER_INACTIVE_MIDDLE);
    }
    PR_TRACE("icon_path=%s", icon_path);
    return addSidebarItem(CHAT_TYPE_USER, (void*)buddy_data,
                          name.c_str(), common_id.c_str(),
                          icon_path);
}

bool RainbowChatGui::addGroup(GroupData* group_data)
{
    if ( ! group_data ) {
        PR_ERROR("invalid argument, group_data is NULL");
        return false;
    }
    const std::string& name = group_data->getName();
    const std::string& common_id = group_data->getGroupIdStr();
    const char* icon_path = getIconPath(ICON_TYPE_GROUP);
    return addSidebarItem(CHAT_TYPE_GROUP, (void*)group_data,
                          name.c_str(), common_id.c_str(),
                          icon_path);
}

void RainbowChatGui::compose()
{
    // gui_container(header_bar, h_container(scrolled_window (self-sidebar), m_io_container(scrolled_window(textview), entry)))
    GtkWidget * gui_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(m_window), gui_container);

    // // header bar
    // GtkWidget* header_bar = gtk_header_bar_new();
    // gtk_box_pack_start(GTK_BOX(gui_container), header_bar, false, true, 0);
    // gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), true);

    // h_container
    GtkWidget* h_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(gui_container), h_container, true, true, 0);

    // sidebar
    GtkWidget* sidebar = m_side_bar.container();
    gtk_box_pack_start(GTK_BOX(h_container), sidebar, false, true, 0);

    // separator, to separate sidebar
    GtkWidget* sidebar_separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(h_container), sidebar_separator, false, true, 0);

    // text view container
    m_io_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(h_container), m_io_container, true, true, 0);

    // box, for entry_send and button_send
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_end(GTK_BOX(m_io_container), box, false, true, 0);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);

    // add entry into box
    GtkWidget* input_container = m_input_entry.container();
    gtk_box_pack_start(GTK_BOX(box), input_container, true, true, 0);
    // separator, to separate send entry
    GtkWidget* entry_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_end(GTK_BOX(m_io_container), entry_separator, false, true, 0);
}



void RainbowChatGui::setWindowTitle(const char* chat_user)
{
    if ( ! chat_user ) {
        chat_user = "";
    }
    m_window_title = "(" + g_gui_data_manager.getSelfName() + ")";
    m_window_title += " Chatting with ";
    m_window_title += chat_user;
    m_window_title += " - " global_app_name " chat window";
    gtk_window_set_title(GTK_WINDOW(m_window),
                         m_window_title.c_str());
}

void RainbowChatGui::setSidebarSelectionFunction(GtkTreeSelectionFunc func)
{
    m_side_bar.setSelectionFunction(func);
}

void RainbowChatGui::updateInputEntry(chat_type type, void* data)
{
    PR_TRACE("to update m input entry");
    m_input_entry.update(type, data);
}

void RainbowChatGui::send(const std::string& message)
{
    PR_TRACE("to send message");
    static const std::string& from = g_gui_data_manager.getSelfUserIdStr();
    static const std::string& session_id = g_gui_data_manager.getSelfSessionId();

    std::string to;
    m_input_entry.getCommonIdStr(to);
    PR_TRACE("from=[%s], to=[%s], session id=[%s], message=[%s]",
             from.c_str(), to.c_str(), session_id.c_str(), message.c_str());
    // TODO: [2013-12-04]

    ChatMessage chat_message(from, to, message, "now", true, session_id);
    Packet packet(chat_message.bodyStr(), chat_message.getType());
    packet.sendTo(g_rainbow_client->connection());
}

// cause group has no conception of on-line or off-line. so just handle USER
bool RainbowChatGui::updateSidebarItem(const char *common_id_str, bool online)
{
    // user's buddies data is updated by rainbow gui. so here,
    // we need to update anymore, we just update the icon.
    // for more precision, to check data's status to determine
    // to update or not.
    // find user from user bar (tree store)
    PR_TRACE("finding user from user bar");
    GtkTreeIter iter;
    bool rc = m_side_bar.findRow(common_id_str, iter);
    if ( ! rc ) {
        PR_WARN("user not found in user bar, common id", common_id_str);
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
    return m_side_bar.update(iter, true, icon_path);
}

void RainbowChatGui::updateChatNormalMessage(const ChatMessage& chat_msg)
{
    // success message, echo or non-echo
    const std::string& from = chat_msg.getFromStr();
    const std::string& to = chat_msg.getToStr();
    const std::string& time = chat_msg.getTimeStr();
    std::string content = chat_msg.getContent();
    const std::string& self_id = g_gui_data_manager.getSelfUserIdStr();
    bool is_echo = (from == self_id);

    PR_TRACE("from=%s, to=%s, content=%s, is_echo=%s, time=%s",
             from.c_str(), to.c_str(), content.c_str(),
             is_echo ? "yes" : "no", time.c_str());
    // get common id
    const char* common_id;
    if ( is_echo ) {
        common_id = to.c_str();
    } else {
        common_id = from.c_str();
    }
    // find chat pair by 'common id'
    ChatPair* chat_pair;
    chat_pair = g_gui_data_manager.findChatPair_Guard(common_id);
    // make sure chat window is created
    if ( ! chat_pair && g_gui_data_manager.isChatWindowCreated() ) {
        PR_WARN("chat pair of common id=%s not added to sidebar yet",
                common_id);
        // TODO: [2013-12-06] for single chat, not for group chat
        BuddyData* buddy_data = g_gui_data_manager.findBuddy_Guard(common_id);
        if ( ! buddy_data ) {
            // TODO: [2013-12-06] to show this warning in GUI
            PR_WARN("coming message contains anonymous user whose id is=%d, "
                    "suggest to put this user in your black list",
                    common_id);
            return;
        }
        bool rc = g_chat_gui.addBuddy(buddy_data);
        if ( ! rc ) {
            PR_ERROR("failed to add buddy=[%s] to chat window side bar",
                     buddy_data->getName().c_str());
            return;
        }
        buddy_data->setChatShowing(true);
        chat_pair = g_gui_data_manager.findChatPair_Guard(common_id);
    }
    ////////////////////////////////////////////////////////////////
    // compose content for showing
    std::string showing_name;
    std::string head;

    if ( is_echo ) {
        showing_name = g_gui_data_manager.getSelfName();
        head = " (" + time + ")" + showing_name + "\n";
        chat_pair->append((content + " ").c_str(), MSG_ST_ECHO_CONTENT);
        chat_pair->append(head.c_str(), MSG_ST_ECHO_HEAD);
    } else {
        showing_name = g_gui_data_manager.getBuddyName_Guard(from.c_str());
        head = showing_name + " (" + time +") ";
        chat_pair->append(head.c_str(), MSG_ST_NON_ECHO_HEAD);
        chat_pair->append((content + "\n").c_str(), MSG_ST_NON_ECHO_CONTENT);
    }
}
void RainbowChatGui::updateChatErrorMessage(const ChatMessage&  chat_msg)
{
    // success message, echo or non-echo
    const std::string& from = chat_msg.getFromStr();
    const std::string& to = chat_msg.getToStr();
    const std::string& self_id = g_gui_data_manager.getSelfUserIdStr();
    bool is_echo = (from == self_id);

    PR_TRACE("from=%s, to=%s, is_echo=%s",
             from.c_str(), to.c_str(), is_echo ? "yes" : "no");
    // get common id
    if ( ! is_echo ) {
        PR_WARN("for chat error message, is echo can only be true");
        return;
    }
    const char* common_id = to.c_str();
    ChatPair* chat_pair = NULL;
    PR_TRACE("to find chat pair by common id=%s", common_id);
    chat_pair = g_gui_data_manager.findChatPair_Guard(common_id);
    // make sure chat window is created
    if ( ! g_gui_data_manager.isChatWindowCreated() ) {
        PR_WARN("chat window not created yet");
        return;
    }
    PR_TRACE("chat pair exist? [%s]", chat_pair ? "yes" : "no");
    if ( ! chat_pair ) {
        PR_WARN("chat pair of common id=%s not added to sidebar yet",
                common_id);
        // TODO: [2013-12-06] for single chat, not for group chat
        BuddyData* buddy_data = g_gui_data_manager.findBuddy_Guard(common_id);
        if ( ! buddy_data ) {
            // TODO: [2013-12-06] to show this warning in GUI
            PR_WARN("coming message contains anonymous user whose id is=%d, "
                    "suggest to put this user in your black list",
                    common_id);
            return;
        }
        bool rc = g_chat_gui.addBuddy(buddy_data);
        if ( ! rc ) {
            PR_ERROR("failed to add buddy=[%s] to chat window side bar",
                     buddy_data->getName().c_str());
            return;
        }
        buddy_data->setChatShowing(true);
        chat_pair = g_gui_data_manager.findChatPair_Guard(common_id);
    }
    ////////////////////////////////////////////////////////////////
    // TODO: [2013-12-09] to do a line
    std::string error_info = chat_msg.getErrorInfo();
    chat_pair->append((error_info+"\n").c_str(), MSG_ST_ERROR);
}

void RainbowChatGui::updateChatMessage(const ChatMessage& chat_msg)
{
    if (chat_msg.isErrorMessage()) {
        PR_TRACE("to update error chat message");
        updateChatErrorMessage(chat_msg);
    } else {
        PR_TRACE("to update normal chat message");
        updateChatNormalMessage(chat_msg);
    }
}
