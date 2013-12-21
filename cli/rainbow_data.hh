// this file is used to store GUI data, incoming is login response, this class
// used to manipulate and manage this data when communication with GUI

#ifndef INCLUDE_RAINBOW_DATA_HPP
#define INCLUDE_RAINBOW_DATA_HPP

#include <glib.h>
#include <string>
#include <vector>
#include <map>

#include <muduo/base/Mutex.h>

// user_bar
#define OFF_LINE "off-line"
#define ON_LINE "on-line"

class buddy_t;
class group_t;
// class ChatPair;
struct ChatPair;

class BuddyData
{
public:
    BuddyData() {}
    BuddyData(const std::string& user_id,
              const std::string& name,
              const std::string& signature,
              const std::string& status,
              const std::string& category);

    BuddyData(const buddy_t& buddy);
    ~BuddyData() {}

    bool isShowingChat() {
        return m_showing_inchat_window;
    }
    void setChatShowing(bool val) {
        m_showing_inchat_window = val;
    }
    const std::string& getUserIdStr() const {
        return m_user_id_str;
    }
    const std::string& getName()  const {
        return m_name;
    }
    const std::string& getSignature()  const {
        return m_signature;
    }
    const std::string& getStatus()  const {
        return m_status;
    }
    const std::string& getCategory()  const {
        return m_category;
    }
    bool isOnLine() const {
        return (m_status == ON_LINE);
    }
    bool isOffLine() const {
        return (m_status == OFF_LINE);
    }
    void setOnLineStatus(bool online) {
        m_status = online ? ON_LINE: OFF_LINE;
    }
    void setSignature(const char* signature);

private:
    std::string m_user_id_str;
    std::string m_name;
    std::string m_signature;
    std::string m_status;
    std::string m_category;
    bool m_showing_inchat_window;
};

class GroupData
{
public:
    GroupData() {}
    GroupData(const group_t& group);
    ~GroupData() {}

    bool isShowingChat() {
        return m_showing_inchat_window;
    }
    void setChatShowing(bool val) {
        m_showing_inchat_window = val;
    }
    const std::string& getGroupIdStr() const {
        return m_group_id_str;
    }
    const std::string& getName()  const {
        return m_name;
    }
    const std::string& getSignature()  const {
        return m_signature;
    }
    const std::string& getAdminUserIdStr()  const {
        return m_admin_user_id_str;
    }

private:
    std::string m_group_id_str;
    std::string m_name;
    std::string m_signature;
    std::string m_admin_user_id_str;
    bool m_showing_inchat_window;
};

enum chat_type
{
    CHAT_TYPE_USER,
    CHAT_TYPE_GROUP,
};

extern bool isChatTypeValid(chat_type type);

class ChatSideBarItem
{
public:
    ChatSideBarItem(chat_type type, void* data);
    ~ChatSideBarItem() {}
    bool isUser() { return (m_type == CHAT_TYPE_USER); }
    bool isGroup() { return (m_type == CHAT_TYPE_GROUP); }
    void* getData() { return m_data; }
    chat_type getType() { return m_type; }
private:
    chat_type m_type;
    // data pointer to BuddyData or GroupData, and this data should
    // NEVER be erase by this class, it's managed by another lists
    void *m_data;
};

enum category_info_action
{
    CATEGORY_INFO_ACTION_PLUS_TOTAL,
    CATEGORY_INFO_ACTION_MINUS_TOTAL,
    CATEGORY_INFO_ACTION_PLUS_ON,
    CATEGORY_INFO_ACTION_MINUS_ON,
    CATEGORY_INFO_ACTION_UPPER_BOUND
};

extern bool isCategoryInfoActionValid(category_info_action action);

class RainBowGuiDataManager
{
public:
    RainBowGuiDataManager();
    ~RainBowGuiDataManager();

    void storeSelfData(const char* self_user_id_str,
                       const char* self_name,
                       const char* self_signature,
                       const char* self_status,
                       const char* self_session_id) {
        m_self_user_id_str = self_user_id_str;
        m_self_name = self_name;
        m_self_signature = self_signature;
        m_self_status = self_status;
        m_self_session_id = self_session_id;
    }

