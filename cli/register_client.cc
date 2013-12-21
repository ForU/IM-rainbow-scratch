#include "../packet_manager.hh"
#include "register_global.hh"
#include "log.hh"

using namespace muduo;
using namespace muduo::net;

RegisterClient::RegisterClient(EventLoop* loop, const InetAddress& serverAddr)
    : m_client(loop, serverAddr, "RegisterClient")
{
    m_client.setConnectionCallback( boost::bind(&RegisterClient::onConnection, this, _1));
    m_client.setMessageCallback(boost::bind(&RegisterClient::onRegisterResponse, this, _1, _2, _3));
    // m_client.enableRetry();
}

void RegisterClient::connect()
{
    PR_TRACE("register client to connect to server");
    m_client.connect();

}
void RegisterClient::disconnect()
{
    m_client.disconnect();
}

TcpConnectionPtr& RegisterClient::connection()
{
    return m_connection;
}

void RegisterClient::onRegisterResponse(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    // maybe lock the register button here or in callback function.
    PR_TRACE("register response coming..., to handle it");
    Packet response_packet;
    if ( ! response_packet.extract(buf) ) {
        PR_ERROR("failed to extract response from server");
        // TODO: [2013-11-26] show response in gui
        return;
    }
    PR_TRACE("message type=[%d], body=[%s]", response_packet.messageType(), response_packet.messageBody().c_str());

    e_message_type msg_type = response_packet.messageType();
    if ( msg_type != E_MESSAGE_TYPE_REGISTER_RESPONSE ) {
        PR_ERROR("response type is not register response");
        // TODO: [2013-11-26] show response in gui
        return;
    }
    // ok, construct response data
    const std::string& msg_body = response_packet.messageBody();
    global_register_gui->processResponse(msg_body.c_str());

    // TODO: [2013-11-25]
    // user response_packet manager get buffer content
    // construct register_response data structure by response_packet message body
    // use [GLOBAL REGISTER GUI] to show the register_response
    // unlock register button
}

void RegisterClient::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    MutexLockGuard lock(m_mutex);
    if (conn->connected()) {
        m_connection = conn;
    } else {
        m_connection.reset();
    }
}
