#include "shynet/thread/acceptthread.h"
#include "shynet/net/acceptheartbeat.h"
#include "shynet/net/acceptiobuffer.h"
#include "shynet/pool/threadpool.h"
#include "shynet/protocol/udpsocket.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/stuff.h"
#include <atomic>

namespace shynet {

namespace thread {
    AcceptThread::AcceptThread(size_t index)
        : Thread(ThreadType::ACCEPT, index)
    {
    }

    static void pipeReadcb(struct bufferevent* bev, void* ptr)
    {
        try {
            AcceptThread* rtk = reinterpret_cast<AcceptThread*>(ptr);
            rtk->process(bev);
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    void AcceptThread::process(bufferevent* bev)
    {
        events::EventBuffer pbuf(bev);
        char buf[sizeof(uintptr_t)] = { 0 };
        do {
            size_t len = pbuf.read(&buf, sizeof(buf));
            if (len == 0) {
                break;
            } else if (len != sizeof(buf)) {
                LOG_WARN << "AcceptThread没有足够的数据";
            } else {
                uintptr_t* p = reinterpret_cast<uintptr_t*>(buf);
                net::ListenEvent* apnf = reinterpret_cast<net::ListenEvent*>(*p);
                if (apnf->type() == SOCK_STREAM) {
                    tcp_accept(apnf);
                } else {
                    udp_accept(apnf);
                }
            }
        } while (true);
    }

    void AcceptThread::tcp_accept(net::ListenEvent* apnf)
    {
        struct sockaddr_storage cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));
        socklen_t socklen = sizeof(cliaddr);

        int newfd = accept(apnf->listenfd(), (struct sockaddr*)&cliaddr, &socklen);
        if (newfd < 0) {
            return;
        }
        if (evutil_make_socket_nonblocking(apnf->listenfd()) < 0) {
            evutil_closesocket(newfd);
            THROW_EXCEPTION("call evutil_make_socket_nonblocking");
        }
        eventTot_++;
        std::shared_ptr<net::IPAddress> newfdAddr = std::make_shared<net::IPAddress>(&cliaddr);
        LOG_TRACE << "accept newfd ip:" << newfdAddr->ip() << " port:" << newfdAddr->port();
        std::shared_ptr<net::AcceptIoBuffer> iobuf;
        if (apnf->enable_ssl()) {
            iobuf = std::make_shared<net::AcceptIoBuffer>(
                base_, newfd, true, apnf->ctx());
        } else {
            iobuf = std::make_shared<net::AcceptIoBuffer>(
                base_, newfd, false);
        }
        std::shared_ptr<net::AcceptNewFd> apnewfd = apnf->accept_newfd(newfdAddr, iobuf).lock();
        iobuf->set_newfd(apnewfd);
        if (apnewfd->enable_check()) {
            //设置检测与服务器连接状态计时器
            std::shared_ptr<net::AcceptHeartbeat> ht = std::make_shared<net::AcceptHeartbeat>(
                apnewfd, timeval { apnewfd->check_second(), 0L });
            int id = utils::Singleton<net::TimerReactorMgr>::instance().add(ht);
            apnewfd->set_check_timeid(id);
        }
    }

    static std::atomic<int32_t> udp_new_connect_fd = 1;

