#ifndef INCLUDE_SESSION_MANAGER_HPP
#define INCLUDE_SESSION_MANAGER_HPP

#include <muduo/base/Mutex.h>
#include "trie_tree.hh"
#include "user_data.hh"
#include "singleton.hh"

// used to manage the relationship of TCP connector and on-line User refer

const int SockfdDim=10;
extern int getSockfdIndex(char c);

class ConnectorRefer
{
public:
    ConnectorRefer(int sockfd=0, const UserId& user_id=USER_ID_INITIALIZER);
    ~ConnectorRefer() {}

    void setSockfd(int sockfd);
    void setUserId(const std::string& user_id);
    void setUserId(const UserId& user_id);

    bool isUserIdUpdated() const { return ! m_user_id.barebone(); }
    // get user id
    UserId& getUserId(bool& updated=s_updated) {
        updated = (! m_user_id.barebone());
        return m_user_id;
    }
    const UserId& getUserId(bool& updated=s_updated) const {
        updated = (! m_user_id.barebone());
        return m_user_id;
    }
    // get sock fd
    std::string& getSockfdStr() { return m_sockfd; }
    const std::string& getSockfdStr() const { return m_sockfd; }
    int getSockfd() const { return atoi(m_sockfd.c_str()); }

private:
    std::string m_sockfd;
    UserId m_user_id;
    static bool s_updated;
};

#define G_session_manager SingletonX<SessionManager>::instance()

class SessionManager
{
public:
    SessionManager() {}
    ~SessionManager() {}

    bool addConnector_Guard(int sockfd) {
        muduo::MutexLockGuard lock(m_mutex);
        return addConnector(sockfd);
    }
    bool updateConnectorUserId_Guard(int sockfd, const UserId& user_id) {
        muduo::MutexLockGuard lock(m_mutex);
        return updateConnectorUserId(sockfd, user_id);
    }
    bool updateConnectorUserId_Guard(int sockfd, const std::string& user_id_str) {
        UserId user_id = user_id_str.c_str();
        return updateConnectorUserId_Guard(sockfd, user_id);
    }
    bool getUserId_Guard(int sockfd, UserId& user_id, bool& is_uid_upadated=s_updated) {
        muduo::MutexLockGuard lock(m_mutex);
        return getUserId(sockfd, user_id, is_uid_upadated);
    }

    // handle off-line connection:
    // 1. clear connector from session manager
    // 2. if user's login, then server send login Status(off)
    //    Notification to friends
    bool handleDownConection_Guard(int sockfd);

private:
    bool addConnector(int sockfd);
    bool getUserId(int sockfd, UserId& user_id, bool& is_uid_upadated=s_updated);
    bool updateConnectorUserId(int sockfd, const UserId& user_id);
    bool clearConnector(int sockfd);

private:
    TrieTree<ConnectorRefer, SockfdDim, getSockfdIndex> m_refers;
    mutable muduo::MutexLock m_mutex;   // protect Session Manager
    static bool s_updated;
};
#endif /* INCLUDE_SESSION_MANAGER_HPP */
