#include "server_core.hh"
#include "session_manager.hh"
#include "packet_manager.hh"

using namespace muduo;
using namespace muduo::net;

void ServerCore::init()
{
    LOG_TRACE << "initializing server core";
    m_server.setConnectionCallback( boost::bind(&ServerCore::onConnection, this, _1));
    m_server.setMessageCallback( boost::bind(&PacketManager::extractPacket, &G_packet_manager, _1, _2, _3));
}

void ServerCore::start()
{
    m_server.start();
}

void ServerCore::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    int sockfd;
    if ( conn->connected() ) {
        sockfd = conn->getSockfd();
        // add to session manager
        G_session_manager.addConnector_Guard(sockfd);
    }
    if ( ! conn->connected() ) {
        sockfd = conn->getSockfd();
        G_session_manager.handleDownConection_Guard(sockfd);
    }

}

