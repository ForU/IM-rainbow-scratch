#ifndef INCLUDE_RAINBOW_GUI_HPP
#define INCLUDE_RAINBOW_GUI_HPP

#include "gui.hh"

class SearchWidgetBar
{
public:
    SearchWidgetBar();
    ~SearchWidgetBar() {}

    GtkWidget* container() { return m_container; }
    // this is for prefer add menu
    void initShowForAddMenu();
    void showAll() { gtk_widget_show_all(m_container); }
    void hide() { gtk_widget_hide(m_container); }
    void hideLabelInfo() { gtk_widget_hide(m_label_user_info); }

    void hideAddButton() { gtk_widget_hide(m_button_add); }
    void hideDeleteButton() { gtk_widget_hide(m_button_delete); }

    void setNotification(const char* notification, e_notification_type type);
    void clearNotificationArea() { gtk_label_set_text(GTK_LABEL(m_label_user_info), "");}
    void composeAndSendRequest(const std::string& target_id_str, int action, const std::string& category="Friends");

    const std::string& getLatestEntryContent() const { return m_latest_entry_content; }
    void setLatestEntryContent(const std::string& entry_content) {
        m_latest_entry_content = entry_content;
    }

private:
    void compose();

private:
    std::string m_latest_entry_content;
    GtkWidget* m_container;
    GtkWidget* m_entry_search;
    GtkWidget* m_label_user_info;
    GtkWidget* m_button_add;
    GtkWidget* m_button_delete;
    GtkWidget* m_button_cancel;
};

class FriendsManagementResponse;

class RainbowGui : public Gui
{
public:
    RainbowGui();
    virtual ~RainbowGui() {}
    void compose();
    void show();
    // user/group bar
    void expandUserBar() { m_user_bar.ExpandTreeViewAll(); }
    void expandGroupBar() { m_group_bar.ExpandTreeViewAll(); }
    void hideUserBar();
    void hideGroupBar();
    void showUserBar();
    void showGroupBar();
    GtkWidget* getButtonToggleUser();
    GtkWidget* getButtonTogglegroup();
    GtkWidget* getButtonTogglePreference();
    GtkWidget* getUserBar();
    GtkWidget* getGroupBar();

    bool addBuddy(const BuddyData& buddy, BuddyData* data_pointer);
    bool addGroup(const GroupData& group, GroupData* data_pointer);

    bool updateBuddy(const char *user_id_str, bool online, bool signature_changed=false, const char* signature="");
    bool updateGroup(const char *group_id_str, bool signature_changed, const char* signature);
    void updateAccountTitle(const char* name, const char*signature);
    void updateCategoryInfoAll();
    void updateCategoryInfo(const char* category);

    void hideSearchWidgetBar() { m_search_widget.hide(); }
    void showSearchWidgetBar() { m_search_widget.initShowForAddMenu(); }

    SearchWidgetBar& getSearchWidget() { return m_search_widget; }
    const SearchWidgetBar& getSearchWidget() const { return m_search_widget; }

    void handleSearchResultInSearchBar(const FriendsManagementResponse& response);

private:
    UserInfoBar m_account_title;
    GtkWidget* m_header_bar;
    GtkWidget* m_toggle_button_user;
    GtkWidget* m_toggle_button_group;
    GtkWidget* m_toggle_button_preference;
    InfoListBar m_user_bar;
    InfoListBar m_group_bar;

    SearchWidgetBar m_search_widget;
    // TODO: [2013-12-11] to use mutex for update
};

#endif /* INCLUDE_RAINBOW_GUI_HPP */
