#ifndef INCLUDE_RAINBOW_CLIENT_HPP
#define INCLUDE_RAINBOW_CLIENT_HPP

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <iostream>
#include <stdio.h>
#include <string>

class RainbowClient : boost::noncopyable
{
public:
    RainbowClient(muduo::net::EventLoop* loop, const muduo::net::InetAddress& serverAddr);
    ~RainbowClient() {}

    void connect();
    void disconnect();
    void write(const std::string& message);
    muduo::net::TcpConnectionPtr& connection() ;
    void onRainbowResponse(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
    void resetConnection() { m_connection.reset(); }

private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);

    muduo::net::TcpClient m_client;
    muduo::MutexLock m_mutex;
    muduo::net::TcpConnectionPtr& m_connection;
    static muduo::net::TcpConnectionPtr s_tcp_connection;
};

#endif /* INCLUDE_RAINBOW_CLIENT_HPP */
