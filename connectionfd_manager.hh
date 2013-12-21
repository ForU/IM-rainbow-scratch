#ifndef INCLUDE_CONNECTIONFD_MANAGER_HPP
#define INCLUDE_CONNECTIONFD_MANAGER_HPP

// #include "user_data.hpp"
#include "trie_tree.hpp"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <vector>

// TODO: [2013-11-12] 
class ConnectionFdManager
{
public:
    ConnectionFdManager();
    virtual ~ConnectionFdManager();

private:
    TrieTree<TcpConnectionPtr> m_connections;
};


#endif /* INCLUDE_CONNECTIONFD_MANAGER_HPP */
