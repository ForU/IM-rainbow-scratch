#include <string.h>
#include <muduo/extra/Json.h>

#include "protocol.hh"
#include "packet_manager.hh"
#include "user_manager.hh"
#include "session_manager.hh"
#include "time_stamp.hh"

using namespace muduo;
using namespace muduo::net;

/* for internal use */
#define JSON_GET_RAW_STRING_EXT(json, key, err_occurred, error_info)    \
    DStr raw_str__ = (json).GetValue(key, NULL);                        \
    if ( NULL == raw_str__.Str() ) {                                    \
        error_info = "no key named [" + std::string(key) + "]";         \
        err_occurred = true;                                            \
    }                                                                   \

#define JSON_GET_INT_VALUE_EXT(json, des, key)                          \
    do {                                                                \
        std::string error_info;                                         \
        bool err_occurred = false;                                      \
        JSON_GET_RAW_STRING_EXT(json, key, err_occurred, error_info);   \
        if ( err_occurred ) {                                           \
            setError(E_ERROR_CODE_INVALID_PROTOCOL, error_info.c_str(), m_body.c_str()); \
            LOG_ERROR << error_info;                                    \
            return false;                                               \
        }                                                               \
        des = atoi(raw_str__.Str());                                    \
    } while ( 0 )

#define JSON_GET_BOOL_VALUE_EXT(json, des, key)                         \
    do {                                                                \
        std::string error_info;                                         \
        bool err_occurred = false;                                      \
        JSON_GET_RAW_STRING_EXT(json, key, err_occurred, error_info);   \
        if ( err_occurred ) {                                           \
            setError(E_ERROR_CODE_INVALID_PROTOCOL, error_info.c_str(), m_body.c_str()); \
            LOG_ERROR << error_info;                                    \
            return false;                                               \
        }                                                               \
        if( "true" == raw_str__ ) {                                     \
            (des) = true;                                               \
        }                                                               \
        else if( "false" == raw_str__ ) {                               \
            (des) = false;                                              \
        }                                                               \
    } while ( 0 )

#define JSON_GET_STR_VALUE_EXT(json, des, key)                          \
        do {                                                            \
            std::string error_info;                                     \
            bool err_occurred = false;                                  \
            JSON_GET_RAW_STRING_EXT(json, key, err_occurred, error_info); \
            if ( err_occurred ) {                                       \
                setError(E_ERROR_CODE_INVALID_PROTOCOL, error_info.c_str(), m_body.c_str()); \
                LOG_ERROR << error_info;                                \
                return false;                                           \
            }                                                           \
            (des) = raw_str__.Str();                                    \
        } while ( 0 )

#define JSON_GET_STR_VALUE_IGNORE_ERR_EXT(json, des, key)               \
        do {                                                            \
            std::string error_info;                                     \
            bool err_occurred = false;                                  \
            JSON_GET_RAW_STRING_EXT(json, key, err_occurred, error_info); \
            if ( err_occurred ) {                                       \
                setError(E_ERROR_CODE_INVALID_PROTOCOL, error_info.c_str(), m_body.c_str()); \
                LOG_WARN << error_info;                                 \
                break;                                                  \
            }                                                           \
            (des) = raw_str__.Str();                                    \
        } while ( 0 )

////////////////////////////////////////////////////////////////

// this file specified
#define JSON_GET_ERROR_CODE(json, error_code)                           \
        do {                                                            \
            int raw_error_code__;                                       \
            JSON_GET_INT_VALUE_EXT(json, raw_error_code__, "error_code"); \
            error_code = (e_error_code)raw_error_code__;                \
        } while(0)
////////////////////////////////////////////////////////////////

static ErrorCodeInfo error_code_info[] = {
    /* 0 */ { E_ERROR_CODE_NORMAL, "valid message", ""},
    /* 1 */ { E_ERROR_CODE_INVALID_PROTOCOL, "invalid protocol", ""},
    /* 2 */ { E_ERROR_CODE_INVALID_MESSAGE, "invalid message data", ""},
    /* 3 */ { E_ERROR_CODE_SERVER_INTERNAL_ERROR, "server internal error", ""},
    // to add
    { E_ERROR_CODE_UPPER_BOUND, "", ""}
};

static bool errorCodeValid(e_error_code code)
{
    return (   code < E_ERROR_CODE_UPPER_BOUND
               && code >= E_ERROR_CODE_NORMAL);
}

static const char* getErrorInfo(e_error_code code)
{
    LOG_TRACE << "getting error info";
    if ( ! errorCodeValid(code) ) {
        return "";
    }
    return error_code_info[code].info;
}

static const char* getExtraInfo(e_error_code code)
{
    LOG_TRACE << "getting extra info";
    if ( ! errorCodeValid(code) ) {
        return "";
    }
    return error_code_info[code].extra;
}

////////////////////////////////////////////////////////////////
const char *Message::ERROR_INFO_DEFAULT = "";
const char *Message::EXTRA_INFO_DEFAULT = "";

Message::Message()
{
    m_msg_data_valid = true;
    m_msg_protocol_valid = true;
    m_type = E_MESSAGE_TYPE_NONE;

    m_error_code = E_ERROR_CODE_NORMAL;
    m_error_info = "";
    m_extra_info = "";

    m_body = "";
    m_request = "";
    m_response = "";
}

const std::string& Message::requestStr() const
{
    return m_request;
}

const std::string& Message::responseStr() const{
    return m_response;
}

void Message::setResponseStr(const char* response_str)
{
    m_response = response_str;
}

void Message::setRequestStr(const char* request_str)
{
    m_request = request_str;
}

void Message::setError(e_error_code code, const char* info, const char* extra)
{
    LOG_TRACE << "coming info:"
              << ", error_code=" << code
              << ", error_info=" << info
              << ", extra_info=" << extra;

    if ( ! errorCodeValid(code) ) {
        LOG_ERROR << "invalid argument, code out of range";
        return;
    }
    m_error_code = code;

    if ( E_ERROR_CODE_INVALID_MESSAGE == m_error_code ) {
        m_msg_data_valid = false;
    } else if ( E_ERROR_CODE_INVALID_PROTOCOL == m_error_code ) {
        m_msg_protocol_valid = false;
    } else {
        ;                       // do nothing
    }

    // info
    if ( 0 == strcmp(info, ERROR_INFO_DEFAULT) ) {
        m_error_info = ::getErrorInfo(m_error_code);
    } else {
        m_error_info = info;
    }
    // extra
    if ( 0 == strcmp(extra, EXTRA_INFO_DEFAULT) ) {
        m_extra_info = ::getExtraInfo(m_error_code);
    } else {
        m_extra_info = extra;
    }

    LOG_TRACE << "after setting error:"
              << ", m_error_code=" << m_error_code
              << ", m_error_info=" << m_error_info
              << ", m_extra_info=" << m_extra_info;
}

