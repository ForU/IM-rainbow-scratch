#ifndef INCLUDE_REGISTER_CLIENT_HPP
#define INCLUDE_REGISTER_CLIENT_HPP

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <iostream>
#include <stdio.h>
#include <string>

class RegisterClient : boost::noncopyable
{
public:
    RegisterClient(muduo::net::EventLoop* loop, const muduo::net::InetAddress& serverAddr);
    ~RegisterClient() {}

    void connect();
    void disconnect();
    void write(const std::string& message);
    muduo::net::TcpConnectionPtr& connection() ;
    void onRegisterResponse(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp receiveTime);

private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);

    muduo::net::TcpClient m_client;
    muduo::MutexLock m_mutex;
    muduo::net::TcpConnectionPtr m_connection;
};

#endif /* INCLUDE_REGISTER_CLIENT_HPP */
