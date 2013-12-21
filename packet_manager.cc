#include <string.h>
#include <muduo/extra/Json.h>

#include "packet_manager.hh"
#include "user_manager.hh"
#include "session_manager.hh"
#include "time_stamp.hh"

using namespace muduo;
using namespace muduo::net;

////////////////////////////////////////////////////////////////
TcpConnectionPtr Packet::s_connection;
Packet::Packet(const TcpConnectionPtr& self_conn)
    : m_self_connection(self_conn)
{
    m_raw_size = 0; m_raw_data = "";
    m_message_type = E_MESSAGE_TYPE_NONE;
    m_message_body = "";
    m_pac_protocol_valid = false;
    m_priority = PACKET_PRIORITY_NORMAL;
}

Packet::Packet(const std::string& message_body, e_message_type message_type, const TcpConnectionPtr& self_conn):
    m_message_type(message_type),
    m_message_body(message_body),
    m_self_connection(self_conn)
{
    m_pac_protocol_valid= false;
    m_pac_message_valid = false;

    composePacketMessage(); // get m_raw_data and m_raw_size
    encapsulate();
}

Packet::Packet(const std::string& message_str,
               const muduo::net::TcpConnectionPtr& self_conn)
    : m_raw_size(message_str.size()),
      m_raw_data(message_str),
      m_self_connection(self_conn)
{
    m_message_type = E_MESSAGE_TYPE_NONE;
    m_message_body = "";
    m_pac_protocol_valid = false;
    m_pac_message_valid = false;
    m_priority = PACKET_PRIORITY_NORMAL;

    if (! parse(message_str)) {
        LOG_WARN << "failed to parse packet from message_str,"
                 << " message_str:\"" << message_str << "\"";
    }
}

void Packet::sendToSelf()
{
    if ( ! isSelfConnExists() ) {
        LOG_WARN << "packet's self connection is down";
        return;
    }
    LOG_INFO << "sending message to "
             << m_self_connection->peerAddress().toIpPort()
             << ", raw data=[" << m_raw_data << "]";

    Buffer buf(m_buf);
    m_self_connection->send(&buf);
}

void Packet::sendTo(const TcpConnectionPtr& conn)
{
    if ( 0 == conn.use_count() ) {
        LOG_WARN << "connection is reset, cant send message";
        return;
    }
    LOG_INFO << "sending message to "
             << conn->peerAddress().toIpPort()
             << ", raw data=[" << m_raw_data << "]";
    // NOTICE: because the lib will erase m_buf, so we send a copy
    // of m_buf to avoid the packet to be changed for multiple use
    Buffer buf(m_buf);
    conn->send(&buf);
}

void Packet::composeErrorResponse() {
    m_message_type = E_MESSAGE_TYPE_FATAL;
    Json json;
    std::vector<DStr> body_vec;
    JSON_VEC_PUSH_STR(json, body_vec, "fatal_info", "invalid_protocol");

    m_message_body = json.SetObject(body_vec, NULL).Str();
    LOG_TRACE << "message body value=[" << m_message_body << "]";
    composePacketMessage();
    encapsulate();
}

bool Packet::extract(Buffer* buf) {
    LOG_TRACE << "extracting packet buffer";
    while ( buf->readableBytes() >= m_head_size ) {
        const void* tmp_data = buf->peek();
        int32_t be32 = *static_cast<const int32_t*>(tmp_data); // SIGBUS
        const int32_t len = sockets::networkToHost32(be32);
        if (len > 65536 || len < 0) {
            LOG_WARN << "Invalid length " << len;
            return false;
        }
        if (buf->readableBytes() >= (unsigned)(len + m_head_size)) {
            buf->retrieve(m_head_size);
            std::string tmp(buf->peek(), len);
            m_raw_data = tmp;
            buf->retrieve(len);
        }
        else {
            break;
        }
    }
    LOG_DEBUG << "after extracting, raw data:\"" << m_raw_data <<"\"";
    return parse(m_raw_data);
}

bool Packet::isProtocolValid() const {
    return m_pac_protocol_valid;
}

const std::string& Packet::rawData() const {
    return (const std::string&)m_raw_data;
}

e_message_type Packet::messageType() const {
    return m_message_type;
}

const std::string& Packet::messageBody() const {
    return m_message_body;
}

bool Packet::parse(const std::string& message_str) {
    // set data size
    m_raw_size = message_str.size();
    if (m_raw_size == 0) {
        LOG_WARN << "empty message_str";
        return false;
    }
    // message type
    LOG_DEBUG << "getting type from message_str";
    Json json(m_raw_data.c_str());

    DStr tmp = json.GetValue("type", NULL);
    if ( NULL == tmp.Str() ) {
        LOG_WARN << "no value named \"type\" in [" << messageBody() << "]";
        return false;
    }
    m_message_type = (e_message_type)atoi(tmp.Str());
    LOG_INFO << "type:\"" << messageType() <<"\"";

    // message body
    tmp.Empty();
    tmp = json.GetObject("body", NULL);
    if ( NULL == tmp.Str() ) {
        LOG_WARN << "no object named \"body\" in [" << messageBody() << "]";
        return false;
    }
    m_message_body = tmp.Str();
    LOG_INFO << "message body:\"" << messageBody() <<"\"";
    // protocol is ok
    m_pac_protocol_valid = true;
    return m_pac_protocol_valid;
}