bool Message::parseError()
{
    if ( 0 == m_body.size() ) {
        LOG_WARN << "m_body not initialized";
        return false;
    }
    Json json(m_body.c_str());
    JSON_GET_ERROR_CODE(json, m_error_code);
    JSON_GET_STR_VALUE_EXT(json, m_error_info, "error_info");
    JSON_GET_STR_VALUE_EXT(json, m_extra_info, "extra_info");
    return true;
}

bool Message::composeErrorResponse()
{
    LOG_TRACE << "composing error response"
              << ", m_error_code=" << m_error_code
              << ", m_error_info=" << m_error_info
              << ", m_extra_info=" << m_extra_info;

    if ( ! errorCodeValid(m_error_code) ) {
        LOG_ERROR << "invalid error code to be send, error_code=" << m_error_code;
        setError(E_ERROR_CODE_SERVER_INTERNAL_ERROR, ERROR_INFO_DEFAULT, "");
        // do not return here.
    }

    Json json;
    std::vector<DStr> body_vec;
    JSON_VEC_PUSH_INT(json, body_vec, "error_code", (int)m_error_code);
    JSON_VEC_PUSH_STR(json, body_vec, "error_info", m_error_info.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "extra_info", m_extra_info.c_str());

    m_response = json.SetObject(body_vec, NULL).Str();

    return true;
}


////////////////////////////////////////////////////////////////
RegisterRequest::RegisterRequest(const std::string &message_body) {
    m_type = E_MESSAGE_TYPE_REGISTER_REQUEST;
    m_body = message_body;
    m_msg_data_valid = true;
    m_msg_protocol_valid = true;

    if ( message_body.size() == 0) {
        LOG_DEBUG << "message body size 0";
        return;
    }
}

RegisterRequest::RegisterRequest(const char *name_val,
                                 const char* email_val,
                                 const char* password_val,
                                 const char* signature_val)
    :m_name(name_val), m_email(email_val),
     m_password(password_val), m_signature(signature_val)
{
    m_type = E_MESSAGE_TYPE_REGISTER_REQUEST;
}

bool RegisterRequest::parse()
{
    LOG_DEBUG << "parsing register request message body:\"" << m_body<<"\"";
    Json json(m_body.c_str());
    // name

    JSON_GET_STR_VALUE_EXT(json, m_name, "name");
    JSON_GET_STR_VALUE_EXT(json, m_email, "email");
    JSON_GET_STR_VALUE_EXT(json, m_password, "password");
    JSON_GET_STR_VALUE_EXT(json, m_signature, "signature");

    LOG_DEBUG << "after register request parse, "
              << "name=[" << m_name << "], "
              << "email=[" << m_email << "], "
              << "password=[" << m_password << "], "
              << "signature=[" << m_signature << "]";

    // message is success parsed ok
    return m_msg_protocol_valid;
}

// check if register name already occupied
// if every ok:
//    new name node
//    new refer node with new user Id
bool RegisterRequest::registeNewUser()
{
    UserId user_id;
    bool rc = G_user_manager.newUser(user_id, m_name, m_signature, m_email);
    if ( ! rc  ) {
        LOG_ERROR << "failed to new user=" << user_id.id() << ", name=" << m_name;
        DStr extra_info;
        extra_info.AssignFmt("failed to register such user=%s", m_name.c_str());
        setError(E_ERROR_CODE_SERVER_INTERNAL_ERROR, ERROR_INFO_DEFAULT, extra_info.Str());
        return false;
    }
    setUserId(user_id.id());
    LOG_TRACE << "successfully new user=" << m_user_id_str << ", name=" << m_name;
    return true;
}

void RegisterRequest::setUserId(const char* user_id)
{
    if (! user_id || '\0' == *user_id) {
        LOG_ERROR << "invalid argument, user_id is NULL";
        return;
    }
    m_user_id_str = user_id;
}

bool RegisterRequest::process(const TcpConnectionPtr& self_conn)
{
    do {
        if ( ! parse() ) {
            m_msg_protocol_valid = false;
            break;
        }
        if (! checkValidation() || ! registeNewUser()) {
            LOG_ERROR << "failed to checkValidation";
            break;
        }
    } while ( 0 );
    // compose response
    if ( isErrorMessage() ) {
        composeErrorResponse();
    } else {
        composeSuccessResponse();
    }
    Packet resp_packet(m_response, E_MESSAGE_TYPE_REGISTER_RESPONSE);
    resp_packet.sendTo(self_conn);
    // shutdown immediately
    self_conn->shutdown();
    return true;
}

bool RegisterRequest::compose(std::string& result_message)
{
    Json json;
    std::vector<DStr> body_vec;
    JSON_VEC_PUSH_STR(json, body_vec, "name", m_name.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "email", m_email.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "password", m_password.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "signature", m_signature.c_str());

    DStr request = json.SetObject(body_vec, NULL);
    // set both result and member data
    result_message = request.Str();
    setRequestStr(request.Str());
    LOG_TRACE << "successfully compose register request=[" << requestStr() << "]";
    return true;
}

bool RegisterRequest::isNameValid() const
{
    int size = m_name.size();
    for ( int i = 0; i < size; ++i ) {
        if ( G_DEFAULT_INDEX == getUserNameReferIndex(m_name[i]) ) {
            return false;
        }
    }
    return true;
}

bool RegisterRequest::checkValidation()
{
    LOG_TRACE << "checking register request message validation";
    DStr error_info, extra_info;
    // protocol
    if ( ! m_msg_protocol_valid ) {
        LOG_ERROR << "not valid login request protocol, body:" << m_body;
        setError(E_ERROR_CODE_INVALID_PROTOCOL, ERROR_INFO_DEFAULT, m_body.c_str());
        return false;
    }

    // to check name validation
    LOG_INFO << "to check whether name is occupied or not";
    if ( ! isNameValid() ) {
        error_info.Empty();
        error_info.AssignFmt("invalid name, name=%s", m_name.c_str());
        extra_info.Empty();
        extra_info.AssignFmt("only following character supported:[%s]", UserNameSupportedChars);
        setError(E_ERROR_CODE_INVALID_MESSAGE, error_info.Str(), extra_info.Str());
        return false;
    }

    // to check whether name is occupied or not
    LOG_INFO << "to check whether name is occupied or not";
    bool name_occupied = G_user_manager.nameOccupied(m_name);
    if ( name_occupied ) {
        LOG_WARN << "already occupied register name=" << m_name;
        error_info.Empty();
        error_info.AssignFmt("register name occupied, name=%s", m_name.c_str());
        LOG_INFO << "error_info=" << error_info.Str();
        setError(E_ERROR_CODE_INVALID_MESSAGE, error_info.Str(), "");
        return false;
    }
    LOG_TRACE << "name=" << m_name << " *NOT OCCUPIED*";

    LOG_INFO << "register request message is *VALID*";
    return m_msg_data_valid;
}

bool RegisterRequest::composeSuccessResponse()
{
    RegisterResponse register_response(m_user_id_str.c_str(),
                                       m_name.c_str(), m_email.c_str(),
                                       m_password.c_str(), m_signature.c_str());
    register_response.compose(m_response);
    LOG_TRACE << "register response composed=["<< responseStr() << "]";
    return false;
}


