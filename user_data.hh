#ifndef INCLUDE_USER_DATA_HPP
#define INCLUDE_USER_DATA_HPP

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/extra/utils.h>

#include <vector>
#include <map>
#include <stdio.h>

#include "singleton.hh"

#define USER_ID_LENGTH_MAX (12)
#define USER_ID_LENGTH_MIN (5)
#define USER_ID_INITIALIZER ((unsigned long)(0))

#define SIGNATURE_LENGTH_MAX 100

#define USER_NAME_LENGTH_MIN (4)
#define USER_NAME_LENGTH_MAX (32)


class UserId
{
public:
    UserId() { memset(m_id, 0, sizeof m_id); }
    UserId(const char *id_str) {
        if ( ! format(id_str) ) LOG_ERROR << "failed to format";
    }
    UserId(unsigned long id_digit) {
        if ( ! format(id_digit) ) LOG_ERROR << "failed to format";
    }

    ~UserId() {}

public:
    bool barebone() const {
        return ( USER_ID_INITIALIZER == digitId() );
    }
    UserId& operator()(const UserId& rhs) {
        if ( ! format(rhs.id()) ) LOG_ERROR << "failed to format";
        return *this;
    }
    UserId& operator()(unsigned long id_digit) {
        if ( ! format(id_digit) ) LOG_ERROR << "failed to format";
        return *this;
    }
    UserId& operator=(unsigned long id_digit) {
        if ( ! format(id_digit) ) LOG_ERROR << "failed to format";
        return *this;
    }
    UserId& operator=(const UserId& rhs) {
        if ( ! format(rhs.id()) )
            LOG_ERROR << "failed to format";
        return *this;
    }
    UserId& operator=(const std::string& std_str) {
        if ( ! format(std_str.c_str()) ) LOG_ERROR << "failed to format";
        return *this;
    }
    UserId& operator=(const char* id_str) {
        if ( ! format(id_str) ) LOG_ERROR << "failed to format";
        return *this;
    }
    bool operator==(const UserId& rhs) const {
        return (0 == strcmp(this->id(), rhs.id()));
    }
    bool operator==(const UserId& rhs) {
        return (0 == strcmp(this->id(), rhs.id()));
    }

    bool operator!=(const UserId& rhs) const {
        return (0 != strcmp(this->id(), rhs.id()));
    }
    bool operator!=(const UserId& rhs) {
        return (0 != strcmp(this->id(), rhs.id()));
    }

    // absolute duplicate, should not format.
    UserId& dup(const char *id_str) {
        if ( ! format(id_str) ) LOG_ERROR << "failed to format";
        return *this;
    }
    const char* id() const {
        return m_id;
    }
    unsigned long digitId() const {
        return atoi(m_id);
    }
private:
    // if argument is string, then absolute copy, no convert; else, convert
    // is operated to keep valid user_id format
    bool format(const char*id_str) {
        if ( ! id_str || '\0' == *id_str ) {
            LOG_ERROR << "invalid argument, id_str is null or empty";
            return false;
        }
        if ( strlen(id_str) > USER_ID_LENGTH_MAX ) {
            LOG_ERROR << "invalid argument, id_str too long";
            return false;
        }
        strncpy(m_id, id_str, sizeof m_id);
        return true;
    }
    bool  format(unsigned long id_digit) {
        return (0 <= snprintf(m_id, sizeof(this->m_id), "%05ld", id_digit) );
    }
private:
    char m_id[USER_ID_LENGTH_MAX];
};

#define G_userid_generator SingletonX<UserIdGenerator>::instance()

class UserIdGenerator
{
public:
    UserIdGenerator() {}
    ~UserIdGenerator() {}

    const UserId& generate() {
        muduo::MutexLockGuard lock(m_mutex);
        ++m_current_id_number;
        m_user_id = m_current_id_number;
        return m_user_id;
    }

private:
    static long m_current_id_number;
    UserId m_user_id;
    mutable muduo::MutexLock m_mutex;
};

class UserDataBrief
{
public:
    UserDataBrief() {
        m_reg_name = "none";
        m_signature = "none";
    }

