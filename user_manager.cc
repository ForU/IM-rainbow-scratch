#include "user_manager.hh"

UserRefer UserManager::s_barebone_user_refer;
UserDataDetail UserManager::s_barebone_user_detail;
bool UserManager::s_is_friend_obj = false;

int getUserIdIndex(char c)
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

bool isUserIdCharSupported(char c)
{
    return (c >= '0' && c <= '9');
}

const char *UserNameSupportedChars = "'0-9', 'a-z', 'A-Z', ' ', '@', '.'";

bool isUserNameCharSupported(char c)
{
    return (G_DEFAULT_INDEX != getUserNameReferIndex(c));
}

int getUserNameReferIndex(char c)
{
    // valid name chars:
    // char  => idx
    // "0-9" => [0-9]
    // "A-Z" => [10 - 35]
    // "a-z" => [36 - 61]
    // "space" => 62
    // "@" => 63
    // "." => 64
    // so, range is [0-64], dim is 65
    int idx = G_DEFAULT_INDEX;
    if ( c >= '0' && c <= '9' ) {
        idx = c - '0';
    } else if ( c >= 'A' && c <= 'Z' ) {
        idx = c - 55;         // -65(A) + 10
    } else if ( c >= 'a' && c <= 'z' ) {
        idx = c - 61;         // -97(a) + 36
    } else if ( c == ' ') {
        idx = 62;              // 62
    } else if ( c == '@' ) {
        idx = 63;
    } else if ( c == '.' ) {
        idx = 64;
    } else {
        LOG_ERROR << "unsupported character=\'" << c << "\'";
        idx = G_DEFAULT_INDEX;
    }
    return idx;
}

bool UserManager::newUser(UserId& user_id,   // return user id
                          const std::string& register_name,
                          const std::string& password,
                          const std::string& signature,
                          const std::string& email_val)
{
    // new user id
    user_id = G_userid_generator.generate();
    if ( user_id.barebone() ) {
        LOG_ERROR << "barebone user id generated by generator, this should not happen";
        return false;
    }
    LOG_DEBUG << "successfully generate user id=" << user_id.id();
    // add User (both user refer and user name refer)
    return addUser(user_id, register_name, password, signature, email_val);
}

bool UserManager::addUser(const UserDataDetail& user_detail)
{
    return addUser(user_detail.userId(),
                   user_detail.registerName(),
                   user_detail.password(),
                   user_detail.signature(),
                   user_detail.email());
}

bool UserManager::addUser(const UserId& user_id,
                          const std::string& register_name,
                          const std::string& password,
                          const std::string& signature,
                          const std::string& email_val)
{
    if ( ! addUserRefer(user_id, register_name, password, signature) ) {
        LOG_ERROR << "failed to add user refer, user=" << user_id.id()
                  << ", name=" << register_name;
        return false;
    }
    LOG_TRACE << "successfully add user refer, user=" << user_id.id()
              << ", name=" << register_name;
    if ( ! addUserNameRefer(user_id, register_name) ) {
        LOG_ERROR << "failed to add user name refer, user=" << user_id.id()
                  << ", name=" << register_name;
        return false;
    }
    LOG_TRACE << "successfully add user name refer, user=" << user_id.id()
              << ", name=" << register_name;
    return true;
}

bool UserManager::addUserRefer(const UserDataDetail& user_detail)
{
    return addUserRefer(user_detail.userId(),
                        user_detail.registerName(),
                        user_detail.password(),
                        user_detail.signature());
}

// add a pure user
bool UserManager::addUserRefer(const UserId& user_id,
                               const std::string& register_name,
                               const std::string& password,
                               const std::string& signature)
{
    LOG_INFO << "creating and storing user refer information: "
             << "user id:[" << user_id.id()
             << "] register name:["<< register_name
             << "] signature:[" << signature <<"]";

    // 1. handle user refer
    UserDataDetail user_detail(user_id, register_name, password, signature);
    UserRefer user_refer;

    user_refer.setDetail(user_detail);
    if ( ! m_refers.insert(user_id.id(), user_refer) ) {
        LOG_ERROR << "failed to add user=" << user_refer.detail().userId().id();
        return false;
    }
    LOG_INFO << "successfully add user=" << user_refer.detail().userId().id();
    return true;
}

