#ifndef SHYNET_NET_IPADDRESS_H
#define SHYNET_NET_IPADDRESS_H

#include "shynet/utils/hash.h"
#include <arpa/inet.h>
#include <string>

namespace shynet {
namespace net {
    /**
     * @brief ip地址
    */
    class IPAddress final {
    public:
        /**
         * @brief 构造
        */
        IPAddress();
        /**
         * @brief 构造
         * @param family 地址族 
         * @param port 端口
        */
        explicit IPAddress(unsigned short family, unsigned short port);
        /**
         * @brief 构造
         * @param ip 地址字符串
         * @param port 端口
        */
        explicit IPAddress(const char* ip, unsigned short port);
        /**
         * @brief 构造
         * @param storage ipv4,ipv6结构 
        */
        explicit IPAddress(const struct sockaddr_storage* storage);
        ~IPAddress() = default;

        /**
         * @brief 获取地址字符串
         * @return 地址字符串
        */
        const std::string ip() const { return ip_; }
        /**
         * @brief 获取端口
         * @return 端口
        */
        unsigned short port() const { return port_; }
        /**
         * @brief 获取地址族
         * @return 地址族
        */
        unsigned short family() const { return family_; }
        /**
         * @brief 获取ipv4,ipv6结构 
         * @return ipv4,ipv6结构 
        */
        sockaddr_storage* sockaddr() { return &addrs_; }
        /**
         * @brief 获取地址长度
         * @return 地址长度
        */
        socklen_t socketlen() const { return sizeof(addrs_); };

        bool operator==(const IPAddress& p) const
        {
            return p.ip_ == ip_ && p.port_ == port_;
        }

    private:
        /**
         * @brief ipv4,ipv6结构 
        */
        struct sockaddr_storage addrs_;
        /**
         * @brief 地址族
        */
        unsigned short family_ = 0;
        /**
         * @brief 地址字符串
        */
        std::string ip_;
        /**
         * @brief 端口
        */
        unsigned short port_ = 0;
    };

    struct IPAddressHash {
    public:
        size_t operator()(const IPAddress& p) const
        {
            return shynet::utils::hash_val(p.ip(), p.port());
        }
    };
}
}

#endif