    UserDataBrief(const UserId &user_id,
                  const std::string& reg_name,
                  const std::string& signature_val,
                  const std::string& category_val="Friends")
        : m_user_id(user_id),
          m_reg_name(reg_name),
          m_signature(signature_val),
          m_category(category_val) {
        LOG_INFO << "user brief constructor, category=" << m_category;
    }
    ~UserDataBrief() {}

    const UserId& userId() const {
        return m_user_id;
    }
    const char* userIdStr() const {
        return m_user_id.id();
    }
    const std::string& registerName() const {
        return m_reg_name;
    }
    const std::string& signature() const {
        return m_signature;
    }
    const std::string& category() const {
        return m_category;
    }

    void setUserId(const UserId& user_id)  {
        m_user_id = user_id;
    }
    void setRegisterName(const std::string& register_name) {
        m_reg_name = register_name;
    }
    void setSignature(const std::string& signature_val) {
        m_signature = signature_val;
    }
    void setCategory(const std::string& category_val) {
        m_category = category_val;
    }

protected:
    UserId m_user_id;
    std::string m_reg_name;
    std::string m_signature;
    std::string m_category;     // only used for buddy-es
};

typedef std::map<std::string, UserDataBrief> UserMap;
typedef std::map<std::string, UserDataBrief>::iterator UserMapIterator;
typedef std::map<std::string, UserDataBrief>::const_iterator UserMapConstIterator;
typedef std::pair<UserMapIterator, bool> UserMapPair;

class GroupDataBrief
{
public:
    GroupDataBrief() {}
    virtual ~GroupDataBrief() {}
private:
    std::string m_name;
    UserMap m_members;
};

typedef std::map<std::string, GroupDataBrief> GroupMap;
typedef std::map<std::string, GroupDataBrief>::iterator GroupMapIterator;

// this is the data loaded from database from the very first time
// principle:
// 1. not modified too frequent
// 2. for access purpose
#define USER_DETAIL_DEFAULT_PASSWORD ""
class UserDataDetail: public UserDataBrief
{
public:
    UserDataDetail() {
        m_barebone = true;
        m_reg_name = "";
        m_signature = "";
        m_password = USER_DETAIL_DEFAULT_PASSWORD;
        m_email = "";
    }

    UserDataDetail(const UserId& id, const std::string& reg_name,
                   const std::string& signature_val,
                   const std::string &password_val=USER_DETAIL_DEFAULT_PASSWORD,
                   const std::string &email_val= "" ) {
        // code start here
        m_user_id = id;
        m_reg_name = reg_name;
        m_signature = signature_val;
        m_password = password_val;
        m_email = email_val;
        m_barebone = false;

        LOG_INFO << "init detail: user_id:[" << m_user_id.id() << "], "
                 << "register name: [" << m_reg_name<< "], "
                 << "password: [" << m_password<< "], "
                 << "signature: [" << m_signature<< "].";
    }

    UserDataDetail& operator=(const UserDataDetail& rhs) {
        m_barebone = false;
        // buddies,
        UserMapConstIterator it = rhs.m_buddies.begin();
        for ( ; it != rhs.m_buddies.end(); ++it ) {
            UserDataBrief tmp_brief;
            tmp_brief.setUserId(it->second.userId());
            tmp_brief.setRegisterName(it->second.registerName());
            tmp_brief.setSignature(it->second.signature());
            tmp_brief.setCategory(it->second.category());
            this->m_buddies.insert(std::make_pair(
                                       std::string(tmp_brief.userId().id()),
                                       tmp_brief));
        }
        // TODO: [2013-11-12] groups:

        this->m_password = rhs.password();
        this->m_signature = rhs.signature();
        this->m_reg_name = rhs.registerName();
        this->m_user_id = rhs.userId();
        this->m_email = rhs.email();

        return *this;
    }
    ~UserDataDetail() {}

public:
    bool isMyFriend (const UserId& user_id) const;

    const std::string& email() const {
        return m_email;
    }

    bool barebone() {
        if (m_barebone) {
            LOG_INFO << "current user refer's detail data is a barebone";
            return true;
        } else {
            LOG_INFO << "current user refer's detail data is *NOT* a barebone";
            return false;
        }
    }

    bool barebone() const {
        if (m_barebone) {
            LOG_INFO << "current user refer's detail data is a barebone";
            return true;
        } else {
            LOG_INFO << "current user refer's detail data is *NOT* a barebone";
            return false;
        }
    }

