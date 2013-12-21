#ifndef INCLUDE_CHAT_GUI_HPP
#define INCLUDE_CHAT_GUI_HPP

#include "gui.hh"

// class BuddyData;
// class GroupData;

class ChatMessage;

class ChatPair
{
public:
    ChatPair(const std::string& common_id="00000");
    ~ChatPair() {}
    const std::string& getCommonId() const { return m_common_id; }
    void append(const char* content, msg_showing_type type=MSG_ST_NORMAL);
    GtkWidget* container() { return m_container; }
    GtkWidget* textview() { return m_textview; }

private:
    void compose();

private:
    std::string m_common_id;
    GtkWidget* m_container;
    GtkWidget* m_textview;        // output
    // TODO: [2013-12-06] to use my version
    muduo::MutexLock m_mutex;
};

class ChatInputEntry
{
public:
    ChatInputEntry();
    ~ChatInputEntry() {}
    GtkWidget* container() { return m_container; }
    bool update(chat_type type, void* data);
    void getName(std::string& name) const;
    void getCommonIdStr(std::string& common_id) const;

private:
    void compose();
    GtkWidget* m_container;
    GtkWidget* m_entry_send;      // input
    GtkWidget* m_button_send;
    chat_type m_type;
    void *m_data;                 // BuddyData or GroupData
    // TODO: [2013-12-04] to mutex update
    // MutexLock m_mutex;
};

class RainbowChatGui : public Gui
{
public:
    RainbowChatGui();
    virtual ~RainbowChatGui() {}
    friend ChatPair;

    void setWindowTitle(const char* chat_user);
    void compose();
    // sidebar
    bool addBuddy(BuddyData* data);
    bool addGroup(GroupData* data);
    bool addSidebarItem(chat_type type,void* sidebar_item_data,
                        const char* sidebar_item_name,
                        const char* sidebar_common_id,
                        const char* icon_path);
    bool updateSidebarItem(const char *common_id, bool online);
    bool setSidebarRowSelected(const char *common_id);
    void setSidebarSelectionFunction(GtkTreeSelectionFunc func);
    // input
    void updateInputEntry(chat_type type, void* data);
    // output
    void updateChatMessage(const ChatMessage& chat_msg);
    void updateChatNormalMessage(const ChatMessage& chat_msg);
    void updateChatErrorMessage(const ChatMessage& chat_msg);
    // other stuff
    void send(const std::string& message);

private:
    ChatPair* newChatPair(const std::string& user_id);
    void setCallbackEntrySendActivate(callback_ptr callback, void* args);

private:
    InfoListBar m_side_bar;
    GtkWidget* m_io_container;
    std::string s_default_user_id;
    ChatInputEntry m_input_entry;
};


#endif /* INCLUDE_CHAT_GUI_HPP */
