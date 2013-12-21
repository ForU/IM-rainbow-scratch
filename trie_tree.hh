#ifndef INCLUDE_TRIE_TREE_HPP
#define INCLUDE_TRIE_TREE_HPP

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>

#include "global.hh"

template <typename T, int Dim,  int (*IndexFunc)(char)>
class TrieTree
{
public:
    TrieTree() {}
    ~TrieTree() {
        destory();
    }

    T* find(const char *identifier) {
        TrieNode *p_node = & m_root;
        const char *p = identifier;
        std::string valid_index_str;
        muduo::MutexLockGuard lock(m_lock);
        while ( p && *p ) {
            int idx = IndexFunc(*p);
            if ( idx == G_DEFAULT_INDEX ) {
                LOG_ERROR << "invalid index=" << idx;
                return NULL;
            }
            p_node = p_node->m_nodes[idx];
            if ( ! p_node ) {
                LOG_TRACE << "already found indexes=[" << valid_index_str << "]";
                LOG_WARN << "not found for index:[" << idx <<"], return";
                return NULL;
            }
            valid_index_str.append(p, 1);
            // LOG_TRACE << "node exists at index:[" << idx <<"]";
            ++p;
        }
        // LOG_TRACE << "\"" << identifier << "\"" << " founded";
        return & (p_node->m_data);
    }

    const T* find(const char *identifier) const {
        const TrieNode *p_node = & m_root;
        const char *p = identifier;
        std::string valid_index_str;
        muduo::MutexLockGuard lock(m_lock);
        while ( p && *p ) {
            int idx = IndexFunc(*p);
            if ( idx == G_DEFAULT_INDEX ) {
                LOG_ERROR << "invalid index=" << idx;
                return NULL;
            }
            p_node = p_node->m_nodes[idx];
            if ( ! p_node ) {
                LOG_TRACE << "already found indexes=[" << valid_index_str << "]";
                LOG_ERROR << "not found for index:[" << idx <<"]";
                return NULL;
            }
            // LOG_TRACE << "node exists at index:[" << idx <<"]";
            valid_index_str.append(p, 1);
            ++p;
        }
        // LOG_TRACE << "\"" << identifier << "\"" << " founded";
        return & (p_node->m_data);
    }

    bool insertOrUpdate(const char *identifier, const T& data) {
        TrieNode *p_node = & m_root;
        const char *p = identifier;
        int idx = -1;

        muduo::MutexLockGuard lock(m_lock);
        while ( p && *p ) {
            idx = IndexFunc(*p);
            if ( idx == G_DEFAULT_INDEX ) {
                LOG_ERROR << "invalid index=" << idx;
                return false;
            }
            // start from the second level, that is bellow the root
            if ( ! p_node->m_nodes[idx] ) {
                p_node->m_nodes[idx] = new TrieNode();
            }
            p_node = p_node->m_nodes[idx];
            if ( ! p_node) return false;
            // head on
            ++p;
        }
        // assignment constructor must be supplied by T,
        // if T is a user defined type. always update here
        p_node->m_data = data;
        LOG_DEBUG << "data address:" << &(p_node->m_data);

        return true;
    }

    bool insert(const char *identifier, const T& data) {
        TrieNode *p_node = & m_root;
        const char *p = identifier;
        int idx = -1;
        bool node_exists = true;

        muduo::MutexLockGuard lock(m_lock);
        while ( p && *p ) {
            idx = IndexFunc(*p);
            if ( idx == G_DEFAULT_INDEX ) {
                LOG_ERROR << "invalid index=" << idx;
                return false;
            }
            // start from the second level, that is bellow the root
            if ( ! p_node->m_nodes[idx] ) {
                p_node->m_nodes[idx] = new TrieNode();
                node_exists = false;
            }
            p_node = p_node->m_nodes[idx];
            if ( ! p_node) return false;
            // head on
            ++p;
        }
        // assignment constructor must be supplied by T,
        // if T is a user defined type.
        if ( node_exists ) {
            LOG_WARN << "node already exists, data will not inserted. "
                     << "Please use UPDATE  instead of INSERT if to update data";
            return true;
        }
        p_node->m_data = data;
        LOG_DEBUG << "data address:" << &(p_node->m_data);

        return true;
    }

    bool update(const char *identifier, const T& data) {
        TrieNode *p_node = & m_root;
        const char *p = identifier;
        int idx = -1;
        bool node_exists = true;

        muduo::MutexLockGuard lock(m_lock);
        while ( p && *p ) {
            idx = IndexFunc(*p);
            if ( idx == G_DEFAULT_INDEX ) {
                LOG_ERROR << "invalid index=" << idx;
                return false;
            }
            // start from the second level, that is bellow the root
            if ( ! p_node->m_nodes[idx] ) {
                p_node->m_nodes[idx] = new TrieNode();
                node_exists = false;
            }
            p_node = p_node->m_nodes[idx];
            if ( ! p_node) return false;
            // head on
            ++p;
        }
        // assignment constructor must be supplied by T,
        // if T is a user defined type.
        if ( ! node_exists ) {
            LOG_WARN << "node not exists, data will not be updated. "
                     << "Please use insert  before UPDATE.";
            return false;
        }
        p_node->m_data = data;
        LOG_DEBUG << "data address:" << &(p_node->m_data);
        return true;
    }

    bool exists(const char *identifier) {
        TrieNode *p_node = & m_root;
        const char *p = identifier;
        int idx = -1;

        muduo::MutexLockGuard lock(m_lock);
        while ( p && *p ) {
            idx = IndexFunc(*p);
            if ( idx == G_DEFAULT_INDEX ) {
                LOG_ERROR << "invalid index=" << idx;
                return NULL;
            }
            // LOG_TRACE << "id: " << idx << " current node: nodes[" << idx<< "]:"<< p_node->m_nodes[idx];
            p_node = p_node->m_nodes[idx];
            if ( ! p_node ) {
                return false;
            }
            ++p;
        }
        return true;
    }

    void destory() {
        _destory(&m_root);
    }

private:
    class TrieNode {
    public:
        TrieNode() {
            for ( int i = 0; i < Dim; ++i ) {
                m_nodes[i] = NULL;
            }
        }
        ~TrieNode() {}
        // public:
        TrieNode *m_nodes[Dim];
        T m_data;
    };

    mutable muduo::MutexLock m_lock;
    TrieNode m_root;

private:
    void _destory(TrieNode *p_node) {
        if ( !p_node )
            return;
        for ( int i = 0; i < Dim ;  ++i) {
            if ( p_node->m_nodes[i] ) {
                // LOG_TRACE << "to delete node: " << i;
                _destory(p_node->m_nodes[i]);
            }
        }
        // only delete the NEW-ed trie node, the root will destroy automatically
        // by OS system.
        if ( p_node != &m_root) {
            delete p_node;
            LOG_TRACE << "delete node done ";
        }
    }
};
#endif /* INCLUDE_TRIE_TREE_HPP */
