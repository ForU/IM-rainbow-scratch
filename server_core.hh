#ifndef INCLUDE_LOGIN_SERVER_HPP
#define INCLUDE_LOGIN_SERVER_HPP

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/bind.hpp>

#include <set>
#include <stdio.h>

#include "user_manager.hh"
#include "packet_manager.hh"

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

class ServerCore : boost::noncopyable
{
public:
    ServerCore(muduo::net::EventLoop* loop,
               const muduo::net::InetAddress& listenAddr)
        : m_server(loop, listenAddr, "ServerCore") {}
    void init();
    void start();

private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);

private:
    muduo::net::TcpServer m_server;
};

#endif /* INCLUDE_LOGIN_SERVER_HPP */
