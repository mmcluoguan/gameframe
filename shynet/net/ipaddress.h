#ifndef SHYNET_NET_IPADDRESS_H
#define SHYNET_NET_IPADDRESS_H

#include <arpa/inet.h>
#include <string>

namespace shynet {
namespace net {
    /// <summary>
    /// ip地址
    /// </summary>
    class IPAddress final {
    public:
        IPAddress();
        explicit IPAddress(unsigned short family, unsigned short port);
        explicit IPAddress(const char* ip, unsigned short port);
        explicit IPAddress(const struct sockaddr_storage* storage);
        ~IPAddress();

        const std::string ip() const { return ip_; }
        unsigned short port() const { return port_; }
        unsigned short family() const { return family_; }
        const sockaddr_storage* sockaddr() const { return &addrs_; }

    private:
        struct sockaddr_storage addrs_;
        unsigned short family_ = 0;
        std::string ip_;
        unsigned short port_ = 0;
    };
}
}

#endif