    const std::string& getSelfUserIdStr() const { return m_self_user_id_str; }
    const std::string& getSelfSessionId() const { return m_self_session_id; }
    const std::string& getSelfName() const { return m_self_name; }

    void storeBuddyData(BuddyData* data) {
        muduo::MutexLockGuard _lock(m_mutex);
        m_data_buddies = g_list_append(m_data_buddies, (void*)data);
    }
    void storeGroupData(GroupData* data) {
        muduo::MutexLockGuard _lock(m_mutex);
        m_data_groups = g_list_append(m_data_groups, (void*)data);
    }
    void storeDataChatSideBarItem(ChatSideBarItem* data) {
        muduo::MutexLockGuard _lock(m_mutex);
        m_data_chat_sidebar_items = g_list_append( m_data_chat_sidebar_items, (void*)data);
    }
    void storeDataChatPairs(ChatPair* data) {
        muduo::MutexLockGuard _lock(m_mutex);
        m_data_chat_pairs = g_list_append( m_data_chat_pairs, (void*)data);
    }

    GList *getDataBuddies() { return m_data_buddies; }
    GList* getDataGroups() { return m_data_groups; }
    GList* getChatPairs() { return m_data_chat_pairs;}
    GList* getSidebarItems() { return m_data_chat_sidebar_items; }

    const std::string& getBuddyName_Guard(const char* user_id_str) {
        muduo::MutexLockGuard _lock(m_mutex);
        return getBuddyName(user_id_str);
    }
    BuddyData* findBuddy_Guard(const char* id_str) {
        muduo::MutexLockGuard _lock(m_mutex);
        return findBuddy(id_str);
    }
    GroupData* findGroup_Guard(const char* id_str) {
        muduo::MutexLockGuard _lock(m_mutex);
        return findGroup(id_str);
    }
    ChatPair* findChatPair_Guard(const char* common_id_str) {
        muduo::MutexLockGuard _lock(m_mutex);
        return findChatPair(common_id_str);
    }
    bool isChatWindowCreated() const {
        muduo::MutexLockGuard _lock(m_mutex);
        return m_chat_window_created;
    }
    void setChatWindowCreated() {
        muduo::MutexLockGuard _lock(m_mutex);
        m_chat_window_created = true;
    }

    // category info part
    // plus_total, decrease_total, plus_on, decrease_on
    bool handleCategoryInfo_Guard(const std::string& category, category_info_action action, int num) {
        muduo::MutexLockGuard _lock(m_mutex);
        return handleCategoryInfo(category, action, num);
    }

    bool getCategoryInfo_Guard(const std::string& category, int& on, int& total) {
        muduo::MutexLockGuard _lock(m_mutex);
        return getCategoryInfo(category, on, total);
    }
    void getCategories_Guard(std::vector<std::string>& category_vec) {
        muduo::MutexLockGuard _lock(m_mutex);
        getCategories(category_vec);
    }

private:
    const std::string& getBuddyName(const char* user_id_str);
    BuddyData* findBuddy(const char* user_id_str);
    GroupData* findGroup(const char* group_id_str);
    ChatPair* findChatPair(const char* common_id_str);
    bool handleCategoryInfo(const std::string& category, category_info_action action, int num);
    bool getCategoryInfo(const std::string& category, int& on, int& total);
    void getCategories(std::vector<std::string>& category_vec);

private:
    std::string m_self_user_id_str;
    std::string m_self_name;
    std::string m_self_signature;
    std::string m_self_status;
    std::string m_self_session_id;

    bool m_chat_window_created;

    // NOTICE: GList must be initialized as NULL
    GList* m_data_buddies;
    GList* m_data_groups;
    GList* m_data_chat_sidebar_items;
    GList* m_data_chat_pairs;

    // category info statics
    struct on_total_t{
        int on;
        int total;
    } ;
    typedef std::map<std::string, on_total_t> CategoryInfoMap;
    typedef CategoryInfoMap::iterator CategoryInfoMapIterator;
    typedef CategoryInfoMap::const_iterator CategoryInfoMapConstIterator;
    typedef std::pair<CategoryInfoMapIterator, bool> CategoryInfoMapPair;

    CategoryInfoMap m_category_info_map;
    // protect all data in manager
    mutable muduo::MutexLock m_mutex;
};

#endif /* INCLUDE_RAINBOW_DATA_HPP */
