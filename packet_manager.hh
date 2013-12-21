#ifndef INCLUDE_MESSAGE_MANAGER_HPP
#define INCLUDE_MESSAGE_MANAGER_HPP

#include <boost/scoped_ptr.hpp>
#include <muduo/extra/Json.h>
#include <muduo/base/Condition.h>

#include "mutex.hh"
#include "singleton.hh"
#include "user_data.hh"
#include "protocol.hh"

#include <list>

#define PACKET_MESSAGE_TYPE_KEY "type"
#define PACKET_MESSAGE_BODY_KEY "body"

enum packet_priority
{
    PACKET_PRIORITY_LOW,
    PACKET_PRIORITY_NORMAL,
    PACKET_PRIORITY_HIGH,
    PACKET_PRIORITY_EMERGENCY,
    PACKET_PRIORITY_UPPER_BOUND,
};

class Packet
{
public:
    Packet(const muduo::net::TcpConnectionPtr& ptr=s_connection);
    // create packet
    Packet(const std::string& message_body, e_message_type message_type, const muduo::net::TcpConnectionPtr& self_conn=s_connection);
    // parse packet
    Packet(const std::string& message_str, const muduo::net::TcpConnectionPtr& ptr=s_connection);
    ~Packet() {}

public:
    void sendTo(const muduo::net::TcpConnectionPtr& connection);
    void sendToSelf();

    void composeErrorResponse();
    bool extract(muduo::net::Buffer* buf);
    bool isProtocolValid() const;

    const std::string& rawData() const;
    e_message_type messageType() const;
    const std::string& messageBody() const;

    packet_priority getPriority() const { return m_priority; }
    muduo::net::TcpConnectionPtr& getSelfConnection() {
        return m_self_connection;
    }

private:
    bool parse(const std::string& message_str);
    bool composePacketMessage();
    void encapsulate();
    bool isSelfConnExists() { return (m_self_connection.use_count() > 0); }

private:
    int32_t m_raw_size;
    std::string m_raw_data;
    e_message_type m_message_type;
    std::string m_message_body;
    const static int m_head_size = sizeof(int32_t);
    bool m_pac_protocol_valid;
    bool m_pac_message_valid;// TODO: [2013-11-20]
    muduo::net::Buffer m_buf;
    packet_priority m_priority;
    muduo::net::TcpConnectionPtr m_self_connection;
    static muduo::net::TcpConnectionPtr s_connection;
};

class PacketList
{
public:
    PacketList() :m_has_packet(m_mutex){}
    ~PacketList() {}

    size_t size() const {
        PthreadMutexLock _lock(m_mutex);
        return m_packets.size();
    }

    bool empty() const {
        PthreadMutexLock _lock(m_mutex);
        return m_packets.empty();
    }

    void addPacket(const Packet& packet) {
        PthreadMutexLock _lock(m_mutex);
        m_packets.push_back(packet);
        // http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
        m_has_packet.notify();
    }

    void getPacket(Packet& rst_packet) {
        PthreadMutexLock _lock(m_mutex);
        while ( m_packets.empty() ) {
            LOG_TRACE << "list empty, waiting";
            m_has_packet.wait();
        }
        rst_packet = m_packets.front();
        m_packets.pop_front();
    }
private:
    std::list<Packet> m_packets;
    PthreadCondition m_has_packet;
    mutable PthreadMutex m_mutex;
};

#define G_packet_manager SingletonX<PacketManager>::instance()
class PacketManager
{
public:
    PacketManager();
    ~PacketManager() {}
    void extractPacket(const muduo::net::TcpConnectionPtr& conn,
                       muduo::net::Buffer* buf,
                       muduo::Timestamp receiveTime);
    bool addPacket(const Packet& packet);
    void getPacket(Packet& rst_packet, packet_priority priority=PACKET_PRIORITY_NORMAL);
    void process();

private:
    // no object can be created
    PacketList& getPacketList(packet_priority priority=PACKET_PRIORITY_NORMAL);
    // packet lists
    PacketList m_p_low_packets;
    PacketList m_p_normal_packets;
    PacketList m_p_high_packets;
    PacketList m_p_emergency_packets;
};

#endif /* INCLUDE_MESSAGE_MANAGER_HPP */
