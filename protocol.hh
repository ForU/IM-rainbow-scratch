#ifndef INCLUDE_PROTOCOL_HPP
#define INCLUDE_PROTOCOL_HPP

#include <boost/scoped_ptr.hpp>
#include <muduo/extra/Json.h>
#include <muduo/base/Condition.h>

#include "singleton.hh"
#include "user_data.hh"

#define MSG_BUF_SIZE 12000
#define MSG_CHAT_BUF_SIZE 1000

enum e_error_code
{
    E_ERROR_CODE_NORMAL,   // 0
    E_ERROR_CODE_INVALID_PROTOCOL,
    E_ERROR_CODE_INVALID_MESSAGE,
    E_ERROR_CODE_SERVER_INTERNAL_ERROR,
    // to add
    E_ERROR_CODE_UPPER_BOUND
};

typedef struct {
    e_error_code code;
    const char *info;
    const char *extra;
} ErrorCodeInfo;

enum e_login_type
{
    E_LOGIN_TYPE_USER_ID,       // user id
    E_LOGIN_TYPE_REGISTER_NAME, // register name
    E_LOGIN_TYPE_EMAIL,         // email address
};

enum e_message_type
{
    E_MESSAGE_TYPE_NONE,        // 0
    E_MESSAGE_TYPE_REGISTER_REQUEST,
    E_MESSAGE_TYPE_REGISTER_RESPONSE,
    E_MESSAGE_TYPE_LOGIN_REQUEST,
    E_MESSAGE_TYPE_LOGIN_RESPONSE,
    E_MESSAGE_TYPE_CHAT,
    E_MESSAGE_TYPE_CHAT_RESPONSE,
    E_MESSAGE_TYPE_GROUP_CHAT,
    E_MESSAGE_TYPE_NOTIFICATION_LOGIN_STATUS,
    E_MESSAGE_TYPE_FRIENDS_MANAGEMENT,
    E_MESSAGE_TYPE_FRIENDS_MANAGEMENT_RESPONSE,

    // TO add here
    E_MESSAGE_TYPE_FATAL,
    E_MESSAGE_TYPE_UPPER_BOUND  // upper bound
};

typedef boost::function<void (const muduo::net::TcpConnectionPtr&)> MessageHandleCallBack;

class Message
{
public:
    Message();
    ~Message() {}
    virtual bool process(const muduo::net::TcpConnectionPtr& self_conn) {return false;}
    virtual bool composeErrorResponse();
    virtual bool compose(std::string& result_message) = 0;

    const std::string& requestStr() const;
    const std::string& responseStr() const;
    const std::string& bodyStr() const { return m_body;}
    e_message_type getType() const { return m_type; }

    void setResponseStr(const char* response_str);
    void setRequestStr(const char* request_str);

    void setError(e_error_code code, const char* info = ERROR_INFO_DEFAULT, const char* extra = EXTRA_INFO_DEFAULT);
    bool parseError();
    e_error_code getErrorCode() const { return m_error_code; }
    bool isErrorMessage() const { return (E_ERROR_CODE_NORMAL != m_error_code); }
    int getErrorCodeInt() const { return (int)m_error_code; }
    const std::string& getErrorInfo() const { return m_error_info; }
    const std::string& getExtraInfo() const { return m_extra_info; }

protected:
    e_message_type m_type;
    bool m_msg_data_valid;
    bool m_msg_protocol_valid;

    e_error_code m_error_code;
    std::string m_error_info;
    std::string m_extra_info;

    std::string m_body;         // for response type(parse)
    std::string m_request;      // for request type(compose)
    std::string m_response;     // for request type(compose)

    static const char *ERROR_INFO_DEFAULT;
    static const char *EXTRA_INFO_DEFAULT;
};

class RegisterRequest : public Message
{
public:
    RegisterRequest(const std::string &message_body);
    RegisterRequest(const char *name, const char* email="", const char* password="", const char* signature_val="");
    ~RegisterRequest() {}
public:
    bool process(const muduo::net::TcpConnectionPtr& self_conn);
    bool compose(std::string& result_message);
    bool parse();
    bool composeResponse();

    const std::string& getName() const { return m_name; }
    const std::string& getUserIdStr() const { return m_user_id_str; }
    const std::string& getSignature() const { return m_signature; }
    const std::string& getPassword() const { return m_password; }
    const std::string& getEmail() const { return m_email; }

protected:
    bool isNameValid() const;
    bool checkValidation();
    void setUserId(const char* user_id);

private:
    bool composeSuccessResponse();
    bool registeNewUser();

protected:
    std::string m_name;
    std::string m_email;
    std::string m_password;
    std::string m_signature;
    std::string m_user_id_str;
};

class RegisterResponse : public RegisterRequest
{
public:
    RegisterResponse(const std::string &message_body);
    RegisterResponse(const char*user_id, const char *name_val, const char* email_val, const char* password_val, const char* signature_val);
    ~RegisterResponse() {}

public:
    bool parse();
    bool compose(std::string& result_message);
private:
    bool parseError();
};

class LoginRequest : public Message
{
public:
    LoginRequest(const std::string &message_body);
    LoginRequest(const char *user_id, const char *password, const char *mask=NULL);
    ~LoginRequest() {}

public:
    bool process(const muduo::net::TcpConnectionPtr& self_conn);
    bool compose(std::string& result_message);
    bool parse();
    bool composeResponse();
    const std::string& userIdStr() const;

private:
    bool checkValidation();
    bool composeSuccessResponse();

private:
    e_login_type m_login_type;
    std::string m_login_value;  // user id, registername, ...
    UserId m_user_id;
    std::string m_user_id_str;
    std::string m_password;
    std::string m_mask;         // ignore now
};

