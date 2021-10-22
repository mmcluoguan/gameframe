#include "shynet/net/acceptnewfd.h"

namespace shynet {
namespace net {

    AcceptNewFd::AcceptNewFd(std::shared_ptr<IPAddress> remoteAddr,
        std::shared_ptr<IPAddress> listenAddr,
        std::shared_ptr<events::EventBuffer> iobuf,
        FilterProces::ProtoType pt,
        bool enableHeart, ssize_t heartSecond)
        : FilterProces(iobuf, pt, FilterProces::Identity::ACCEPTOR)
    {
        remote_addr_ = remoteAddr;
        listen_addr_ = listenAddr;
        enable_heart_ = enableHeart;
        heart_second_ = heartSecond;
    }
    AcceptNewFd::~AcceptNewFd()
    {
    }

    int AcceptNewFd::input()
    {
        return process();
    }

    std::shared_ptr<IPAddress> AcceptNewFd::remote_addr() const
    {
        return remote_addr_;
    }

    std::shared_ptr<IPAddress> AcceptNewFd::listen_addr() const
    {
        return listen_addr_;
    }

    bool AcceptNewFd::enableHeart() const
    {
        return enable_heart_;
    }

    ssize_t AcceptNewFd::heart_second() const
    {
        return heart_second_;
    }

    std::weak_ptr<AcceptHeartbeat> AcceptNewFd::heart() const
    {
        return heart_;
    }

    void AcceptNewFd::set_heart(std::weak_ptr<AcceptHeartbeat> heart)
    {
        heart_ = heart;
    }

}
}