void UserManager::init()
{
    // data emulated load from database
    addUser(UserId(1), "Jacoo Lee", USER_DETAIL_DEFAULT_PASSWORD, "hello everyone");
    addUser(UserId(2), "Tommy", USER_DETAIL_DEFAULT_PASSWORD, "Sometimes you need to step outside, clear your head, and remind yourself of who you are and where you wanna be.");
    addUser(UserId(3), "Michael", USER_DETAIL_DEFAULT_PASSWORD, "Death comes to all, but great achievements raise a monument which shall endure until the sun grows old.");
    addUser(UserId(4), "Lily", USER_DETAIL_DEFAULT_PASSWORD, "I am Lily");
    addUser(UserId(5), "Jason", USER_DETAIL_DEFAULT_PASSWORD, "I am Jason");
    addUser(UserId(6), "Stan", USER_DETAIL_DEFAULT_PASSWORD, "I am stan");
    addUser(UserId(7), "Jessica", USER_DETAIL_DEFAULT_PASSWORD, "I am Jessica");
    addUser(UserId(8), "Winston", USER_DETAIL_DEFAULT_PASSWORD, "I am Winston");
    addUser(UserId(9), "Stevenson", USER_DETAIL_DEFAULT_PASSWORD, "I am Stevenson");
    addUser(UserId(10), "Ten", USER_DETAIL_DEFAULT_PASSWORD, "Ten");
    addUser(UserId(5467899), "XX5467899", USER_DETAIL_DEFAULT_PASSWORD, "");


    // make friends
    // A, B are Friends, B, C are Friends
    UserId id1(1), id2(2), id3(3), id4(4), id5(5);
    UserId id6(6), id7(7), id8(8), id9(9), id5467899(5467899);

    makeFriends(id1, id2, "basketball");
    makeFriends(id1, id3, "Family");
    makeFriends(id1, id4, "Family");
    makeFriends(id1, id5, "Buddies");
    makeFriends(id1, id6, "Buddies");
    makeFriends(id1, id7, "Buddies");
    makeFriends(id1, id8, "Buddies");
    makeFriends(id1, id9, "Buddies");
    makeFriends(id1, id5467899, "Chess Friends");

    makeFriends(id2, id3);
    makeFriends(id2, id4);
    makeFriends(id2, id5);
    makeFriends(id2, id6);
    makeFriends(id2, id7);
    makeFriends(id2, id8);
    makeFriends(id2, id9);

    makeFriends(id3, id4);
    makeFriends(id3, id5);
    makeFriends(id4, id5);
}

UserDataDetail& UserManager::getUser(const UserId& user_id)
{
    LOG_TRACE << "getting user: [" << user_id.id() <<"]";
    UserRefer& refer = getRefer(user_id);
    if ( refer.barebone() ) {
        LOG_WARN << "not found, user=[" << user_id.id() << "]";
        return s_barebone_user_detail;
    }
    LOG_INFO << "user [" << refer.detail().userId().id() << "] founded";
    return refer.detail();
}

const UserDataDetail& UserManager::getUser(const UserId& user_id) const
{
    LOG_TRACE << "getting user: [" << user_id.id() <<"]";
    const UserRefer& refer = getRefer(user_id);
    if ( refer.barebone() ) {
        LOG_WARN << "not found, user=[" << user_id.id() << "]";
        return s_barebone_user_detail;
    }
    LOG_INFO << "user [" << refer.detail().userId().id() << "] founded";
    return refer.detail();
}