////////////////////////////////////////////////////////////////
RegisterResponse::RegisterResponse(const std::string &message_body):
    RegisterRequest(message_body)
{
    m_type = E_MESSAGE_TYPE_REGISTER_RESPONSE;
    m_body = message_body;
    m_msg_data_valid = true;
    m_msg_protocol_valid = true;

    if ( message_body.size() == 0) {
        LOG_DEBUG << "message body size 0";
        return;
    }
}

bool RegisterResponse::parseError()
{
        if ( 0 == m_body.size() ) {
        LOG_WARN << "m_body not initialized";
        return false;
    }
    Json json(m_body.c_str());
    // basic
    JSON_GET_ERROR_CODE(json, m_error_code);
    JSON_GET_STR_VALUE_EXT(json, m_error_info, "error_info");
    JSON_GET_STR_VALUE_EXT(json, m_extra_info, "extra_info");
    return true;
}

bool RegisterResponse::parse()
{
    LOG_TRACE << "parsing friends management add response message";
    Json json(m_body.c_str());

    JSON_GET_ERROR_CODE(json, m_error_code);
    if ( isErrorMessage() ) {
        LOG_TRACE << "message is error message";
        return parseError();
    }

    // parse normal response
    JSON_GET_STR_VALUE_EXT(json, m_user_id_str, "user_id");
    JSON_GET_STR_VALUE_EXT(json, m_name, "name");
    JSON_GET_STR_VALUE_EXT(json, m_password, "password");
    JSON_GET_STR_VALUE_EXT(json, m_signature, "signature");

    LOG_TRACE << "user_id=" << m_user_id_str << ", name=" << m_name;

    return true;
}

RegisterResponse::RegisterResponse(const char*user_id,
                                   const char *name_val,
                                   const char* email_val,
                                   const char* password_val,
                                   const char* signature_val)
    : RegisterRequest(name_val, email_val, password_val, signature_val)
{
    m_type = E_MESSAGE_TYPE_REGISTER_RESPONSE;
    m_user_id_str = user_id;
}

bool RegisterResponse::compose(std::string& result_message)
{
    Json json;
    std::vector<DStr> body_vec;
    JSON_VEC_PUSH_INT(json, body_vec, "error_code", m_error_code);
    JSON_VEC_PUSH_STR(json, body_vec, "user_id", m_user_id_str.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "name", m_name.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "email", m_email.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "password", m_password.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "signature", m_signature.c_str());

    DStr response = json.SetObject(body_vec, NULL);
    // set both result and member data
    result_message = response.Str();
    setResponseStr(response.Str());
    LOG_TRACE << "successfully compose register response=[" << responseStr() << "]";
    return true;
}

////////////////////////////////////////////////////////////////
LoginRequest::LoginRequest(const std::string &message_body) {
    m_type = E_MESSAGE_TYPE_LOGIN_REQUEST;
    m_body = message_body;
    m_msg_data_valid = true;
    m_msg_protocol_valid = true;

    if ( message_body.size() == 0) {
        LOG_DEBUG << "message body size 0";
        return;
    }
}

LoginRequest::LoginRequest(const char *user_id, const char *password, const char *mask) :
    m_user_id(user_id), m_user_id_str(user_id),
    m_password(password) {
    m_type = E_MESSAGE_TYPE_LOGIN_REQUEST;
    m_msg_data_valid = true;
    m_msg_protocol_valid = true;
    // TODO: [2013-11-19] to compose
    }


bool LoginRequest::process(const TcpConnectionPtr& self_conn)
{
    do {
        if ( ! parse() ) {
            m_msg_protocol_valid = false;
            break;
        }
        if (! checkValidation() ) {
            LOG_ERROR << "failed to checkValidation";
            break;
        }
    } while ( 0 );
    if ( isErrorMessage() ) {
        composeErrorResponse();
    } else {
        G_user_manager.updateReferBasic(m_user_id, E_STATUS_ONLINE, self_conn);
        composeSuccessResponse();
    }
    // packet and sent
    Packet resp_packet(m_response, E_MESSAGE_TYPE_LOGIN_RESPONSE);
    resp_packet.sendTo(self_conn);
    if ( ! m_msg_protocol_valid || ! m_msg_data_valid ) {
        self_conn->shutdown();
        return false;
    }

    // notify all on-line buddies that "I'am on line now"
    // find buddies first
    const UserMap& buddies = G_user_manager.getUser(m_user_id).buddies();
    if ( 0 == buddies.size() ) {
        LOG_TRACE << "no buddy of user=" << m_user_id.id()
                  << " is online, do nothing";
        return true;
    }
    LoginStatusNotification login_notf(m_user_id, E_STATUS_ONLINE);
    std::string notf_str;
    login_notf.compose(notf_str);
    Packet packet(notf_str, login_notf.getType());
    // loop the buddy
    UserMapConstIterator it = buddies.begin();
    for ( ; it != buddies.end(); ++it ) {
        UserId user_id = it->first.c_str();
        UserRefer& buddy = G_user_manager.getRefer(user_id);
        if ( buddy.barebone() ) {
            LOG_WARN << "not exist user=" << user_id.id()
                     << ", this should never happen here";
            continue;
        }
        if ( buddy.online() ) {
            LOG_TRACE << "buddy=[" << user_id.id() << "] is on-line, to send notification=["
                      << notf_str << "]";
            packet.sendTo(buddy.connection());
        } else {
            LOG_TRACE << "buddy=[" << user_id.id() << "] is off-line";
        }
    }

    // register this user into session manager
    LOG_TRACE << "register current login user to session manager";
    int sockfd;
    bool rc;
    sockfd = self_conn->getSockfd();
    rc = G_session_manager.updateConnectorUserId_Guard(sockfd, m_user_id);
    if ( ! rc ) {
        LOG_ERROR << "failed to update session manager connector's user id, "
                  << "sockfd=" << sockfd;
        return false;
    }
    return true;
}

bool LoginRequest::compose(std::string& result_message)
{
    Json json;
    std::vector<DStr> body_vec;
    // TODO: [2013-11-28] to support login name type
    JSON_VEC_PUSH_STR(json, body_vec, "user_id", m_user_id_str.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "password", m_password.c_str());

    DStr request = json.SetObject(body_vec, NULL);
    // set both result and member data
    result_message = request.Str();
    setRequestStr(request.Str());
    LOG_TRACE << "successfully compose login request=[" << requestStr() << "]";
    return true;
}

const std::string& LoginRequest::userIdStr() const
{
    return m_user_id_str;
}

