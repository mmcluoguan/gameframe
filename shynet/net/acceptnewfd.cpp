#include "shynet/net/acceptnewfd.h"

namespace shynet {
namespace net {

    AcceptNewFd::AcceptNewFd(std::shared_ptr<IPAddress> remoteAddr,
        std::shared_ptr<IPAddress> listenAddr,
        std::shared_ptr<events::EventBuffer> iobuf,
        FilterProces::ProtoType pt,
        bool enable_check, ssize_t check_second)
        : FilterProces(iobuf, pt, FilterProces::Identity::ACCEPTOR)
    {
        remote_addr_ = remoteAddr;
        listen_addr_ = listenAddr;
        enable_check_ = enable_check;
        check_second_ = check_second;
    }

    InputResult AcceptNewFd::input()
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

    bool AcceptNewFd::enable_check() const
    {
        return enable_check_;
    }

    ssize_t AcceptNewFd::check_second() const
    {
        return check_second_;
    }

    int AcceptNewFd::check_timeid() const
    {
        return check_timeid_;
    }

    void AcceptNewFd::set_check_timeid(int heart)
    {
        check_timeid_ = heart;
    }

}
}