bool UserManager::eraseFriends(const UserId& user_id1, const UserId& user_id2)
{
    const UserDataDetail &ud1 = getUser(user_id1);
    const UserDataDetail &ud2 = getUser(user_id2);
    if ( ud1.barebone() ) {
        LOG_WARN << "not created yet, user=[" << user_id1.id() << "]";
        return false;
    }
    if ( ud2.barebone() ) {
        LOG_WARN << "not created yet, user=[" << user_id2.id() << "]";
        return false;
    }

    // lock from here
    UserDataBrief user_brief1(ud1.userId(), ud1.registerName(), ud1.signature());
    UserDataBrief user_brief2(ud2.userId(), ud2.registerName(), ud2.signature());

    // FIXME: [2013-11-15]
    LOG_INFO << user_brief1.userId().id();
    LOG_INFO << user_brief2.userId().id();

    // ud1.deleteFriend(user_brief2);
    // ud2.deleteFriend(user_brief1);

    return true;
}

bool UserManager::makeFriends(const UserId& user_id1,
                              const UserId& user_id2,
                              const std::string& category,
                              bool& is_already_friend)
{
    // 1. find each user's refer
    // 2. change detail(refereed by refer) information about Friends
    UserDataDetail &ud1 = getUser(user_id1);
    if ( ud1.barebone() ) {
        LOG_WARN << "not such user=[" <<  user_id1.id() <<  "]";;
        return false;
    }
    bool rc = ud1.isMyFriend(user_id2);
    if ( rc  ) {                // already friend
        is_already_friend = true;
        LOG_TRACE << "they are already friends? " << is_already_friend;
        return true;
    }
    UserDataDetail &ud2 = getUser(user_id2);
    if ( ud2.barebone() ) {
        LOG_WARN << "not such user=[" <<  user_id2.id() <<  "]";;
        return false;
    }
    UserDataBrief user_brief1(ud1.userId(), ud1.registerName(), ud1.signature(), category);
    UserDataBrief user_brief2(ud2.userId(), ud2.registerName(), ud2.signature(), category);

    ud2.addFriend(user_brief1);
    ud1.addFriend(user_brief2);

    LOG_INFO << "successfully making " << user_id1.id() << " "<< user_id2.id() << "as Friends";
    return true;
}

bool UserManager::getFriendsRefers(const UserId& user_id,
                                   const std::vector<UserRefer>& refers) const
{
    // find detail about user USER_ID
    // get map of friends
    // for each friend, get its detail by it userid
    // TODO: [2013-11-12]
    return true;
}

UserRefer& UserManager::getRefer(const UserId& user_id)
{
    UserRefer *refer = m_refers.find(user_id.id());
    if ( NULL == refer ) {
        LOG_WARN << "not found, refer=[" << user_id.id()<< "]";
        return s_barebone_user_refer;
    }
    LOG_INFO << "refer [" << refer->detail().userId().id()<< "] found";
    return *refer;
}

const UserRefer& UserManager::getRefer(const UserId& user_id) const
{
    const UserRefer *refer = m_refers.find(user_id.id());
    if ( NULL == refer ) {
        LOG_WARN << "not found, refer=[" << user_id.id()<< "]";
        return s_barebone_user_refer;
    }
    LOG_WARN << "found, refer=[" << refer->detail().userId().id()<< "]";
    return *refer;
}

void UserManager::showUser(const UserId& user_id) const
{
    LOG_INFO << "showing user [" << user_id.id() << "] information:";
    showSelfInformation(user_id);
    showFriends(user_id);
    showGroups(user_id);
}

void UserManager::showGroups(const UserId& user_id) const
{
    LOG_INFO << "showing user [" << user_id.id() << "] group information:";
}