bool LoginRequest::checkValidation()
{
    LOG_TRACE << "checking login request message validation";
    DStr error_info;
    // protocol
    UserRefer& user_refer= G_user_manager.getRefer(m_user_id);
    // user not exists
    if ( user_refer.barebone() ) {
        LOG_WARN << "not exist user=" << m_user_id.id();
        error_info.Empty(); error_info.AssignFmt("user(id=%s) not exist", m_user_id.id());
        setError(E_ERROR_CODE_INVALID_MESSAGE, error_info.Str(), "");
        return false;
    }
    // NOTICE: check password first before login check.
    // check user_id and password
    UserDataDetail &user_detail = user_refer.detail();
    if ( m_user_id != user_detail.userId() ||
        user_detail.password() != m_password ) {
        LOG_WARN << "not valid login message, body=" << m_body << "m_user_id=" << m_user_id.id() << ", detail user_id=" << user_detail.userId().id();
        error_info.Empty(); error_info.Assign("invalid login name or password");
        setError(E_ERROR_CODE_INVALID_MESSAGE, error_info.Str(), "");
        return false;
    }
    // already login-ed
    if ( user_refer.logined() ) {
        LOG_WARN << "already login-ed user=" << m_user_id.id();
        error_info.Empty(); error_info.AssignFmt("user=%s already login-ed", m_user_id.id());
        setError(E_ERROR_CODE_INVALID_MESSAGE, error_info.Str(), "client not robust");
        return false;
    }

    LOG_INFO << "login message is *VALID*";
    return m_msg_data_valid;
}

bool LoginRequest::composeSuccessResponse()
{
    LoginResponse login_response(m_user_id);
    login_response.compose(m_response);
    return true;
}


bool LoginRequest::parse() {
    LOG_DEBUG << "parsing login request message body:\"" << m_body<<"\"";
    Json json(m_body.c_str());
    std::string user_id_str;
    JSON_GET_STR_VALUE_EXT(json, user_id_str, "user_id");

    // check name's validation
    int size = user_id_str.size();
    for ( int i = 0; i < size; ++i ) {
        if ( ! isUserIdCharSupported(user_id_str[i])) {
            DStr error_info;
            error_info.AssignFmt("unsupported user id char:[%c] in id:[%s]",
                                 user_id_str[i], user_id_str.c_str());
            setError(E_ERROR_CODE_INVALID_MESSAGE, error_info.Str(), "");
            return false;
        }
    }
    // name is ok
    m_user_id.dup(user_id_str.c_str());
    JSON_GET_STR_VALUE_EXT(json, m_password, "password");

    LOG_DEBUG << "login type:[" << m_login_type << "], "
              << "login value:[" << m_login_value << "], "
              << "user id:[" << m_user_id.id() << "], "
              << "password:[" << m_password << "], "
              << "mask:[" << m_mask << "]";
    // message is success parsed ok
    m_msg_protocol_valid = true;
    return true;
}

LoginResponse::LoginResponse(const std::string &message_body)
{
    m_body = message_body;
    // as for response, parse immediately after constructing
    if ( ! parse() ) {
        LOG_ERROR << "failed to parse coming message body";
        return;
    }
}

LoginResponse::LoginResponse(const UserId& user_id):
    m_self_user_id(user_id) {
}

bool LoginResponse::parse()
{
    LOG_TRACE << "staring to parse login response=[" << m_body << "]";
    // message is in the m_body
    Json json(m_body.c_str());
    JSON_GET_ERROR_CODE(json, m_error_code);
    if ( E_ERROR_CODE_NORMAL != m_error_code ) {
        return parseError();
    }
    // parse success message
    // parsing user self stuff
    LOG_TRACE << "parsing user self stuff";
    JSON_GET_STR_VALUE_EXT(json, m_self_status, "status");
    JSON_GET_STR_VALUE_EXT(json, m_self_user_id_str, "user_id");
    JSON_GET_STR_VALUE_EXT(json, m_self_name, "name");
    JSON_GET_STR_VALUE_EXT(json, m_self_signature, "signature");
    JSON_GET_STR_VALUE_EXT(json, m_self_session_id, "session_id");

    // buddies
    LOG_TRACE << "parsing user buddies";
    std::vector<DStr> buddies;
    json.GetObjectArray(buddies, "buddies", NULL);
    int size = buddies.size();
    for ( int i = 0; i < size; ++i ) {
        Json json_buddy(buddies[i].Str());
        buddy_t buddy;
        JSON_GET_STR_VALUE_EXT(json_buddy, buddy.user_id, "user_id");
        JSON_GET_STR_VALUE_EXT(json_buddy, buddy.name, "name");
        JSON_GET_STR_VALUE_EXT(json_buddy, buddy.signature, "signature");
        JSON_GET_STR_VALUE_EXT(json_buddy, buddy.category, "category");
        JSON_GET_STR_VALUE_EXT(json_buddy, buddy.status, "status");
        m_buddies.push_back(buddy);
    }
    LOG_TRACE << "parsing user groups";
    // groups
    std::vector<DStr> groups;
    json.GetObjectArray(groups, "groups", NULL);
    size = groups.size();
    for ( int i = 0; i < size; ++i ) {
        Json json_group(groups[i].Str());
        group_t group;
        JSON_GET_STR_VALUE_EXT(json_group, group.group_id, "user_id");
        JSON_GET_STR_VALUE_EXT(json_group, group.name, "name");
        JSON_GET_STR_VALUE_EXT(json_group, group.signature, "signature");
        JSON_GET_STR_VALUE_EXT(json_group, group.admin_user_id, "admin_user_id");
        m_groups.push_back(group);
    }
    LOG_TRACE << "parse done";
    return true;
}

bool LoginResponse::compose(std::string& result_message)
{
    G_user_manager.showUser(m_self_user_id);
    UserRefer& user_refer = G_user_manager.getRefer(m_self_user_id);
    if ( user_refer.barebone() ) {
        LOG_WARN << "user [" << m_self_user_id.id() << "] not exist, return now";
        return false;
    }
    const UserDataDetail &user_detail = user_refer.detail();
    const UserMap& buddies = user_detail.buddies();

    Json json;
    std::vector<DStr> response_object;
    JSON_VEC_PUSH_INT(json, response_object, "error_code", (int)E_ERROR_CODE_NORMAL);
    JSON_VEC_PUSH_STR(json, response_object, "status", user_refer.statusStr());

    // for Json's fault, self information comes first
    // self info
    JSON_VEC_PUSH_STR(json, response_object, "user_id", user_detail.userId().id());
    JSON_VEC_PUSH_STR(json, response_object, "name", user_detail.registerName().c_str());
    JSON_VEC_PUSH_STR(json, response_object, "signature", user_detail.signature().c_str());
    JSON_VEC_PUSH_STR(json, response_object, "session_id", "TODO");

    // buddies
    std::vector<DStr> buddies_dstr_vec;
    UserMapConstIterator it = buddies.begin();
    for ( ; it != buddies.end(); ++it ) {
        DStr user_id, register_name, signature;
        // compose single buddy info
        std::vector<DStr> tmp;
        JSON_VEC_PUSH_STR(json, tmp, "user_id", it->first.c_str());
        JSON_VEC_PUSH_STR(json, tmp, "name", it->second.registerName().c_str());
        JSON_VEC_PUSH_STR(json, tmp, "signature", it->second.signature().c_str());
        JSON_VEC_PUSH_STR(json, tmp, "category", it->second.category().c_str());
        // get status
        UserRefer& tmp_refer = G_user_manager.getRefer(UserId(it->first.c_str()));
        if ( tmp_refer.barebone() ) {
            LOG_WARN << "not exist user=" << it->first << ", this should never happen here";
            return false;
        }
        JSON_VEC_PUSH_STR(json, tmp, "status", tmp_refer.statusStr());

        DStr single_buddy_str = json.SetObject(tmp, NULL);
        buddies_dstr_vec.push_back(single_buddy_str);
    }
    DStr buddies_object;           // key: "buddies"
    buddies_object = json.SetArray(buddies_dstr_vec, "buddies");
    response_object.push_back(buddies_object);
    // LOG_TRACE << "buddies:[" << buddies_object.Str() <<"]";

    // groups
    // compose response
    DStr response_dstr = json.SetObject(response_object, NULL);
    // set both result and member data
    result_message = response_dstr.Str();
    setResponseStr(response_dstr.Str());

    if ( ! isValidJson(m_response.c_str()) ) {
        LOG_ERROR <<"invalid json";
    } else {
        LOG_DEBUG <<"valid json";
    }

    LOG_INFO << "complete response:[" << m_response <<"]";
    return true;
}

