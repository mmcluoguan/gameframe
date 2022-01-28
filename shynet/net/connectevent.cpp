#include "shynet/net/connectevent.h"
#include "shynet/pool/threadpool.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/singleton.h"
#include "shynet/utils/stuff.h"

namespace shynet {
namespace net {

    ConnectEvent::ConnectEvent(std::shared_ptr<net::IPAddress> connect_addr,
        FilterProces::ProtoType pt,
        __socket_type type,
        bool enable_ssl,
        bool enable_check, ssize_t check_second)
        : protocol::FilterProces(nullptr, pt, protocol::FilterProces::Identity::CONNECTOR)
    {
        connect_addr_ = connect_addr;
        type_ = type;
        enable_ssl_ = enable_ssl;
        check_second_ = check_second;
        enable_check_ = enable_check;
        if (enable_ssl_ && type_ == SOCK_STREAM) {
            ctx_ = SSL_CTX_new(SSLv23_client_method());
            if (ctx_ == nullptr) {
                THROW_EXCEPTION("call SSL_CTX_new");
            }
        }
    }

    ConnectEvent::ConnectEvent(const char* hostname, short port,
        FilterProces::ProtoType pt,
        __socket_type type,
        bool enable_ssl, bool enable_check, ssize_t check_second)
        : protocol::FilterProces(nullptr, pt, protocol::FilterProces::Identity::CONNECTOR)
    {
        hostname_ = hostname;
        dnsport_ = port;
        type_ = type;
        enable_ssl_ = enable_ssl;
        enable_check_ = enable_check;
        check_second_ = check_second;
        struct hostent* phe = gethostbyname(hostname);
        if (phe == 0 || phe->h_addr_list[0] == 0) {
            THROW_EXCEPTION("call gethostbyname");
        }
        struct in_addr addr;
        memset(&addr, 0, sizeof(in_addr));
        memcpy(&addr, phe->h_addr_list[0], sizeof(struct in_addr));
        char ip[65] = { 0 };
        strcpy(ip, inet_ntoa(addr));
        connect_addr_ = std::make_shared<IPAddress>(ip, port);
        if (enable_ssl_ && type_ == SOCK_STREAM) {
            ctx_ = SSL_CTX_new(SSLv23_client_method());
            if (ctx_ == nullptr) {
                THROW_EXCEPTION("call SSL_CTX_new");
            }
        }
    }

    ConnectEvent::~ConnectEvent()
    {
        if (dnsbase_ != nullptr) {
            evdns_base_free(dnsbase_, 0);
        }
        if (ctx_ != nullptr)
            SSL_CTX_free(ctx_);
    }

    int ConnectEvent::connectid() const
    {
        return conectid_;
    }

    void ConnectEvent::set_connectid(int id)
    {
        conectid_ = id;
    }

    std::shared_ptr<net::IPAddress> ConnectEvent::connect_addr() const
    {
        return connect_addr_;
    }

    bool ConnectEvent::enable_ssl() const
    {
        return enable_ssl_;
    }

    SSL_CTX* ConnectEvent::ctx() const
    {
        return ctx_;
    }

    void ConnectEvent::success()
    {
        if (pt() == ProtoType::WEBSOCKET) {
            request_handshake();
        } else
            complete();
    }

    void ConnectEvent::input(int fd)
    {
        if (type_ == SOCK_DGRAM) {
            char buffer[MAXIMUM_MTU_SIZE];
            ssize_t ret = udpsocket_->recvfrom(buffer, MAXIMUM_MTU_SIZE);
            if (ret > 0) {
                //std::cout << " ConnectEvent::input guid:" << udpsocket_->guid()
                //          << " ret:" << ret << std::endl;
                if (udpsocket_->guid() == static_cast<uint32_t>(-1)) {
                    //处理不可靠消息
                    if (buffer[0] == (char)protocol::UdpMessageDefine::ID_ATTEMPT_CONNECT_ACK
                        && udpsocket_->st == protocol::UdpSocket::CONNECTING) {
                        uint32_t guid;
                        memcpy(&guid, buffer + sizeof(char), sizeof(guid));
                        guid = ntohl(guid);
                        //std::cout << "ID_ATTEMPT_CONNECT_ACK:" << guid << std::endl;
                        //建立可靠的udp连接层
                        udpsocket_->set_guid(guid);
                        udpsocket_->st = protocol::UdpSocket::CONNECTSUCCESS;
                        udpsocket_->init_pair_buffer(base());
                        set_iobuf(udpsocket_->output_buffer);
                        auto udpth = utils::Singleton<pool::ThreadPool>::instance().udpTh().lock();
                        if (udpth) {
                            udpth->add_connect_udp(guid, udpsocket_);
                        }
                        //发送第3次握手
                        buffer[0] = (char)protocol::UdpMessageDefine::ID_CONNECT;
                        udpsocket_->sendto(buffer, MAXIMUM_MTU_SIZE);
                        //LOG_DEBUG << "CONNECTING:" << guid;
                        success();
                    }
                } else {
                    if (buffer[0] == (char)protocol::UdpMessageDefine::ID_CLOSE) {
                        uint32_t guid;
                        memcpy(&guid, buffer + sizeof(char), sizeof(guid));
                        guid = htonl(guid);
                        if (udpsocket_->guid() == guid) {
                            auto shclient = udpsocket_->cnev.lock();
                            if (shclient) {
                                shclient->close(net::CloseType::SERVER_CLOSE);
                            }
                        }
                    } else {
                        //处理可靠消息
                        udpsocket_->recv(buffer, ret);
                    }
                }
            } else {
                if (errno != EAGAIN) {
                    std::error_condition econd = std::system_category().default_error_condition(errno);
                    LOG_WARN << "[" << econd.category().name()
                             << "(" << econd.value() << ")"
                             << " " << econd.message() << "] ";
                }
            }
        }
    }

    InputResult ConnectEvent::input()
    {
        return process();
    }

    bool ConnectEvent::enable_check() const
    {
        return enable_check_;
    }
    int ConnectEvent::check_timeid() const
    {
        return check_timeid_;
    }

    void ConnectEvent::set_check_timeid(int ht)
    {
        check_timeid_ = ht;
    }

    ssize_t ConnectEvent::check_second() const
    {
        return check_second_;
    }

    uint32_t ConnectEvent::fd() const
    {
        auto ptr = iobuf();
        if (ptr != nullptr)
            return ptr->fd();
        return -1;
    }
}
}