void UserManager::showSelfInformation(const UserId& user_id) const
{
    LOG_INFO << "showing user [" << user_id.id() << "] self information:";
    const UserDataDetail &user_detail = getUser(user_id);
    if ( user_detail.barebone() ) {
        LOG_WARN << "not created yet, user=[" << user_id.id() << "]";
        return;
    }
    LOG_INFO << "user id:" << "[" << user_detail.userId().id() << "], "
             << "register name:" << "[" << user_detail.registerName() << "], "
             << "password:" << "[" << user_detail.password() << "], "
             << "signature:" << "[" << user_detail.signature() << "]";
}

void UserManager::showFriends(const UserId &user_id) const
{
    LOG_INFO << "showing user [" << user_id.id() << "] friends information:";
    const UserDataDetail &ud_ref = getUser(user_id);
    if ( ud_ref.barebone() ) { return; }

    const UserMap &friends = ud_ref.buddies();

    int cnt = 0;
    UserMapConstIterator it;
    for ( it = friends.begin(); it != friends.end(); ++it ) {
        LOG_INFO << "user:[" << user_id.id() << "]'s friend["
                 << ++cnt <<"]:[" << it->first << "]";
    }
}

void UserManager::updateStatus(const UserId& user_id, e_status status)
{
    UserRefer &user_refer = G_user_manager.getRefer(user_id);
    if ( user_refer.barebone() ) {
        LOG_WARN << "not found, user=[" << user_id.id() << "]";
        return;
    }
    user_refer.setStatus(status);
    LOG_INFO << "updating status of user="<< user_id.id()
             <<" as status=" << user_refer.statusStr();
}

void UserManager::updateConnection(const UserId& user_id,
                                   const muduo::net::TcpConnectionPtr& connection)
{
    UserRefer &user_refer = G_user_manager.getRefer(user_id);
    if ( user_refer.barebone() ) {
        LOG_WARN << "not found, user=[" << user_id.id() << "]";
        return;
    }
    user_refer.setConnectionPtr(connection);
    LOG_INFO << "updating connection of user="<< user_id.id();
}


void UserManager::updateReferBasic(const UserId& user_id, e_status status,
                                   const muduo::net::TcpConnectionPtr& connection)
{
    UserRefer &user_refer = G_user_manager.getRefer(user_id);
    if ( user_refer.barebone() ) {
        LOG_WARN << "not found, user=[" << user_id.id() << "]";
        return;
    }
    user_refer.setStatus(status);
    LOG_INFO << "user="<< user_id.id() << ", status updated:"
             << user_refer.statusStr();
    // check whether need to update conneciton
    if ( ! ::isOnLine(status) ) {
        LOG_DEBUG << "update for off-line, do not update connection";
        return;
    }
    user_refer.setConnectionPtr(connection);
    LOG_INFO << "user=" << user_id.id() << ", connection updated:"
             << user_refer.connection()->peerAddress().toIpPort();
}

bool UserManager::nameOccupied(const std::string& name)
{
    if ( 0 == name.size() ) {
        LOG_ERROR << "invalid argument, name is empty";
        // FIXME: [2013-11-21] true or false
        return true;
    }

    const UserNameRefer *name_refer = m_name_refers.find(name.c_str());
    if ( NULL != name_refer ) {
        LOG_DEBUG << "name occupied, name=" << name;
        return true;
    }
    LOG_DEBUG << "name not occupied, name=" << name;
    return false;
}

bool UserManager::addUserNameRefer(const UserId& user_id, const std::string& name)
{
    if ( 0 == name.size() || user_id.barebone()) {
        LOG_ERROR << "invalid user name information, name is empty or user id not set";
        return false;
    }

    LOG_INFO << "creating and storing user name information: user id:["
             << user_id.id() << "] name=["<< name << "]";

    UserNameRefer name_refer(user_id, name);
    if ( ! m_name_refers.insert(name.c_str(), name_refer) ) {
        LOG_INFO << "failed to add user=" << name << ", id=" << user_id.id();
        return false;
    }
    LOG_INFO << "successfully add user=" << name << ", id="<< user_id.id();
    return true;
}
