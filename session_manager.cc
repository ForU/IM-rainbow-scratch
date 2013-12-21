#include "global.hh"            // for G_DEFAULT_INDEX
#include "session_manager.hh"
#include "user_manager.hh"
#include "packet_manager.hh"


int getSockfdIndex(char c)
{
    int idx = G_DEFAULT_INDEX;
    if ( c >= '0' && c <= '9' ) {
        idx = c - '0';
    }
    else {
        LOG_WARN << "unsupported character=\'" << c << "\'";
        idx = G_DEFAULT_INDEX;
    }
    return idx;
}

static
void int2str(std::string& rst_string, int int_val)
{
#define SIZE 12
    char buf[SIZE];
    memset(buf, sizeof buf, 0);
    snprintf(buf, sizeof buf, "%d", int_val);
    rst_string = buf;
}

bool ConnectorRefer::s_updated = false;

ConnectorRefer::ConnectorRefer(int sockfd, const UserId& user_id)
{
    setSockfd(sockfd);
    setUserId(user_id);
}

void ConnectorRefer::setSockfd(int sockfd)
{
    if ( sockfd < 0 ) {
        LOG_WARN << "invalid argument, sockfd <  0";
        return;
    }
    LOG_TRACE << "converting INT(sockfd=" << sockfd << ") to string";
    int2str(m_sockfd, sockfd);
    LOG_TRACE << "successfully converted, sockfd string=" << m_sockfd;
}

void ConnectorRefer::setUserId(const std::string& user_id)
{
    m_user_id = user_id;
}

void ConnectorRefer::setUserId(const UserId& user_id)
{
    m_user_id = user_id;
}

bool SessionManager::s_updated = false;
bool SessionManager::addConnector(int sockfd)
{
    ConnectorRefer refer((unsigned long)sockfd);
    bool rc = m_refers.insertOrUpdate(refer.getSockfdStr().c_str(), refer);
    if ( ! rc ) {
        LOG_ERROR << "failed to add connector=" << refer.getSockfdStr();
        return false;
    }
    LOG_TRACE << "successfully add connector=" << refer.getSockfdStr();
    return true;
}

bool SessionManager::updateConnectorUserId(int sockfd, const UserId& user_id)
{
    std::string sockfd_str;
    int2str(sockfd_str, sockfd);

    ConnectorRefer* refer = m_refers.find(sockfd_str.c_str());
    if ( ! refer ) {
        LOG_WARN << "not found, connector refer=" << sockfd_str;
        return false;
    }
    refer->setUserId(user_id);
    LOG_TRACE << "successfully update for connector="
              << refer->getSockfdStr()
              << ", use_id=" << refer->getUserId().id();
    return true;
}


bool SessionManager::getUserId(int sockfd, UserId& user_id, bool& is_uid_upadated)
{
    std::string sockfd_str;
    int2str(sockfd_str, sockfd);

    const ConnectorRefer* refer = m_refers.find(sockfd_str.c_str());
    if ( ! refer ) {
        LOG_WARN << "not found, connector refer=[" << sockfd_str << "]";
        return false;
    }
    user_id = refer->getUserId(is_uid_upadated);
    LOG_TRACE << "successfully get use_id=" << user_id.id();
    if ( is_uid_upadated ) {
        LOG_DEBUG << "user id updated";
    } else {
        LOG_DEBUG << "user id not updated";
    }
    return true;
}

bool SessionManager::clearConnector(int sockfd)
{
    // TODO: [2013-12-06]
    return true;
}

// using no guard function inside
bool SessionManager::handleDownConection_Guard(int sockfd)
{
    bool rc = clearConnector(sockfd);
    if ( ! rc ) {
        LOG_ERROR << "failed to clear connector=" <<  sockfd;
        return false;
    }

    // check if user's login
    bool is_uid_upadated;
    UserId self_user_id;
    if ( ! getUserId_Guard(sockfd, self_user_id, is_uid_upadated) ) {
        LOG_ERROR << "failed to get user id using sockfd=" <<  sockfd;
        return false;
    }
    if ( ! is_uid_upadated ) {
        LOG_WARN << "user id not updated, that is, not login session";
        return true;
    }

    LOG_TRACE << "handle off-line notification, update user manager";
    G_user_manager.updateStatus(self_user_id, E_STATUS_OFFLINE);

    // TODO: [2013-12-06] merge into PacketManager
    // notify all on-line buddies that "I'am off line now"
    LOG_TRACE << "send off-line notification to on-line friends";
    LoginStatusNotification offline_notf(self_user_id, E_STATUS_OFFLINE);
    std::string notf_str;
    offline_notf.compose(notf_str);
    Packet packet(notf_str, offline_notf.getType());
    // find buddies
    const UserMap& buddies = G_user_manager.getUser(self_user_id).buddies();
    UserMapConstIterator it = buddies.begin();
    for ( ; it != buddies.end(); ++it ) {
        UserId buddy_user_id = it->first.c_str();
        UserRefer& buddy = G_user_manager.getRefer(buddy_user_id);
        if ( buddy.barebone() ) {
            LOG_WARN << "not exist user=" << buddy_user_id.id()
                     << ", this should never happen here";
            continue;
        }
        if ( buddy.online() ) {
            LOG_TRACE << "buddy=[" << buddy_user_id.id()
                      << "] is on-line, to send notification=["
                      << notf_str << "]";
            packet.sendTo(buddy.connection());
        } else {
            LOG_TRACE << "buddy=[" << buddy_user_id.id() << "] is off-line";
        }
    }
    return true;
}
