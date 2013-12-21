#ifndef INCLUDE_GLOBAL_HPP
#define INCLUDE_GLOBAL_HPP

#include <gtk/gtk.h>

#define global_app_name "rainbow"
#define G_DEFAULT_INDEX -1

enum icon_type
{
    ICON_TYPE_LOWER_BOUND,      // 0
    ICON_TYPE_USER_ACTIVE_MIDDLE,
    ICON_TYPE_USER_INACTIVE_MIDDLE,
    ICON_TYPE_USER_ACTIVE_LARGE,
    ICON_TYPE_USER_INACTIVE_LARGE,
    ICON_TYPE_USERS,            // users has one type
    ICON_TYPE_GROUP,
    ICON_TYPE_PREFERENCE,
    ICON_TYPE_SEARCH,
    ICON_TYPE_PASSWORD,
    // to add
    ICON_TYPE_UPPER_BOUND
};

bool iconTypeValid(icon_type type);
const char* getIconPath(icon_type type);
const char* getIconDefaultPath();


typedef struct {
    icon_type type;
    const char* path;
} icon_info_t;

enum msg_showing_type
{
    MSG_ST_NORMAL,
    MSG_ST_ECHO_HEAD,
    MSG_ST_ECHO_CONTENT,
    MSG_ST_NON_ECHO_HEAD,
    MSG_ST_NON_ECHO_CONTENT,
    MSG_ST_NOTIFICATION,
    MSG_ST_WARNING,
    MSG_ST_ERROR,
    MSG_ST_UPPER_BOUND
};

typedef struct {
    msg_showing_type type;
    const char* tag_name;
} msg_showing_info_t;

extern msg_showing_info_t msg_showing_infos[];

extern const char* getTagName(msg_showing_type type);
extern bool isMsgShowingTypeValid(msg_showing_type type);
extern void createTextViewTags(GtkTextBuffer* buffer);

#endif /* INCLUDE_GLOBAL_HPP */