ChatMessage::ChatMessage(const std::string &message_body,
                         bool is_from_client)
    : m_is_from_client (is_from_client)
{
    m_type = E_MESSAGE_TYPE_CHAT;
    m_body= message_body;
    LOG_TRACE << "chat message body=[" << m_body << "]";
    if (! parse()) {
        LOG_WARN << "failed to parse chat message when constructing, body=[" << m_body << "]";
        return;
    }
}

ChatMessage::ChatMessage(const std::string& from_val,
                         const std::string& to_val,
                         const std::string& content_val,
                         const std::string& time,
                         bool is_from_client,
                         const std::string& session_id)
    : m_from_str(from_val),
      m_to_str(to_val),
      m_content(content_val),
      m_time_str(time),
      m_is_from_client(is_from_client),
      m_session_id(session_id)
{
    m_type = E_MESSAGE_TYPE_CHAT;
    compose(m_body);
}

// server side
bool ChatMessage::process(const TcpConnectionPtr& self_conn)
{

    do {
        if (! parse()) {
            m_msg_protocol_valid = false;
            break;
        }
        if ( ! checkValidation() ) {
            LOG_ERROR << "failed to checkValidation";
            break;
        }
    } while ( 0 );
    if ( isErrorMessage() ) {
        composeErrorResponse();
        Packet resp_packet(m_response, getType());
        resp_packet.sendTo(self_conn);
        return true;
    }
    // send message to "m_to", first
    Json json;
    UserRefer& to_refer = G_user_manager.getRefer(m_to);
    LOG_TRACE << "to send message to TO user=" << m_to.id() << ", status=" << to_refer.statusStr();
    if ( to_refer.offline() ) {
        DStr error_info; error_info.AssignFmt("to=%s logout", m_to.id());
        setError(E_ERROR_CODE_INVALID_MESSAGE, error_info.Str(), "message can't be send");
        composeErrorResponse();
        Packet resp_packet(m_response, getType());
        resp_packet.sendTo(self_conn);
        // maybe if m_from is VIP, then we should store the message
        return true;
    }
    // send message to "to", no session id, original time
    ChatMessage msg_for_to(m_from_str,
                           m_to_str,
                           m_content,
                           m_time_str);
    LOG_TRACE << "chat message 4 to=[" << msg_for_to.bodyStr()<< "]";
    Packet pac_for_to(msg_for_to.bodyStr(), getType());
    if (to_refer.connection()->connected()) {
        // Packet pac_to(m_body, E_MESSAGE_TYPE_CHAT);
        pac_for_to.sendTo(to_refer.connection());
    }

    // send message to "from", no session id, original time
    UserRefer& from_refer = G_user_manager.getRefer(m_from);
    LOG_TRACE << "to send message to FROM user=" << m_from.id()
              << ", status=" << from_refer.statusStr();
    if ( from_refer.offline() ) {
        LOG_TRACE << "FROM=" << m_from.id() << " is down too";
        return true;
    }
    // with the FROM and TO switched
    ChatMessage msg_for_from(m_from_str,
                             m_to_str,
                             m_content,
                             m_time_str);
    LOG_TRACE << "chat message 4 from=[" << msg_for_from.bodyStr()<< "]";
    Packet pac_for_from(msg_for_from.bodyStr(), getType());
    if (from_refer.connection()->connected()) {
        pac_for_from.sendTo(from_refer.connection());
    }
    return true;
}

bool ChatMessage::composeErrorResponse()
{
    LOG_TRACE << "composing error response"
              << ", m_error_code=" << m_error_code
              << ", m_error_info=" << m_error_info
              << ", m_extra_info=" << m_extra_info;

    if ( ! errorCodeValid(m_error_code) ) {
        LOG_ERROR << "invalid error code to be send, error_code=" << m_error_code;
        setError(E_ERROR_CODE_SERVER_INTERNAL_ERROR, ERROR_INFO_DEFAULT, "");
        // do not return here.
    }
    Json json;
    std::vector<DStr> body_vec;
    // chat message specific
    JSON_VEC_PUSH_STR(json, body_vec, "from", m_from_str.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "to", m_to_str.c_str());
    // basic
    // NOTICE: must convert (e_error_code) to (int)
    JSON_VEC_PUSH_INT(json, body_vec, "error_code", (int)m_error_code);
    JSON_VEC_PUSH_STR(json, body_vec, "error_info", m_error_info.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "extra_info", m_extra_info.c_str());

    m_response = json.SetObject(body_vec, NULL).Str();
    return true;
}

bool ChatMessage::compose(std::string& result_message)
{
    Json json;
    std::vector<DStr> body_vec;

    // basic
    if ( ! m_is_from_client ) {
        JSON_VEC_PUSH_INT(json, body_vec, "error_code", (int)m_error_code);
        JSON_VEC_PUSH_STR(json, body_vec, "error_info", m_error_info.c_str());
        JSON_VEC_PUSH_STR(json, body_vec, "extra_info", m_extra_info.c_str());
    }
    // chat message - specific info
    JSON_VEC_PUSH_STR(json, body_vec, "from", m_from_str.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "to", m_to_str.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "content", m_content.c_str());
    if ( ! m_time_str.compare("now") ) {
        Time time;
        m_time_str = time.getNowTime();
    }
    JSON_VEC_PUSH_STR(json, body_vec, "time", m_time_str.c_str());
    // must contains session id for msg which comes from client
    if (m_is_from_client) {
        JSON_VEC_PUSH_STR(json, body_vec, "session_id", m_session_id.c_str());
    }

    DStr tmp = json.SetObject(body_vec, NULL);
    result_message = tmp.Str();
    LOG_TRACE << "after compose, chat message=["<< result_message << "]";
    return true;
}

