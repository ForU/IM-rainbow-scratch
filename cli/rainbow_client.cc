#include "../packet_manager.hh"
#include "rainbow_gui.hh"
#include "rainbow_global.hh"
#include "log.hh"

using namespace muduo;
using namespace muduo::net;

muduo::net::TcpConnectionPtr RainbowClient::s_tcp_connection;

RainbowClient::RainbowClient(EventLoop* loop, const InetAddress& serverAddr)
    : m_client(loop, serverAddr, "RainbowClient"),
      m_connection(s_tcp_connection)
{
    m_client.setConnectionCallback( boost::bind(&RainbowClient::onConnection, this, _1));
    m_client.setMessageCallback(boost::bind(&RainbowClient::onRainbowResponse, this, _1, _2, _3));
    // m_client.enableRetry();
}

void RainbowClient::connect()
{
    PR_TRACE("rainbow client to connect to server");
    m_client.connect();

}
void RainbowClient::disconnect()
{
    m_client.disconnect();
}

TcpConnectionPtr& RainbowClient::connection()
{
    return m_connection;
}

void RainbowClient::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    MutexLockGuard lock(m_mutex);
    if (conn->connected()) {
        m_connection = conn;
    } else {
        // m_connection.reset();
    }
}

gboolean onLoginStatusNotification(gpointer user_data)
{
    std::string local_msg_body = (char*)user_data;
    PR_TRACE("store as local message body=%s", local_msg_body.c_str());

    LoginStatusNotification notf(local_msg_body);
    PR_TRACE("type: %d", (e_message_type)notf.getType());

    bool online = notf.isOnLineNotification();
    const char* from_id = notf.getFromStr().c_str();

    // update rainbow window
    PR_TRACE("to rainbow GUI update buddy");
    g_rainbow_gui.updateBuddy(from_id, online);
    PR_TRACE("to update category data");
    BuddyData* buddy = g_gui_data_manager.findBuddy_Guard(from_id);
    if ( ! buddy ) {
        PR_ERROR("failed to buddy with id=%s", from_id);
        return FALSE;
    }
    category_info_action action;
    action = online ? CATEGORY_INFO_ACTION_PLUS_ON :
        CATEGORY_INFO_ACTION_MINUS_ON;
    const std::string& category = buddy->getCategory();
    g_gui_data_manager.handleCategoryInfo_Guard(category, action , 1);
    g_rainbow_gui.updateCategoryInfo(category.c_str());

    // update rainbow chat window
    PR_TRACE("to update rainbow chat GUI sidebar");
    if ( g_gui_data_manager.isChatWindowCreated() ) {
        g_chat_gui.updateSidebarItem(from_id, online);
    }
    // always return FALSE for GSourceFunc
    return FALSE;
}

gboolean onChatMessage(gpointer user_data)
{
    std::string local_msg_body = (char*)user_data;
    PR_TRACE("store as local message body=%s", local_msg_body.c_str());
    // is from server
    ChatMessage chat_msg(local_msg_body, false);
    ////////////////////////////////////////////////////////////////
    // check if chat window is created
    if ( ! g_gui_data_manager.isChatWindowCreated() ) {
        PR_TRACE("chat window not created yet, created here");
        g_chat_gui.create();
        g_gui_data_manager.setChatWindowCreated();
        g_chat_gui.show();
    }
    g_chat_gui.updateChatMessage(chat_msg);
    // always return FALSE for GSourceFunc
    return FALSE;
}

