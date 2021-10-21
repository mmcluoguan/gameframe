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

        const std::string ip() const;
        unsigned short port() const;
        unsigned short family() const;
        const sockaddr_storage* sockaddr() const;

    private:
        struct sockaddr_storage addrs_;
        unsigned short family_ = 0;
        std::string ip_;
        unsigned short port_ = 0;
    };
}
}

#endif