bool ChatMessage::parseError()
{
    if ( 0 == m_body.size() ) {
        LOG_WARN << "m_body not initialized";
        return false;
    }
    Json json(m_body.c_str());
    // chat message specific info
    JSON_GET_STR_VALUE_EXT(json, m_from_str, "from");
    JSON_GET_STR_VALUE_EXT(json, m_to_str, "to");
    // basic
    JSON_GET_ERROR_CODE(json, m_error_code);
    JSON_GET_STR_VALUE_EXT(json, m_error_info, "error_info");
    JSON_GET_STR_VALUE_EXT(json, m_extra_info, "extra_info");

    return true;
}

bool ChatMessage::parse()
{
    LOG_DEBUG << "parsing chat message body:\"" << m_body<<"\"";
    Json json(m_body.c_str());

    if ( ! m_is_from_client ) {
        JSON_GET_ERROR_CODE(json, m_error_code);
        if ( E_ERROR_CODE_NORMAL != m_error_code ) {
            return parseError();
        }
    }

    // parse success message
    // from
    JSON_GET_STR_VALUE_EXT(json, m_from_str, "from");
    m_from.dup(m_from_str.c_str());
    LOG_DEBUG << "get body's from=" << m_from.id();
    // to
    JSON_GET_STR_VALUE_EXT(json, m_to_str, "to");
    m_to.dup(m_to_str.c_str());
    LOG_DEBUG << "get body's to=" << m_to.id();
    // content
    JSON_GET_STR_VALUE_EXT(json, m_content, "content");
    LOG_DEBUG << "get body's content=" << m_content;
    // time
    JSON_GET_STR_VALUE_EXT(json, m_time_str, "time");
    LOG_DEBUG << "get body's time=" << getTimeStr();
    // session id only for server' parser
    if ( m_is_from_client ) {
        JSON_GET_STR_VALUE_EXT(json, m_session_id, "session_id");
    }
    m_msg_protocol_valid = true;
    return true;
}

bool ChatMessage::checkValidation()
{
    LOG_TRACE << "checking chat message validation";
    DStr error_info;
    // check protocol validation generated by parse()
    if ( ! m_msg_protocol_valid ) {
        LOG_ERROR << "not valid chat protocol , body:" << m_body;
        setError(E_ERROR_CODE_INVALID_PROTOCOL, ERROR_INFO_DEFAULT,
                 m_body.c_str());
        return false;
    }

    // check whether "from" already login or not
    UserRefer& from_refer = G_user_manager.getRefer(m_from);
    if ( from_refer.offline() ) {
        LOG_WARN << "is off line, user=" << m_from.id();
        error_info.Empty(); error_info.AssignFmt("from=%s is off line", m_from.id());
        setError(E_ERROR_CODE_INVALID_MESSAGE, error_info.Str(), "please login before chat");
        return false;
    }
    // check whether "to" user exists
    UserRefer& to_refer = G_user_manager.getRefer(m_to);
    if ( to_refer.barebone() ) {
        LOG_WARN << "not exist user=" << m_to.id() << ", this should never happen here";
        error_info.Empty(); error_info.AssignFmt("user(id:%s) not exists", m_to.id());
        setError(E_ERROR_CODE_INVALID_MESSAGE, error_info.Str(), "should never happen");
        return false;
    }
    const std::string& to_name = to_refer.detail().registerName();
    if ( to_refer.offline() ) {
        LOG_WARN << "is off line, user=" << m_to.id();
        error_info.Empty();
        error_info.AssignFmt("%s is off line, message won't be sent",to_name.c_str());
        setError(E_ERROR_CODE_INVALID_MESSAGE, error_info.Str(), "");
        return false;
    }

	return m_msg_data_valid;
}

LoginStatusNotification::LoginStatusNotification(const std::string& message_body)
{
    m_body = message_body;
    m_type = E_MESSAGE_TYPE_NOTIFICATION_LOGIN_STATUS;
    // parse immediately after constructing
    if ( ! parse() ) {
        LOG_ERROR << "failed to parse coming message body";
        return;
    }
}

LoginStatusNotification::LoginStatusNotification(const UserId& from, e_status status)
    : m_from_str(from.id()), m_status(status) {
    m_status_str = ::getStatusStr(status);
    LOG_TRACE << "constructing login notification";
    m_type = E_MESSAGE_TYPE_NOTIFICATION_LOGIN_STATUS;
}

bool LoginStatusNotification::compose(std::string& result_message)
{
    LOG_TRACE << "composing login notification";
    Json json;
    std::vector<DStr> body_vec;
    JSON_VEC_PUSH_STR(json, body_vec, "from", m_from_str.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "status", m_status_str.c_str());

    DStr notification_body = json.SetObject(body_vec, NULL);
    result_message = notification_body.Str();
    LOG_TRACE << "successfully compose login notification=["
              << result_message << "]";
    return true;
}

bool LoginStatusNotification::parse()
{
    LOG_TRACE << "parsing login notification";

    Json json(m_body.c_str());
    JSON_GET_STR_VALUE_EXT(json, m_from_str, "from");
    JSON_GET_STR_VALUE_EXT(json, m_status_str, "status");

    if ( ! getStatus(m_status, m_status_str.c_str()) ) {
        LOG_ERROR << "failed to get status when parsing";
        return false;
    }
    LOG_TRACE << "successfully to parse login notification";
    return true;
}