gboolean onFriendsManagementResponse(gpointer user_data)
{
    std::string local_msg_body = (char*)user_data;
    PR_TRACE("store as local message body=%s", local_msg_body.c_str());
    // is from server
    FriendsManagementResponse resp(local_msg_body);
    SearchWidgetBar& search_bar = g_rainbow_gui.getSearchWidget();
    if ( resp.isErrorMessage() ) {
        search_bar.setNotification(resp.getErrorInfo().c_str(),
                                   E_NOTIFICATION_ERROR);
        return FALSE;
    }

    ////////////////////////////////////////////////////////////////
    e_friends_manage_action action = resp.getRequestAction();
    switch (action) {
    case FRIENDS_MANAGE_ACTION_SEARCH: {
        PR_TRACE("action is search, to show user into in search bar",
                 resp.getTargetName().c_str());
        g_rainbow_gui.handleSearchResultInSearchBar(resp);
        break;
    }
    case FRIENDS_MANAGE_ACTION_ADD: {
        PR_TRACE("action is add, to add user into user bar, "
                 "and handle the search barh",
                 resp.getTargetName().c_str());

        // handle search bar, to hide it
        search_bar.clearNotificationArea();
        search_bar.hide();

        // first of all, update g_gui_data_manager data
        PR_TRACE("update category statics for data manager");
        const std::string& category = resp.getTargetCategory();
        g_gui_data_manager.handleCategoryInfo_Guard( category, CATEGORY_INFO_ACTION_PLUS_TOTAL, 1);
        if ( resp.isTargetOnline() ) {
            g_gui_data_manager.handleCategoryInfo_Guard(category, CATEGORY_INFO_ACTION_PLUS_ON, 1);
        }

        // << buddy data
        PR_TRACE("add buddy into data manager");
        const std::string& user_id_str = resp.getTargetIdStr();
        const std::string& name = resp.getTargetName();
        const std::string& signature = resp.getTargetSignature();
        const std::string& status = resp.getTargetStatus();
        BuddyData *data = new BuddyData( user_id_str,
                                         name,
                                         signature,
                                         status,
                                         category);
        g_gui_data_manager.storeBuddyData(data);

        // update rainbow gui by buddy
        PR_TRACE("update rainbow gui by buddy");
        g_rainbow_gui.addBuddy(*data, data);

        // update rainbow gui by category statics
        PR_TRACE("update rainbow gui by category=%s statics", category.c_str());
        g_rainbow_gui.updateCategoryInfo(category.c_str());

        break;
    }
    case FRIENDS_MANAGE_ACTION_DELETE: {
        PR_TRACE("action is delete, to delete user=%d from user bar",
                 resp.getTargetName().c_str());
        break;
    }
    default: {
        PR_TRACE("unknown request action=%d", (int)action);
        return FALSE;
    }
    }

    return FALSE;
}

void RainbowClient::onRainbowResponse(const TcpConnectionPtr& conn,
                                      Buffer* buf,
                                      Timestamp receiveTime)
{
    // maybe lock the rainbow button here or in callback function.
    PR_TRACE("rainbow response coming..., to handle it");
    Packet response_packet;
    if ( ! response_packet.extract(buf) ) {
        PR_ERROR("failed to extract response from server");
        return;
    }
    PR_TRACE("message type=[%d], body=[%s]",
             response_packet.messageType(),
             response_packet.messageBody().c_str());

    e_message_type msg_type = response_packet.messageType();
    const std::string& msg_body = response_packet.messageBody();

    if ( msg_body.size() > MSG_BUF_SIZE ) {
        std::string error_info = "too large message body, will be throw away";
        PR_WARN("%s", error_info.c_str());
        g_login_gui.setNotification(error_info.c_str(), E_NOTIFICATION_ERROR);
        return;
    }

    // protect static memory
    MutexLockGuard _lock(m_mutex);
    // use static for memory performance
    static char msg_buf[MSG_BUF_SIZE];
    strncpy(msg_buf, msg_body.c_str(), sizeof msg_buf);

    switch (msg_type) {
    case E_MESSAGE_TYPE_LOGIN_RESPONSE: {
        PR_TRACE("to parse login response");
        pthread_mutex_lock(&g_mutex_login_response);
        if ( g_login_response ) {
            PR_TRACE("erase old login response");
            delete g_login_response;
        }
        g_login_response = new LoginResponse(msg_body);
        pthread_mutex_unlock(&g_mutex_login_response);

        PR_TRACE("to signal that login response received");
        pthread_cond_signal(&g_condtion_login_res_received);
        break;
    }
    case E_MESSAGE_TYPE_NOTIFICATION_LOGIN_STATUS: {
        PR_TRACE("login notification received, to update rainbow's user bar");
        int id = g_idle_add((GSourceFunc)onLoginStatusNotification, (void*)msg_buf);
        PR_TRACE("g_idle_add a source function, id=%d", id);
        break;
    }
    // both for chat response and chat
    case E_MESSAGE_TYPE_CHAT_RESPONSE:
    case E_MESSAGE_TYPE_CHAT: {
        int id = g_idle_add((GSourceFunc)onChatMessage, (void*)msg_buf);
        PR_TRACE("g_idle_add a source function for chat message, id=%d", id);
        break;
    }
    case E_MESSAGE_TYPE_FRIENDS_MANAGEMENT_RESPONSE: {
        int id = g_idle_add((GSourceFunc)onFriendsManagementResponse,
                            (void*)msg_buf);
        PR_TRACE("g_idle_add a source function for add friend, id=%d", id);
        break;
    }

    default: {
        PR_TRACE("unknown message type received");
        break;
    }
    }
}

