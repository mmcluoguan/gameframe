#ifndef SHYNET_UTILS_CONSISTENT_HASH_H
#define SHYNET_UTILS_CONSISTENT_HASH_H

#include <cstdint>
#include <map>
#include <optional>
#include <sstream>
#include <string>

namespace shynet {
namespace utils {
    namespace {
        /**
         * @brief // 32 位的 Fowler-Noll-Vo 哈希算法 https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
         * @param key 
         * @return 
        */
        inline uint32_t fnv_hash(std::string key)
        {
            const int p = 16777619;
            uint32_t hash = 2166136261;
            for (size_t idx = 0; idx < key.size(); ++idx) {
                hash = (hash ^ key[idx]) * p;
            }
            hash += hash << 13;
            hash ^= hash >> 7;
            hash += hash << 3;
            hash ^= hash >> 17;
            hash += hash << 5;
            if (hash < 0) {
                hash = -hash;
            }
            return hash;
        }
    }

    /**
     * @brief 一致性hash
     * @tparam V 节点类型
    */
    template <typename V>
    class consistent_hash {
    public:
        /**
         * @brief 构造
         * @param virtual_node_num 虚拟节点数量
        */
        consistent_hash(uint32_t virtual_node_num = 100)
        {
            virtual_node_num_ = virtual_node_num;
        }

        /**
         * @brief 添加实际节点
         * @param node 实际节点
        */
        void addnode(V node)
        {
            for (uint32_t i = 0; i < virtual_node_num_; i++) {
                std::stringstream ss;
                ss << node << "#" << i;
                uint32_t partition = fnv_hash(ss.str());
                server_node_.insert({ partition, node });
            }
        }

        /**
         * @brief 删除实际节点
         * @param node 实际节点
        */
        void delnode(V node)
        {
            for (uint32_t i = 0; i < virtual_node_num_; i++) {
                std::stringstream ss;
                ss << node << "#" << i;
                uint32_t partition = fnv_hash(ss.str());
                auto it = server_node_.find(partition);
                if (it != server_node_.end()) {
                    server_node_.erase(it);
                }
            }
        }

        /**
         * @brief 返回指定数据分配的实际节点
         * @tparam K 数据类型
         * @param v 数据值
         * @return 实际节点
        */
        template <typename K>
        std::optional<V> selectnode(const K& k)
        {
            std::stringstream ss;
            ss << k;
            uint32_t partition = fnv_hash(ss.str());
            auto it = server_node_.lower_bound(partition);
            if (it == server_node_.end()) {
                if (server_node_.empty()) {
                    return std::nullopt;
                } else {
                    return std::make_optional<V>(server_node_.begin()->second);
                }
            }
            return std::make_optional<V>(it->second);
        }

    private:
        /**
         * @brief hash值与实际节点的映射
        */
        std::map<uint32_t, V> server_node_;
        /**
         * @brief 虚拟节点数量
        */
        uint32_t virtual_node_num_ = 0;
    };
}
}

#endif