// for login on-line is handle by login request, and for server, the
// login status notification only process "off-line" type notification
bool LoginStatusNotification::process(const TcpConnectionPtr& self_conn)
{
    // we do not need to self connection in function
    if ( isOnLineNotification() ) {
        // for on line, already handled by login request process
        LOG_TRACE << "login status notification is on-line type, "
                  << "do nothing on server side";
        return true;
    }

    // off-line
    // TODO: [2013-12-06] move on-line/off-line status from user manger to into
    // session manager
    // update user manager's data
    LOG_TRACE << "handle off-line notification, update user manager";
    UserId self_user_id;
    self_user_id.dup(m_from_str.c_str());
    G_user_manager.updateStatus(self_user_id, E_STATUS_OFFLINE);

    // notify all on-line buddies that "I'am off line now"
    LOG_TRACE << "send off-line notification to on-line friends";
    // find buddies
    const UserMap& buddies = G_user_manager.getUser(self_user_id).buddies();
    if ( 0 == buddies.size() ) {
        LOG_TRACE << "no buddy of user=" << self_user_id.id()
                  << " is online, do nothing";
        return true;
    }
    // has on-line buddies
    LoginStatusNotification ls_notf(self_user_id, E_STATUS_ONLINE);
    std::string notf_str;
    ls_notf.compose(notf_str);
    Packet packet(notf_str, ls_notf.getType());
    // loop to send
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

FriendsManagementMessage::FriendsManagementMessage(const std::string& message_body)
{
    m_body = message_body;
    m_type = E_MESSAGE_TYPE_FRIENDS_MANAGEMENT;
    if (! parse() ) {
        LOG_ERROR << "failed to parse coming message body";
        return;
    }
}

FriendsManagementMessage::FriendsManagementMessage(
    e_friends_manage_action action,
    const std::string& requester_id_str,
    const std::string& target_id_str,
    const std::string& category,
    const std::string& session_id)
    : m_action(action),
      m_requester_id_str(requester_id_str),
      m_target_id_str(target_id_str),
      m_category(category),
      m_session_id(session_id)
{
    m_type = E_MESSAGE_TYPE_FRIENDS_MANAGEMENT;
    compose(m_body);
}

bool FriendsManagementMessage::compose(std::string& result_message)
{
    LOG_TRACE << "composing friends management message";
    Json json;
    std::vector<DStr> body_vec;
    JSON_VEC_PUSH_INT(json, body_vec, "action", m_action);
    JSON_VEC_PUSH_STR(json, body_vec, "requester_id", m_requester_id_str.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "target_id", m_target_id_str.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "session_id", m_session_id.c_str());

    if ( FRIENDS_MANAGE_ACTION_ADD == m_action ||
         FRIENDS_MANAGE_ACTION_UPDATE == m_action) {
        JSON_VEC_PUSH_STR(json, body_vec, "category", m_category.c_str());
    }

    DStr msg_body = json.SetObject(body_vec, NULL);
    result_message = msg_body.Str();
    LOG_TRACE << "successfully friends management message=["
              << result_message << "]";
    return true;
}

bool isFriendsManageActionValid(e_friends_manage_action action)
{
    return (action < FRIENDS_MANAGE_ACTION_UPPER_BOUND &&
            action >= FRIENDS_MANAGE_ACTION_ADD);
}

bool FriendsManagementMessage::parse()
{
    LOG_TRACE << "parsing friends management message";

    Json json(m_body.c_str());
    int action;
    JSON_GET_INT_VALUE_EXT(json, action, "action");
    m_action = (e_friends_manage_action)action;
    JSON_GET_STR_VALUE_EXT(json, m_requester_id_str, "requester_id");
    JSON_GET_STR_VALUE_EXT(json, m_target_id_str, "target_id");
    JSON_GET_STR_VALUE_EXT(json, m_session_id, "session_id");
    if ( ! isFriendsManageActionValid(m_action) ) {
        LOG_ERROR << "invalid Friends management action=" << (int)m_action;
        return false;
    }
    if ( FRIENDS_MANAGE_ACTION_ADD == m_action ||
         FRIENDS_MANAGE_ACTION_UPDATE == m_action) {
        JSON_GET_STR_VALUE_EXT(json, m_category, "category");
    }
    LOG_TRACE << "successfully to parse friends management message";
    return true;
}

bool FriendsManagementMessage::checkValidation()
{
    // check whether "target" user exists
    UserId target_id = m_target_id_str.c_str();
    UserRefer& target_refer = G_user_manager.getRefer(target_id);
    if ( target_refer.barebone() ) {
        LOG_WARN << "not exist user=" << target_id.id() << ", this should never happen here";
        DStr error_info;
        error_info.AssignFmt("user(id:%s) not exists", target_id.id());
        setError(E_ERROR_CODE_INVALID_MESSAGE, error_info.Str(), "");
        return false;
    }
    // check session id
    // TODO: [2013-12-13]
    return true;
}

bool FriendsManagementMessage::composeErrorResponse()
{
    LOG_TRACE << "composing error response"
              << ", m_error_code=" << m_error_code
              << ", m_error_info=" << m_error_info
              << ", m_extra_info=" << m_extra_info;

    if ( ! errorCodeValid(m_error_code) ) {
        LOG_ERROR << "invalid error code to be send, error_code=" << m_error_code;
        setError(E_ERROR_CODE_SERVER_INTERNAL_ERROR, ERROR_INFO_DEFAULT, "");
        // do not return here.
    }
    Json json;
    std::vector<DStr> body_vec;
    JSON_VEC_PUSH_STR(json, body_vec, "requester_id", m_requester_id_str.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "target_id", m_target_id_str.c_str());
    // basic
    // NOTICE: must convert (e_error_code) to (int)
    JSON_VEC_PUSH_INT(json, body_vec, "error_code", (int)m_error_code);
    JSON_VEC_PUSH_STR(json, body_vec, "error_info", m_error_info.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "extra_info", m_extra_info.c_str());

    m_response = json.SetObject(body_vec, NULL).Str();
    return true;
}

bool FriendsManagementMessage::processManageActionSearch(
    const muduo::net::TcpConnectionPtr& self_conn)
{
    // is already friend
    UserId requester_id = m_requester_id_str.c_str();
    UserId target_id = m_target_id_str.c_str();

    // target details
    UserRefer& target_refer = G_user_manager.getRefer(target_id);
    const UserDataDetail &target_detail = target_refer.detail();
    bool is_alread_friend = target_detail.isMyFriend(requester_id);

    FriendsManagementResponse search_resp( FRIENDS_MANAGE_ACTION_SEARCH,
                                           target_detail.userIdStr(),
                                           target_detail.registerName(),
                                           target_detail.signature(),
                                           target_refer.statusStr(),
                                           "", // search return none
                                           is_alread_friend);
    std::string search_response;
    bool rc = search_resp.compose(search_response);
    if ( ! rc ) {
        LOG_ERROR << "failed to compose search response";
        return false;
    }

    Packet resp_packet(
        search_response,
        E_MESSAGE_TYPE_FRIENDS_MANAGEMENT_RESPONSE);
    resp_packet.sendTo(self_conn);

    return true;
}

bool FriendsManagementMessage::processManageActionAdd(
    const muduo::net::TcpConnectionPtr& self_conn)
{
    std::string response_msg;

    UserId requester_id = m_requester_id_str.c_str();
    UserId target_id = m_target_id_str.c_str();
    DStr error_info;
    bool rc;

    bool is_alread_friend = false;
    rc = G_user_manager.makeFriends(requester_id, target_id,
                                    m_category,
                                    is_alread_friend);
    if ( !rc ) {
        error_info.AssignFmt("failed to add %s as %s's %s",
                             target_id.id(),
                             requester_id.id(),
                             m_category.c_str());
        LOG_ERROR << error_info.Str();
        setError(E_ERROR_CODE_SERVER_INTERNAL_ERROR, error_info.Str());
        rc = composeErrorResponse();
        response_msg = m_response;
        Packet packet( response_msg,
                       E_MESSAGE_TYPE_FRIENDS_MANAGEMENT_RESPONSE);
        // only send to self
        packet.sendTo(self_conn);
        return true;
    }

    ////////////////////////////////////////////////////////////////
    // now compose success result response for request user
    UserRefer& target_refer = G_user_manager.getRefer(target_id);
    const UserDataDetail &target_detail = target_refer.detail();
    FriendsManagementResponse resp_for_requester( FRIENDS_MANAGE_ACTION_ADD,
                                                  target_detail.userIdStr(),
                                                  target_detail.registerName(),
                                                  target_detail.signature(),
                                                  target_refer.statusStr(),
                                                  m_category,
                                                  is_alread_friend);
    rc = resp_for_requester.compose(response_msg);
    if ( ! rc ) {
        LOG_ERROR << "failed to compose add response for request user";
        return false;
    }
    Packet packet_for_requester( response_msg,
                                 E_MESSAGE_TYPE_FRIENDS_MANAGEMENT_RESPONSE);
    packet_for_requester.sendTo(self_conn);

    ////////////////////////////////////////////////////////////////
    // now compose success result response for target user
    UserRefer& requester_refer = G_user_manager.getRefer(requester_id);
    const UserDataDetail& requester_detail = requester_refer.detail();
    FriendsManagementResponse resp_for_target( FRIENDS_MANAGE_ACTION_ADD,
                                               requester_detail.userIdStr(),
                                               requester_detail.registerName(),
                                               requester_detail.signature(),
                                               requester_refer.statusStr(),
                                               m_category,
                                               is_alread_friend);
    rc = resp_for_target.compose(response_msg);
    if ( ! rc ) {
        LOG_ERROR << "failed to compose add response for target user";
        return false;
    }
    Packet packet_for_target( response_msg,
                              E_MESSAGE_TYPE_FRIENDS_MANAGEMENT_RESPONSE);
    packet_for_target.sendTo(target_refer.connection());

    // end
    return true;
}

bool FriendsManagementMessage::process(const muduo::net::TcpConnectionPtr& self_conn)
{
    do {
        if ( ! parse() ) {
            m_msg_protocol_valid = false;
            setError(E_ERROR_CODE_INVALID_PROTOCOL, ERROR_INFO_DEFAULT,
                     m_body.c_str());
            break;
        }
        if (! checkValidation()) {
            LOG_ERROR << "failed to checkValidation";
            break;
        }
    } while ( 0 );
    if ( isErrorMessage() ) {
        composeErrorResponse();
        Packet resp_packet(
            m_response,
            E_MESSAGE_TYPE_FRIENDS_MANAGEMENT_RESPONSE);
        resp_packet.sendTo(self_conn);
        return true;
    }

    bool rc;
    std::string response_msg;

    switch (m_action) {
    case FRIENDS_MANAGE_ACTION_ADD: {
        rc = processManageActionAdd(self_conn);
        break;
    }
    case FRIENDS_MANAGE_ACTION_SEARCH: {
        rc = processManageActionSearch(self_conn);
        break;
    }
    case FRIENDS_MANAGE_ACTION_UPDATE: {
        break;
    }
    case FRIENDS_MANAGE_ACTION_DELETE: {
        break;
    }
    default:
        LOG_ERROR << "unknown action type=" << (int)m_action;
        return false;
    }

    if ( ! rc ) {
        LOG_ERROR << "failed to process this message";
        return false;
    }
    return true;
}

FriendsManagementResponse::FriendsManagementResponse(
    const std::string& message_body)
{
    m_body = message_body;
    m_type = E_MESSAGE_TYPE_FRIENDS_MANAGEMENT_RESPONSE;
    if ( ! parse() ) {
        LOG_ERROR << "failed to parse coming message body";
        return;
    }
}

FriendsManagementResponse::FriendsManagementResponse(
    e_friends_manage_action request_action,
    const std::string& target_id_str,
    const std::string& target_name,
    const std::string& target_signature,
    const std::string& target_status,
    const std::string& target_category,
    bool is_already_friend )
    : m_request_action(request_action),
      m_target_id_str(target_id_str),
      m_target_name(target_name),
      m_target_signature(target_signature),
      m_target_status(target_status),
      m_target_category(target_category),
      m_is_already_friend(is_already_friend)
{
    m_type = E_MESSAGE_TYPE_FRIENDS_MANAGEMENT_RESPONSE;
}

bool FriendsManagementResponse::compose(std::string& result_message)
{
    LOG_TRACE << "composing friends management add response message";
    Json json;
    std::vector<DStr> body_vec;

    // error code
    JSON_VEC_PUSH_INT(json, body_vec, "error_code", m_error_code);
    // message specific stuff
    JSON_VEC_PUSH_INT(json, body_vec, "request_action", m_request_action);
    JSON_VEC_PUSH_STR(json, body_vec, "target_id", m_target_id_str.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "target_name", m_target_name.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "target_signature", m_target_signature.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "target_status", m_target_status.c_str());
    JSON_VEC_PUSH_STR(json, body_vec, "target_category", m_target_category.c_str());
    JSON_VEC_PUSH_BOOL(json, body_vec, "already_friend", m_is_already_friend);

    DStr msg_body = json.SetObject(body_vec, NULL);
    result_message = msg_body.Str();
    LOG_TRACE << "successfully compose friends management "
        "add response message=[" << result_message << "]";
    return true;
}

bool FriendsManagementResponse::parseError()
{
    if ( 0 == m_body.size() ) {
        LOG_WARN << "m_body not initialized";
        return false;
    }
    Json json(m_body.c_str());
    // chat message specific info
    JSON_GET_STR_VALUE_EXT(json, m_target_id_str, "target_id");
    // basic
    JSON_GET_ERROR_CODE(json, m_error_code);
    JSON_GET_STR_VALUE_EXT(json, m_error_info, "error_info");
    JSON_GET_STR_VALUE_EXT(json, m_extra_info, "extra_info");
    return true;
}

bool FriendsManagementResponse::parse()
{
    LOG_TRACE << "parsing friends management add response message";
    Json json(m_body.c_str());

    JSON_GET_ERROR_CODE(json, m_error_code);
    if ( isErrorMessage() ) {
        LOG_TRACE << "message is error message";
        return parseError();
    }

    int request_action;
    JSON_GET_INT_VALUE_EXT(json, request_action, "request_action");
    m_request_action = (e_friends_manage_action)request_action;

    JSON_GET_STR_VALUE_EXT(json, m_target_id_str, "target_id");
    JSON_GET_STR_VALUE_EXT(json, m_target_name, "target_name");
    JSON_GET_STR_VALUE_EXT(json, m_target_signature, "target_signature");
    JSON_GET_STR_VALUE_EXT(json, m_target_status, "target_status");
    JSON_GET_STR_VALUE_EXT(json, m_target_category, "target_category");
    JSON_GET_BOOL_VALUE_EXT(json, m_is_already_friend, "already_friend");

    LOG_TRACE << "successfully parse friends management add response message";
    return true;
}

bool FriendsManagementResponse::isTargetOnline() const
{
    e_status status;
    bool rc = ::getStatus(status, m_target_status.c_str());
    if ( ! rc ) {
        LOG_ERROR << "failed to convert target status";
        return false;
    }
    return isOnLine(status);
}
