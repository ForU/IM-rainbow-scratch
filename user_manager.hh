#ifndef INCLUDE_USER_MANAGER_HPP
#define INCLUDE_USER_MANAGER_HPP

#include "user_data.hh"
#include "trie_tree.hh"
#include "singleton.hh"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/extra/utils.h>
#include <vector>

// UserManager manages TrieTree<UserRefer> and the storage of a list of
// UserDataDetail.
//
// when start, UserManager loads the detail information of all users from
// database or files, and create a basic trie tree with node' data properly
// initialized, which means each node's data pointer points to right detail
// storage.
//
// manager supply operations on user's detail data the trie tree.
// addUser:
// virtualDeleteUser:
// updateUser:
//


extern int getUserIdIndex(char c);
const int UserIdDim = 10;
extern bool isUserIdCharSupported(char c);

extern int getUserNameReferIndex(char c);
// NOTICE: change with function: getUserNameReferIndex's char set
const int UserNameReferDim = 65;
extern const char *UserNameSupportedChars;
extern bool isUserNameCharSupported(char c);


#define G_user_manager SingletonX<UserManager>::instance()

class UserManager
{
public:
    // basic, the right way is: UserManager has a configure file which defines
    // the user_info_file and the basic database info:
    // server_ip, access_user, database_name, table_name, ...
    // TODO: [2013-11-12] Here we just support the most simple one: for test
    UserManager() {}
    ~UserManager() {}

    void init();

    ////////////////////////////////////////////////////////////////
    // high-level API
    ////////////////////////////////////////////////////////////////
    bool newUser(UserId& user_id,
                 const std::string& register_name,
                 const std::string& password,
                 const std::string& signature="",
                 const std::string& email_val="");

    ////////////////////////////////////////////////////////////////
    // user part
    ////////////////////////////////////////////////////////////////
    UserDataDetail& getUser(const UserId& user_id);
    const UserDataDetail& getUser(const UserId& user_id) const;

    bool virtualDeleteUser(const UserId& user_id);
    bool updateUser(const UserId& user_id); // TODO: [2013-11-12]

    bool makeFriends(const UserId& user_id1,
                     const UserId& user_id2,
                     const std::string& category="Friends",
                     bool& is_already_friend=s_is_friend_obj);

    bool eraseFriends(const UserId& user_id1, const UserId& user_id2);

    // bool joinGroups(const UserId& user_id, const GroupId& group_id);
    bool getFriendsRefers(const UserId& user_id, const std::vector<UserRefer>& refers) const;
    UserRefer& getRefer(const UserId& user_id);
    const UserRefer& getRefer(const UserId& user_id) const;

    void showUser(const UserId& user_id) const;
    void showGroups(const UserId& user_id) const;
    void showSelfInformation(const UserId& user_id) const;
    void showFriends(const UserId &user_id) const;
    void updateStatus(const UserId& user_id, e_status status);
    void updateConnection(const UserId& user_id, const muduo::net::TcpConnectionPtr& connection);
    void updateReferBasic(const UserId& user_id, e_status status, const muduo::net::TcpConnectionPtr& connection);

    ////////////////////////////////////////////////////////////////
    // user Name part
    ////////////////////////////////////////////////////////////////
    bool nameOccupied(const std::string& name);

private:
    bool addUser(const UserDataDetail& user_detail);
    bool addUser(const UserId& user_id, const std::string& register_name, const std::string& password=USER_DETAIL_DEFAULT_PASSWORD, const std::string& signature="", const std::string& email_val="");

    bool addUserRefer(const UserDataDetail& user_detail);
    bool addUserRefer(const UserId& user_id, const std::string& register_name, const std::string& password=USER_DETAIL_DEFAULT_PASSWORD, const std::string& signature="");
    bool addUserNameRefer(const UserId& user_id, const std::string& name);

private:
    mutable muduo::MutexLock m_user_mutex; // for m_refers
    mutable muduo::MutexLock m_name_mutex; // for m_name_refers
    TrieTree<UserRefer, UserIdDim, getUserIdIndex> m_refers;
    TrieTree<UserNameRefer, UserNameReferDim, getUserNameReferIndex> m_name_refers;

    static UserRefer s_barebone_user_refer;
    static UserDataDetail s_barebone_user_detail;
    static bool s_is_friend_obj;
};

#endif /* INCLUDE_USER_MANAGER_HPP */

