#include "global.hh"
#include "log.hh"

// users only has one
#define USERS_ICON "/home/lk/prog/muduo/muduo/examples/hi/cli/icons/users-24.png"

// user
#define USER_ICON_INACTIVE_MIDDLE  "/home/lk/prog/muduo/muduo/examples/hi/cli/icons/user-24.png"
#define USER_ICON_ACTIVE_MIDDLE  "/home/lk/prog/muduo/muduo/examples/hi/cli/icons/active-user-24.png"
#define USER_ICON_INACTIVE_LARGE   "/home/lk/prog/muduo/muduo/examples/hi/cli/icons/user-32.png"
#define USER_ICON_ACTIVE_LARGE   "/home/lk/prog/muduo/muduo/examples/hi/cli/icons/active-user-32.png"
// group
#define GROUP_ICON "/home/lk/prog/muduo/muduo/examples/hi/cli/icons/group-24.png"

// preference
#define PREFERENCE_ICON "/home/lk/prog/muduo/muduo/examples/hi/cli/icons/settings-24.png"


// preference
#define SEARCH_ICON "/home/lk/prog/muduo/muduo/examples/hi/cli/icons/search-24.png"

// password
#define KEY_ICON "/home/lk/prog/muduo/muduo/examples/hi/cli/icons/key-24.png"

#define DEFAULT_ICON USER_ICON_INACTIVE_MIDDLE

static const icon_info_t icon_infos[] = {
    { ICON_TYPE_LOWER_BOUND,  "" },
    { ICON_TYPE_USER_ACTIVE_MIDDLE, USER_ICON_ACTIVE_MIDDLE },
    { ICON_TYPE_USER_INACTIVE_MIDDLE, USER_ICON_INACTIVE_MIDDLE },
    { ICON_TYPE_USER_ACTIVE_LARGE, USER_ICON_ACTIVE_LARGE },
    { ICON_TYPE_USER_INACTIVE_LARGE, USER_ICON_INACTIVE_LARGE },
    { ICON_TYPE_USERS, USERS_ICON },
    { ICON_TYPE_GROUP, GROUP_ICON },
    { ICON_TYPE_PREFERENCE, PREFERENCE_ICON },
    { ICON_TYPE_SEARCH, SEARCH_ICON },
    { ICON_TYPE_PASSWORD, KEY_ICON },
    // to add
};

bool iconTypeValid(icon_type type)
{
    return (type < ICON_TYPE_UPPER_BOUND && type > ICON_TYPE_LOWER_BOUND);
}

const char* getIconPath(icon_type type)
{
    if ( ! iconTypeValid(type) ) {
        return DEFAULT_ICON;
    }
    return icon_infos[type].path;
}

const char* getIconDefaultPath()
{
    return DEFAULT_ICON;
}


msg_showing_info_t msg_showing_infos[] = {
    { MSG_ST_NORMAL, "normal"},
    { MSG_ST_ECHO_HEAD, "echo_head"},
    { MSG_ST_ECHO_CONTENT, "echo_content"},
    { MSG_ST_NON_ECHO_HEAD, "non_echo_head"},
    { MSG_ST_NON_ECHO_CONTENT, "non_echo_content"},
    { MSG_ST_NOTIFICATION, "notification"},
    { MSG_ST_WARNING, "warning"},
    { MSG_ST_ERROR, "error"}
};

void createTextViewTags(GtkTextBuffer* buffer)
{
    gtk_text_buffer_create_tag(buffer, "normal",
                               "style", PANGO_STYLE_NORMAL,
                               "justification", GTK_JUSTIFY_LEFT,
                               "foreground", "#2D3F47",
                               NULL);
    // echo
    gtk_text_buffer_create_tag(buffer, "echo_head",
                               "style", PANGO_STYLE_NORMAL,
                               "weight", PANGO_WEIGHT_BOLD,
                               "justification", GTK_JUSTIFY_RIGHT,
                               "foreground", "#2D3F47",
                               NULL);
    gtk_text_buffer_create_tag(buffer, "echo_content",
                               "style", PANGO_STYLE_NORMAL,
                               "justification", GTK_JUSTIFY_RIGHT,
                               "foreground", "#47352D",
                               NULL);
    // non_echo
    gtk_text_buffer_create_tag(buffer, "non_echo_head",
                               "style", PANGO_STYLE_NORMAL,
                               "weight", PANGO_WEIGHT_BOLD,
                               "justification", GTK_JUSTIFY_LEFT,
                               "foreground", "#2D3F47",
                               NULL);
    gtk_text_buffer_create_tag(buffer, "non_echo_content",
                               "style", PANGO_STYLE_NORMAL,
                               "justification", GTK_JUSTIFY_LEFT,
                               "foreground", "#237654",
                               NULL);
    // notification
    gtk_text_buffer_create_tag(buffer, "notification",
                               "style", PANGO_STYLE_ITALIC,
                               "justification", GTK_JUSTIFY_CENTER,
                               "foreground", "#987654",
                               NULL);
    // warning
    gtk_text_buffer_create_tag(buffer, "warning",
                               "style", PANGO_STYLE_ITALIC,
                               "justification", GTK_JUSTIFY_CENTER,
                               "foreground", "#FFCD00",
                               NULL);
    // error
    gtk_text_buffer_create_tag(buffer, "error",
                               "style", PANGO_STYLE_ITALIC,
                               "justification", GTK_JUSTIFY_CENTER,
                               "foreground", "#FF0000",
                               NULL);
}

bool isMsgShowingTypeValid(msg_showing_type type)
{
    return (type < MSG_ST_UPPER_BOUND &&
            type >= MSG_ST_NORMAL);

}

const char* getTagName(msg_showing_type type)
{
    if ( ! isMsgShowingTypeValid(type) ) {
        PR_WARN("invalid msg showing type=%d, use \"normal\"",
                (int)type);
        type = MSG_ST_NORMAL;
        return msg_showing_infos[type].tag_name;;
    }
    return msg_showing_infos[type].tag_name;
}