class LoginResponse : public Message
{
public:
    LoginResponse();
    LoginResponse(const std::string &message_body);
    LoginResponse(const UserId& user_id);
    virtual ~LoginResponse() {}

    const std::string& getUserIdStr() const { return m_self_user_id_str; }
    const std::string& getName() const { return m_self_name; }
    const std::string& getSignature() const { return m_self_signature; }
    const std::string& getStatus() const { return m_self_status; }
    const std::string& getSessionId() const { return m_self_session_id; }
    const std::vector<buddy_t>& getBuddies() const { return m_buddies; }
    const std::vector<group_t>& getGroups() const { return m_groups; }

public:
    bool parse();
    bool compose(std::string& result_message);

private:
    UserId m_self_user_id;
    std::string m_self_user_id_str;
    std::string m_self_status;
    std::string m_self_name;
    std::string m_self_signature;
    std::string m_self_session_id;

    std::vector<buddy_t> m_buddies;
    std::vector<group_t> m_groups;
};

class ChatMessage : public Message
{
public:
    ChatMessage(const std::string &message_body, bool is_from_client=true);
    ChatMessage(const std::string& from_val,
                const std::string& to_val,
                const std::string& content_val,
                const std::string& time = "now",
                bool is_from_client = false,
                const std::string& session_id="");
    ~ChatMessage() {}

public:
    bool process(const muduo::net::TcpConnectionPtr& self_conn);

    bool composeErrorResponse();
    bool compose(std::string& result_message);

    bool parse();
    bool parseError();

    const UserId& getFromId() const { return m_from; }
    const UserId& getToId() const { return m_to; }
    const std::string& getFromStr() const { return m_from_str; }
    const std::string& getToStr() const { return m_to_str; }
    const std::string& getContent() const { return m_content; }
    const std::string& getTimeStr() const { return m_time_str; }

private:
    bool checkValidation();

private:
    UserId m_from;
    UserId m_to;
    std::string m_from_str;
    std::string m_to_str;
    std::string m_content;
    std::string m_time_str;
    bool m_is_from_client;
    std::string m_session_id;
};

class LoginStatusNotification : public Message
{
public:
    LoginStatusNotification(const std::string& message_body);
    LoginStatusNotification(const UserId& from, e_status status);
    ~LoginStatusNotification() {}
    bool compose(std::string& result_message);
    bool parse();
    const std::string& getFromStr() { return m_from_str; }
    bool isOnLineNotification() const { return ( m_status == E_STATUS_ONLINE); }
    const std::string& getStatusStr() { return m_status_str; }
    bool process(const muduo::net::TcpConnectionPtr& self_conn);

private:
    std::string m_from_str;
    std::string m_status_str;
    e_status m_status;
};

enum e_friends_manage_action
{
    FRIENDS_MANAGE_ACTION_ADD,
    FRIENDS_MANAGE_ACTION_SEARCH,
    FRIENDS_MANAGE_ACTION_DELETE,
    FRIENDS_MANAGE_ACTION_UPDATE,
    FRIENDS_MANAGE_ACTION_UPPER_BOUND
};

extern bool isFriendsManageActionValid(e_friends_manage_action action);

class FriendsManagementMessage : public Message
{
public:
    FriendsManagementMessage(const std::string& message_body);
    FriendsManagementMessage(e_friends_manage_action action,
                             const std::string& requester_id_str,
                             const std::string& target_id_str,
                             const std::string& category,
                             const std::string& session_id);
    ~FriendsManagementMessage() {}

    const std::string& getRequesterIdStr() { return m_requester_id_str; }
    const std::string& getTargetIdStr() { return m_target_id_str; }

    bool compose(std::string& result_message);
    bool parse();
    bool checkValidation();
    bool process(const muduo::net::TcpConnectionPtr& self_conn);

private:
    bool composeErrorResponse();
    bool processManageActionAdd(const muduo::net::TcpConnectionPtr& self_conn);
    bool processManageActionSearch(const muduo::net::TcpConnectionPtr& self_conn);

private:
    e_friends_manage_action m_action;
    std::string m_requester_id_str;
    std::string m_target_id_str;
    std::string m_category;     // only for add/update
    std::string m_session_id;
};

class FriendsManagementResponse : public Message
{
public:
    FriendsManagementResponse(const std::string& message_body);
    FriendsManagementResponse( e_friends_manage_action request_action,
                               const std::string& target_id_str,
                               const std::string& target_name,
                               const std::string& target_signature,
                               const std::string& target_status,
                               const std::string& target_category,
                               bool is_already_friend);
    ~FriendsManagementResponse() {}


    const std::string& getTargetIdStr() const { return m_target_id_str; }
    const std::string& getTargetName() const { return m_target_name; }
    const std::string& getTargetSignature() const { return m_target_signature; }
    const std::string& getTargetStatus() const { return m_target_status; }
    const std::string& getTargetCategory() const { return m_target_category; }
    bool isAlreadyFriend() const { return m_is_already_friend; }
    bool isTargetOnline() const;

    e_friends_manage_action getRequestAction() const {
        return m_request_action;
    }

    bool compose(std::string& result_message);
    bool parse();
    bool parseError();

private:
    e_friends_manage_action m_request_action;
    std::string m_target_id_str;
    std::string m_target_name;
    std::string m_target_signature;
    std::string m_target_status;
    std::string m_target_category;
    bool m_is_already_friend;
};

#endif /* INCLUDE_PROTOCOL_HPP */