bool Packet::composePacketMessage() {
    Json json;
    std::vector<DStr> message;
    DStr type = json.SetIntValue(PACKET_MESSAGE_TYPE_KEY, m_message_type);
    if ( NULL == type.Str() ) {
        LOG_ERROR << "failed to compose message type";
        return false;
    }
    DStr body = json.AddObjectKey(PACKET_MESSAGE_BODY_KEY, m_message_body.c_str());
    if ( NULL == body.Str() ) {
        LOG_ERROR << "failed to compose message body";
        return false;
    }
    message.push_back(type);
    message.push_back(body);

    m_raw_data = json.SetObject(message, NULL).Str();
    m_raw_size = m_raw_data.size();

    LOG_TRACE << "packet=["<< m_raw_data << "]";
    return true;
}

// condition: m_raw_data and m_raw_size is prepared
void Packet::encapsulate() {
    int32_t be32 = sockets::hostToNetwork32(m_raw_size);
    m_buf.prepend(&be32, sizeof be32); // head
    m_buf.append(m_raw_data.c_str(), m_raw_data.size()); // message body
}


////////////////////////////////////////////////////////////////
static bool isPacketPriorityValid(packet_priority priority)
{
    return (priority < PACKET_PRIORITY_UPPER_BOUND
            && priority >= PACKET_PRIORITY_LOW);
}

PacketManager::PacketManager()
{
    LOG_TRACE << "constructing packet manager";
}

void PacketManager::extractPacket(const TcpConnectionPtr& self_conn,
                                  Buffer* buf,
                                  Timestamp receiveTime)
{
    Packet packet(self_conn);
    LOG_DEBUG << "to extract coming packet ...";
    if ( ! packet.extract(buf) ) {
        packet.composeErrorResponse();
        packet.sendTo(self_conn);
        self_conn->shutdown();  // FIXME: disable reading
    }
    G_packet_manager.addPacket(packet);
}

PacketList& PacketManager::getPacketList(packet_priority priority)
{
    // NOTICE: make sure priority is valid before calling this fun
    switch (priority) {
    case PACKET_PRIORITY_LOW: return m_p_low_packets;
    case PACKET_PRIORITY_NORMAL: return m_p_normal_packets;
    case PACKET_PRIORITY_HIGH: return m_p_high_packets;
    case PACKET_PRIORITY_EMERGENCY: return m_p_emergency_packets;
    default: return m_p_normal_packets;
    }
    return m_p_normal_packets;
}

bool PacketManager::addPacket(const Packet& packet)
{
    // TODO: [2013-12-08] signal
    LOG_TRACE << "adding packet into packet manager";
    packet_priority priority = packet.getPriority();
    if ( ! isPacketPriorityValid(priority) ) {
        LOG_ERROR << "invalid packet priority=" << priority;
        return false;
    }
    PacketList& list = getPacketList(priority);
    list.addPacket(packet);
    return true;
}

void PacketManager::getPacket(Packet& rst_packet, packet_priority priority)
{
    // TODO: [2013-12-08] signal
    LOG_TRACE << "getting packet from packet manager, priority="
              << (int)priority;
    PacketList& list = getPacketList(priority);
    list.getPacket(rst_packet);
}

void PacketManager::process()
{
    while ( 1 ) {
        Packet packet;
        getPacket(packet, packet.getPriority());

        const std::string& message_body =  packet.messageBody();
        e_message_type message_type = packet.messageType();
        LOG_DEBUG << "coming message type=" << message_type
                  << ", data=" << packet.messageBody();

        // core ////////////////
        switch (message_type) {
        case E_MESSAGE_TYPE_NONE: {
            LOG_DEBUG << "message type: none";
            break;
        }
        case E_MESSAGE_TYPE_LOGIN_REQUEST: {
            LoginRequest msg(message_body);
            msg.process(packet.getSelfConnection());
            break;
        }
        // TODO: [2013-11-25] fix (e_message_type)1
        case(e_message_type)1: {
            RegisterRequest msg(message_body);
            msg.process(packet.getSelfConnection());
            break;
        }
        case E_MESSAGE_TYPE_LOGIN_RESPONSE: {
            break;
        }
        case E_MESSAGE_TYPE_NOTIFICATION_LOGIN_STATUS: {
            // for login on-line is handle by login request, and
            // for server, the login status notification only
            // process "off-line" type notification
            LoginStatusNotification msg(message_body);
            msg.process(packet.getSelfConnection());
            break;
        }
        case E_MESSAGE_TYPE_CHAT: {
            ChatMessage msg(message_body);
            msg.process(packet.getSelfConnection());
            break;
        }
        case E_MESSAGE_TYPE_FRIENDS_MANAGEMENT: {
            FriendsManagementMessage msg(message_body);
            msg.process(packet.getSelfConnection());
            break;
        }
        default: {
            LOG_ERROR << "unknown message";
            break;
        }
        } // switch
    } // while(1)
}