    const std::string& password() const {
        return m_password;
    }

    void setPassword(const std::string& password_val) {
        m_password = password_val;
    }

    void addFriend(const UserDataBrief& friend_brief) {
        UserMapPair pair = m_buddies.insert(
            std::make_pair(std::string(friend_brief.userId().id()), friend_brief));
        if ( false == pair.second ) {
            LOG_WARN << friend_brief.userId().id()
                     << " already "<< userId().id() <<"'s friend";
        } else {
            LOG_INFO << "successfully "<< userId().id()
                     << " adds friend=" << friend_brief.userId().id();
        }
    }

    void deleteFriend(const UserId& friend_user_id) {
        m_buddies.erase(std::string(friend_user_id.id()));
        LOG_INFO << "successfully erase friend=" << friend_user_id.id();
    }

    const UserMap& buddies() const {
        return m_buddies;
    }

private:
    bool m_barebone;
    UserMap m_buddies;
    GroupMap m_groups;
    std::string m_password;     // FIXME: [2013-11-08]
    std::string m_email;
    // to add
};

enum e_status {
    E_STATUS_LOWER_BOUND,       // 0
    E_STATUS_ONLINE,
    E_STATUS_OFFLINE,
    E_STATUS_HIDING,
    E_STATUS_UPPER_BOUND
};

typedef struct status_info status_info;
struct status_info {
    e_status status;
    const char *str;
};

extern bool isStatusValid(e_status status);
extern const char* getStatusStr(e_status status);
extern bool getStatus(e_status& status, const char* status_str);
extern bool isOnLine(e_status status);

class UserNameRefer
{
public:
    UserNameRefer() :m_user_id(USER_ID_INITIALIZER) {}
    UserNameRefer(const UserId& user_id, const std::string& name_val):
        m_user_id(user_id), m_name(name_val) {}
    ~UserNameRefer() {}

public:
    UserNameRefer& operator=(const UserNameRefer& rhs) {
        this->m_name = rhs.name();
        this->m_user_id = rhs.userId();
        return *this;
    }
    const std::string& name() const { return m_name; }
    const UserId& userId() const { return m_user_id;}

private:
    UserId m_user_id;
    std::string m_name;
};


class UserRefer
{
public:
    UserRefer() : m_status(E_STATUS_OFFLINE), m_barebone(true) {}
    ~UserRefer() {}

    UserRefer& operator= (const UserRefer& rhs) {
        LOG_INFO << "calling assignment constructor";
        this->m_status = rhs.status();
        this->m_detail = rhs.detail();
        this->m_conptr = rhs.connection();
        LOG_INFO << "assigning constructor: to get id:[" << m_detail.userId().id() <<"]";
        this->m_barebone = false;
        return *this;
    }
    const e_status& status() const {
        return m_status;
    }

    const char* statusStr() const {
        return getStatusStr(m_status);
    }

    UserDataDetail& detail() {
        return m_detail;
    }

    const UserDataDetail& detail() const {
        return m_detail;
    }

    const muduo::net::TcpConnectionPtr& connection() const {
        return m_conptr;
    }

    void setStatus(e_status s) {
        m_status = s;
    }

    void setConnectionPtr(const muduo::net::TcpConnectionPtr& conptr_val) {
        m_conptr = conptr_val;
    }

    void setDetail(UserDataDetail& detail_val) {
        m_detail = detail_val;
        m_barebone = false;
    }

    bool barebone() const {
        return m_barebone;
    }

    bool online() {
        return (m_status == E_STATUS_ONLINE);
    }

    bool logined() {
        return (m_status == E_STATUS_ONLINE);
    }

    bool offline() {
        return (m_status == E_STATUS_OFFLINE);
    }

private:
    // following for self-related stuff
    enum e_status m_status;
    UserDataDetail m_detail;
    muduo::net::TcpConnectionPtr m_conptr;
    // for return value of getRefer
    bool m_barebone;
    std::string m_identification; // 32, effect when user is logined
};

struct buddy_t {
    std::string user_id;
    std::string name;
    std::string signature;
    std::string category;
    std::string status;
} ;

struct group_t {
    std::string group_id;
    std::string name;
    std::string signature;
    std::string admin_user_id;
};

#endif /* INCLUDE_USER_DATA_HPP */
