#include "user_data.hh"
#include "global.hh"

long UserIdGenerator::m_current_id_number = 11;

static status_info s_status[] = {
    // WARNING: keep index same as e_status for fast access
    { E_STATUS_LOWER_BOUND, ""},
    { E_STATUS_ONLINE, "on-line" },
    { E_STATUS_OFFLINE, "off-line" },
    { E_STATUS_HIDING, "hiding" },
    // to, add
    { E_STATUS_UPPER_BOUND, NULL} // use NULL to terminate the list
};

bool isStatusValid(e_status status)
{
    return (status < E_STATUS_UPPER_BOUND && status > E_STATUS_LOWER_BOUND);
}

const char* getStatusStr(e_status status)
{
    if (! isStatusValid(status)) {
        LOG_ERROR << "status out of range, status=" << (int)status;
        status = E_STATUS_LOWER_BOUND;
    }
    return s_status[status].str;
}

bool getStatus(e_status& status, const char* status_str)
{
    if ( ! status_str || !*status_str ) {
        LOG_ERROR << "invalid argument, status str is NULL or empty";
        return false;
    }
    // loop to get status
    int idx = 0;
    do {
        if ( ! strcmp(status_str, s_status[idx].str) ) { // equal
            status = (e_status) idx;
            // check again
            if ( ! isStatusValid(status) ) {
                LOG_ERROR << "status gotten is invalid, status=" << (int) status;
                return false;
            }
            return true;
        }
    } while ( s_status[++idx].str );
    return false;
}

bool isOnLine(e_status status)
{
    return (status == E_STATUS_ONLINE);
}

bool UserDataDetail::isMyFriend(const UserId& user_id) const
{
    if ( m_user_id == user_id ) {
        LOG_TRACE << "is myself, treated as my friend";
        return true;
    }
    std::string user_id_str;
    user_id_str = user_id.id();

    UserMapConstIterator it = m_buddies.find(std::string(user_id_str));
    return ( m_buddies.end() != it );
}
