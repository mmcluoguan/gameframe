#include "shynet/net/IPAddress.h"
#include "shynet/utils/Logger.h"
#include <cstring>

namespace shynet {
namespace net {
    IPAddress::IPAddress() { }
    IPAddress::IPAddress(unsigned short family, unsigned short port)
    {
        family_ = family;
        port_ = port;
        memset(&addrs_, 0, sizeof(addrs_));
        if (family == AF_INET) {
            struct sockaddr_in* in4 = (struct sockaddr_in*)&addrs_;
            in4->sin_family = AF_INET;
            in4->sin_addr.s_addr = INADDR_ANY;
            in4->sin_port = htons(port);
        } else if (family == AF_INET6) {
            struct sockaddr_in6* in6 = (struct sockaddr_in6*)&addrs_;
            in6->sin6_family = AF_INET6;
            in6->sin6_addr = in6addr_any;
            in6->sin6_port = htons(port);
        } else {
            std::ostringstream err;
            err << "family_:" << family_ << " 无效";
            THROW_EXCEPTION(err.str());
        }
    }
    IPAddress::IPAddress(const char* ip, unsigned short port)
        : ip_(ip)
    {
        port_ = port;
        memset(&addrs_, 0, sizeof(addrs_));
        int ret = inet_pton(AF_INET, ip, &(*(struct sockaddr_in*)&addrs_).sin_addr);
        if (ret == 1) {
            struct sockaddr_in* in4 = (struct sockaddr_in*)&addrs_;
            in4->sin_family = AF_INET;
            in4->sin_port = htons(port);
            family_ = AF_INET;
        } else {
            ret = inet_pton(AF_INET6, ip, &(*(struct sockaddr_in6*)&addrs_).sin6_addr);
            if (ret == 1) {
                struct sockaddr_in6* in6 = (struct sockaddr_in6*)&addrs_;
                in6->sin6_family = AF_INET6;
                in6->sin6_addr = in6addr_any;
                in6->sin6_port = htons(port);
                family_ = AF_INET6;
            } else {
                std::ostringstream err;
                err << "ip=" << ip << " 无效";
                THROW_EXCEPTION(err.str());
            }
        }
    }
    IPAddress::IPAddress(const sockaddr_storage* storage)
    {
        family_ = storage->ss_family;
        memcpy(&addrs_, storage, sizeof(addrs_));

        if (family_ == AF_INET) {
            char ipbuff[INET_ADDRSTRLEN] = { 0 };
            struct sockaddr_in* in4 = (struct sockaddr_in*)&addrs_;
            if (inet_ntop(family_, &in4->sin_addr, ipbuff, INET_ADDRSTRLEN) == nullptr) {
                THROW_EXCEPTION("call inet_ntop");
            }
            ip_ = std::string(ipbuff);
            port_ = in4->sin_port;
        } else if (family_ == AF_INET6) {
            char ipbuff[INET6_ADDRSTRLEN] = { 0 };
            struct sockaddr_in6* in6 = (struct sockaddr_in6*)&addrs_;
            if (inet_ntop(family_, &in6->sin6_addr, ipbuff, INET6_ADDRSTRLEN) == nullptr) {
                THROW_EXCEPTION("call inet_ntop");
            }
            ip_ = std::string(ipbuff);
            port_ = in6->sin6_port;
        } else {
            std::ostringstream err;
            err << "family_:" << family_ << " 无效";
            THROW_EXCEPTION(err.str());
        }
    }
    const std::string IPAddress::ip() const { return ip_; }
    unsigned short IPAddress::port() const { return port_; }
    unsigned short IPAddress::family() const { return family_; }
    const sockaddr_storage* IPAddress::sockaddr() const { return &addrs_; }
    IPAddress::~IPAddress() { }
} // namespace net
} // namespace shynet
