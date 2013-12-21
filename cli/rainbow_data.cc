#include "rainbow_data.hh"
#include "../user_data.hh"
#include "log.hh"
#include "gui.hh"
#include "chat_gui.hh"

bool isChatTypeValid(chat_type type)
{
    return (type == CHAT_TYPE_USER || type == CHAT_TYPE_GROUP);
}


BuddyData::BuddyData(const std::string& user_id,
                     const std::string& name,
                     const std::string& signature,
                     const std::string& status,
                     const std::string& category) :
    m_user_id_str(user_id),
    m_name(name),
    m_signature(signature),
    m_status(status),
    m_category(category),
    m_showing_inchat_window(false)
{
    PR_TRACE("constructing BuddyData from pieces");
}


BuddyData::BuddyData(const buddy_t& buddy)
{
    PR_TRACE("constructing BuddyData from buddy_t");

    m_user_id_str = buddy.user_id;
    m_name = buddy.name;
    m_signature = buddy.signature;
    m_status = buddy.status;
    m_category = buddy.category;
    m_showing_inchat_window = false;
}

void BuddyData::setSignature(const char* signature)
{
    if ( ! signature ) {
        PR_ERROR("invalid argument, signature is NULL");
        return;
    }
    m_signature = signature;
}

GroupData::GroupData(const group_t& group)
{
    m_group_id_str = group.group_id;
    m_name = group.name;
    m_signature = group.signature;
    m_admin_user_id_str = group.admin_user_id;
    m_showing_inchat_window = false;
}

ChatSideBarItem::ChatSideBarItem(chat_type type, void* data)
    : m_type(type), m_data(data)
{
    PR_TRACE("constructing chat sidebar item, type=%s",
             m_type == CHAT_TYPE_USER ? "user" : "group");
}

RainBowGuiDataManager::RainBowGuiDataManager()
{
    m_data_buddies = NULL;
    m_data_groups = NULL;
    m_data_chat_sidebar_items = NULL;
    m_data_chat_pairs = NULL;

    m_chat_window_created = false;
}

RainBowGuiDataManager::~RainBowGuiDataManager()
{
    // free buddies
    GList* items = m_data_buddies;
    while ( items ) {
        delete (BuddyData*)(items->data);
        items = g_list_next(items);
    }
    g_list_free(m_data_buddies);
    // free groups
    items = m_data_groups;
    while ( items ) {
        delete (GroupData*)(items->data);
        items = g_list_next(items);
    }
    g_list_free(m_data_groups);
    // TODO: [2013-12-04]
}

const std::string& RainBowGuiDataManager::getBuddyName(const char* user_id_str)
{
    static const std::string __empty_name = "";
    const BuddyData* data = findBuddy(user_id_str);
    if ( ! data ) {
        PR_WARN("not found for user=%s", user_id_str);
        return __empty_name;
    }
    return data->getName();
}

BuddyData* RainBowGuiDataManager::findBuddy(const char* user_id_str)
{
    // TODO: [2013-12-05] to lock m_data_buddies
    if ( ! user_id_str ) {
        PR_ERROR("invalid argument, user_id_str is NULL");
        return NULL;
    }
    GList* items = m_data_buddies;
    BuddyData* data = NULL;
    while ( items ) {
        data = (BuddyData*)(items->data);
        const std::string &tmp = (data)->getUserIdStr();
        if ( ! tmp.compare(user_id_str) ) { // equal
            return data;
        }
        items = g_list_next(items);
    }
    return NULL;
}

GroupData* RainBowGuiDataManager::findGroup(const char* group_id_str)
{
    if ( ! group_id_str ) {
        PR_ERROR("invalid argument, group_id_str is NULL");
        return NULL;
    }
    GList* items = m_data_groups;
    GroupData* data = NULL;
    while ( items ) {
        data = (GroupData*)(items->data);
        const std::string& tmp = data->getGroupIdStr();
        if ( ! tmp.compare(group_id_str) ) { // equal
            return data;
        }
        items = g_list_next(items);
    }
    return NULL;
}

ChatPair* RainBowGuiDataManager::findChatPair(const char* common_id_str)
{
    if ( ! common_id_str ) {
        PR_ERROR("invalid argument, common_id_str is NULL");
        return NULL;
    }
    GList* items = m_data_chat_pairs;
    ChatPair* data = NULL;
    while ( items ) {
        data = (ChatPair*)(items->data);
        const std::string& tmp = data->getCommonId();
        if ( ! tmp.compare(common_id_str) ) { // equal
            PR_TRACE("chat pair found, id=%s", common_id_str);
            return data;
        }
        items = g_list_next(items);
    }
    return NULL;
}


bool isCategoryInfoActionValid(category_info_action action)
{
    return (action >= CATEGORY_INFO_ACTION_PLUS_TOTAL
            || action < CATEGORY_INFO_ACTION_UPPER_BOUND);
}

// category info part
// plus_total, decrease_total, plus_on, decrease_on
bool RainBowGuiDataManager::handleCategoryInfo(const std::string& category,
                                               category_info_action action,
                                               int num)
{
    if ( ! isCategoryInfoActionValid(action) ) {
        PR_ERROR("invalid action type=%d", (int)action);
        return false;
    }
    // check if category already exist
    CategoryInfoMapIterator it = m_category_info_map.find(category);
    if ( m_category_info_map.end() == it ) {
        PR_TRACE("category=%s not created yet, to create", category.c_str());
        on_total_t tmp; tmp.on = 0; tmp.total = 0;
        CategoryInfoMapPair pair;
        pair = m_category_info_map.insert(std::make_pair(category, tmp));
        it = pair.first;
    }

    switch (action) {
    case CATEGORY_INFO_ACTION_PLUS_TOTAL:
        it->second.total += (num);
        break;
    case CATEGORY_INFO_ACTION_MINUS_TOTAL:
        it->second.total -= (num);
        break;
    case CATEGORY_INFO_ACTION_PLUS_ON:
        it->second.on += (num);
        break;
    case CATEGORY_INFO_ACTION_MINUS_ON:
        it->second.on -= (num);
        break;
    default:
        PR_WARN("not supported action=%d", (int)action);
        return false;
    }
    if ( it->second.on > it->second.total
         || it->second.on < 0
         || it->second.total < 0) {
        PR_ERROR("invalid on=%d, total=%s", it->second.on, it->second.total);
        return false;
    }
    return true;
}

bool RainBowGuiDataManager::getCategoryInfo(const std::string& category,
                                            int& on, int& total)
{
    CategoryInfoMapIterator it = m_category_info_map.find(category);
    if ( m_category_info_map.end() == it ) {
        PR_TRACE("failed to find category info by category=%s");
        return false;
    }
    on = it->second.on;
    total = it->second.total;
    return true;
}

void RainBowGuiDataManager::getCategories(std::vector<std::string>& category_vec)
{
    CategoryInfoMapIterator it = m_category_info_map.begin();
    for ( ; it != m_category_info_map.end();  ++it ) {
        category_vec.push_back(it->first);
    }
}