    void AcceptThread::udp_accept(net::ListenEvent* apnf)
    {
        if (apnf->type() == SOCK_DGRAM) {
            char buffer[MAXIMUM_MTU_SIZE];
            struct sockaddr_storage cliaddr;
            memset(&cliaddr, 0, sizeof(cliaddr));
            socklen_t socklen = sizeof(cliaddr);

            ssize_t ret = recvfrom(apnf->listenfd(), buffer, MAXIMUM_MTU_SIZE, 0,
                reinterpret_cast<sockaddr*>(&cliaddr),
                &socklen);
            if (ret > 0) {
                //std::cout << " AcceptThread::udp_accept"
                //          << " ret:" << ret << std::endl;
                auto udpth = utils::Singleton<pool::ThreadPool>::instance().udpTh().lock();
                if (udpth) {
                    net::IPAddress ipaddr(&cliaddr);
                    auto sock = udpth->find_accept_udp(ipaddr).lock();
                    if (sock == nullptr) {
                        //处理不可靠消息
                        if (buffer[0] == (char)protocol::UdpMessageDefine::ID_ATTEMPT_CONNECT) {
                            net::IPAddressHash iphash;
                            uint32_t guid = static_cast<uint32_t>(iphash(ipaddr));
                            std::cout << "ID_ATTEMPT_CONNECT:" << guid << std::endl;
                            //udp_new_connect_fd++;
                            char msg[MAXIMUM_MTU_SIZE] { 0 };
                            msg[0] = (char)protocol::UdpMessageDefine::ID_ATTEMPT_CONNECT_ACK;
                            guid = htonl(guid);
                            memcpy(msg + sizeof(char), &guid, sizeof(guid));
                            sendto(apnf->listenfd(), msg, sizeof(msg), 0,
                                reinterpret_cast<sockaddr*>(&cliaddr),
                                socklen);
                        } else if (buffer[0] == (char)protocol::UdpMessageDefine::ID_CONNECT) {
                            //3次握手完成,建立可靠的udp连接层
                            uint32_t guid;
                            memcpy(&guid, buffer + sizeof(char), sizeof(guid));
                            guid = ntohl(guid);
                            //LOG_DEBUG << "ID_CONNECT:" << guid;
                            std::shared_ptr<net::IPAddress> newfdAddr = std::make_shared<net::IPAddress>(&cliaddr);
                            auto sock = std::make_shared<protocol::UdpSocket>(protocol::FilterProces::Identity::ACCEPTOR);
                            sock->fd = apnf->listenfd();
                            sock->addr = *newfdAddr;
                            sock->set_guid(guid);
                            sock->init_pair_buffer(base_);
                            sock->client = apnf->accept_newfd(newfdAddr, sock->output_buffer);
                            auto shclient = sock->client.lock();
                            if (shclient) {
                                shclient->set_udpsock(sock);
                                udpth->add_accept_udp(*newfdAddr, sock);
                            }
                        }
                    } else {
                        //std::cout << " AcceptThread::udp_accept111 guid:" << sock->guid()
                        //          << " ret:" << ret << std::endl;
                        if (buffer[0] == (char)protocol::UdpMessageDefine::ID_CLOSE) {
                            uint32_t guid;
                            memcpy(&guid, buffer + sizeof(char), sizeof(guid));
                            guid = htonl(guid);
                            if (sock->guid() == guid) {
                                auto shclient = sock->client.lock();
                                if (shclient) {
                                    shclient->close(net::CloseType::CLIENT_CLOSE);
                                }
                            }
                        } else {
                            //处理可靠消息
                            sock->recv(buffer, ret);
                        }
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

    int AcceptThread::run()
    {
        try {
            LOG_TRACE << "AcceptThread::run threadtype:" << (int)type();

            base_ = std::make_shared<events::EventBase>();
            base_->make_pair_buffer(pair_);
            pair_[0]->enabled(EV_WRITE);
            pair_[0]->disable(EV_READ);
            pair_[1]->enabled(EV_READ);
            pair_[1]->disable(EV_WRITE);
            pair_[1]->setcb(pipeReadcb, nullptr, nullptr, this);
            base_->loop(EVLOOP_NO_EXIT_ON_EMPTY);
            pair_[0].reset();
            pair_[1].reset();
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
        return 0;
    }
    int AcceptThread::stop()
    {
        base_->loopexit();
        return 0;
    }

    int AcceptThread::notify(const void* data, size_t len) const
    {
        return pair_[0]->write(data, len);
    }
}
}
