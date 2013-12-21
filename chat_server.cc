
#include "login_server.hh"

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();

    if (argc == 1) {
        printf("Usage: %s port\n", argv[0]);
        return -1;
    }

    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress serverAddr(port);
    LoginServer server(&loop, serverAddr);

    server.start();
    loop.loop();

    return 0;
}

#include "trie_tree.hh"
#include "user_data.hh"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

#include <set>
#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer : boost::noncopyable
{
public:
    ChatServer(EventLoop* loop,
               const InetAddress& listenAddr)
        : m_server(loop, listenAddr, "ChatServer"),
          m_codec(boost::bind(&ChatServer::onStringMessage, this, _1, _2, _3))
        {
            m_server.setConnectionCallback(
                boost::bind(&ChatServer::onConnection, this, _1));
            m_server.setMessageCallback(
                boost::bind(&LengthHeaderCodec::onMessage, &m_codec, _1, _2, _3));
        }

    void start()
        {
            m_server.start();
        }

private:
    void onConnection(const TcpConnectionPtr& conn)
        {
            LOG_INFO << conn->localAddress().toIpPort() << " -> "
                     << conn->peerAddress().toIpPort() << " is "
                     << (conn->connected() ? "UP" : "DOWN");

            if (conn->connected()) {
                // connections_.insert(conn);
            }
            else {
                // connections_.erase(conn);
            }
        }

    void onStringMessage(const TcpConnectionPtr&,
                         const string& message,
                         Timestamp)
        {
            // TODO: [2013-11-08] 

            // for (ConnectionList::iterator it = connections_.begin();
            //      it != connections_.end();
            //      ++it)
            // {
            //     // m_codec.send(get_pointer(*it), message);
            // }
        }

    // typedef std::set<TcpConnectionPtr> ConnectionList;
    TcpServer m_server;
    LengthHeaderCodec m_codec;
    // ConnectionList connections_;
    TrieTree<UserRefer> m_users;
};

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 1)
    {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress serverAddr(port);
        ChatServer server(&loop, serverAddr);
        server.start();
        loop.loop();
    }
    else
    {
        printf("Usage: %s port\n", argv[0]);
    }
}

