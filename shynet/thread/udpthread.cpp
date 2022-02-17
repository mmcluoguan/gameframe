#include "shynet/thread/udpthread.h"
#include "shynet/net/acceptheartbeat.h"
#include "shynet/net/connectheartbeat.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/net/listenreactormgr.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/singleton.h"
#include "shynet/utils/stuff.h"
#include <event2/util.h>

namespace shynet {
namespace thread {
    UdpThread::UdpThread(size_t index)
        : Thread(ThreadType::UDP, index)
    {
    }

    static void pipeReadcb(struct bufferevent* bev, void* ptr)
    {
        try {
            UdpThread* rtk = reinterpret_cast<UdpThread*>(ptr);
            rtk->process(bev);
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    void UdpThread::process(bufferevent* bev)
    {
        events::EventBuffer pbuf(bev);
        int ident = 0;
        int targetid = 0;
        do {

            size_t len = pbuf.read(&ident, sizeof(ident));
            if (len == 0) {
                break;
            } else if (len != sizeof(ident)) {
                THROW_EXCEPTION("UdpThread没有足够的数据");
            }
            len = pbuf.read(&targetid, sizeof(targetid));
            if (len == 0) {
                break;
            } else if (len != sizeof(targetid)) {
                THROW_EXCEPTION("UdpThread没有足够的数据");
            }
            char buffer[MAXIMUM_MTU_SIZE];
            ssize_t ret;
            if (ident == static_cast<int>(protocol::FilterProces::Identity::ACCEPTOR)) {
                auto apnf = utils::Singleton<net::ListenReactorMgr>::instance().find(targetid);
                if (apnf) {
                    struct sockaddr_storage cliaddr;
                    memset(&cliaddr, 0, sizeof(cliaddr));
                    socklen_t socklen = sizeof(cliaddr);

                    ret = ::recvfrom(apnf->listenfd(), buffer, MAXIMUM_MTU_SIZE, 0,
                        reinterpret_cast<sockaddr*>(&cliaddr),
                        &socklen);
                    if (ret > 0) {
                        net::IPAddress ipaddr(&cliaddr);
                        auto sock = find_accept_udp(ipaddr).lock();
                        if (sock == nullptr) {
                            handler_accept_no_reliable(apnf, cliaddr, buffer, ret);
                        } else {
                            handler_accept_reliable(sock, buffer, ret);
                        }
                    }
                }
            } else if (ident == static_cast<int>(protocol::FilterProces::Identity::CONNECTOR)) {
                auto cnt = utils::Singleton<net::ConnectReactorMgr>::instance().find(targetid);
                if (cnt) {
                    ret = cnt->udpsock()->recvfrom(buffer, MAXIMUM_MTU_SIZE);
                    if (ret > 0) {
                        if (cnt->udpsock()->guid() == static_cast<uint32_t>(-1)) {
                            handler_connect_no_reliable(cnt, buffer, ret);
                        } else {
                            handler_connect_reliable(cnt->udpsock(), buffer, ret);
                        }
                    }
                }
            }
            if (ret <= 0) {
                if (errno != EAGAIN) {
                    std::error_condition econd = std::system_category().default_error_condition(errno);
                    LOG_WARN << "[" << econd.category().name()
                             << "(" << econd.value() << ")"
                             << " " << econd.message() << "] ";
                }
            }
        } while (true);
    }

    void UdpThread::handler_accept_reliable(std::shared_ptr<protocol::UdpSocket> sock, char* buffer, ssize_t size)
    {
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
            sock->recv(buffer, size);
        }
    }

    void UdpThread::handler_accept_no_reliable(std::shared_ptr<net::ListenEvent> apnf, sockaddr_storage& cliaddr, char* buffer, ssize_t size)
    {
        socklen_t socklen = sizeof(cliaddr);
        //处理不可靠消息
        if (buffer[0] == (char)protocol::UdpMessageDefine::ID_ATTEMPT_CONNECT) {
            uint32_t guid;
            utils::stuff::random(&guid, sizeof(uint32_t));
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
            std::shared_ptr<net::IPAddress> newfdAddr = std::make_shared<net::IPAddress>(&cliaddr);
            auto sock = std::make_shared<protocol::UdpSocket>(protocol::FilterProces::Identity::ACCEPTOR);
            sock->fd = apnf->listenfd();
            sock->addr = *newfdAddr;
            sock->set_guid(guid);
            sock->init_pair_buffer(base_);
            sock->output_buffer->set_fd(guid);
            sock->client = apnf->accept_newfd(newfdAddr, sock->output_buffer);
            auto shclient = sock->client.lock();
            if (shclient) {
                shclient->set_udpsock(sock);
                add_accept_udp(*newfdAddr, sock);
                if (shclient->enable_check()) {
                    //设置检测与客户端连接状态计时器
                    std::shared_ptr<net::AcceptHeartbeat> ht = std::make_shared<net::AcceptHeartbeat>(
                        shclient, timeval { shclient->check_second(), 0L });
                    int id = utils::Singleton<net::TimerReactorMgr>::instance().add(ht);
                    shclient->set_check_timeid(id);
                }
            }
        }
    }

    void UdpThread::connect_server(std::shared_ptr<net::ConnectEvent> connect)
    {
        int fd = socket(connect->connect_addr()->family(), SOCK_DGRAM, IPPROTO_IP);
        if (fd == -1) {
            THROW_EXCEPTION("call socket");
        }
        if (evutil_make_socket_nonblocking(fd) < 0) {
            evutil_closesocket(fd);
            THROW_EXCEPTION("call evutil_make_socket_nonblocking");
        }
        if (evutil_make_socket_closeonexec(fd) < 0) {
            evutil_closesocket(fd);
            THROW_EXCEPTION("call evutil_make_socket_closeonexec");
        }
        auto sock = std::make_shared<protocol::UdpSocket>(protocol::FilterProces::Identity::CONNECTOR);
        sock->fd = fd;
        sock->addr = *connect->connect_addr();
        sock->cnev = connect;
        connect->set_event(base_, fd, EV_READ | EV_PERSIST);
        base_->addevent(connect, nullptr);
        connect->set_udpsock(sock);
        add_waitconnect(connect->connectid());
    }

    void UdpThread::handler_connect_reliable(std::shared_ptr<protocol::UdpSocket> sock, char* buffer, ssize_t size)
    {
        if (buffer[0] == (char)protocol::UdpMessageDefine::ID_CLOSE) {
            uint32_t guid;
            memcpy(&guid, buffer + sizeof(char), sizeof(guid));
            guid = htonl(guid);
            if (sock->guid() == guid) {
                auto shclient = sock->cnev.lock();
                if (shclient) {
                    shclient->close(net::CloseType::SERVER_CLOSE);
                }
            }
        } else {
            sock->recv(buffer, size);
        }
    }

    void UdpThread::handler_connect_no_reliable(std::shared_ptr<net::ConnectEvent> cnt, char* buffer, ssize_t size)
    {
        auto sock = cnt->udpsock();
        if (buffer[0] == (char)protocol::UdpMessageDefine::ID_ATTEMPT_CONNECT_ACK
            && sock->st == protocol::UdpSocket::CONNECTING) {
            uint32_t guid;
            memcpy(&guid, buffer + sizeof(char), sizeof(guid));
            guid = ntohl(guid);
            //建立可靠的udp连接层
            sock->set_guid(guid);
            sock->st = protocol::UdpSocket::CONNECTSUCCESS;
            sock->init_pair_buffer(base_);
            sock->output_buffer->set_fd(guid);
            cnt->set_iobuf(sock->output_buffer);
            add_connect_udp(guid, sock);
            //发送第3次握手
            buffer[0] = (char)protocol::UdpMessageDefine::ID_CONNECT;
            sock->sendto(buffer, MAXIMUM_MTU_SIZE);
            if (cnt->enable_check()) {
                //设置检测与服务器连接状态计时处理器
                std::shared_ptr<net::ConnectHeartbeat> ht
                    = std::make_shared<net::ConnectHeartbeat>(
                        cnt, timeval { cnt->check_second(), 0L });
                int id = utils::Singleton<net::TimerReactorMgr>::instance().add(ht);
                cnt->set_check_timeid(id);
            }
            cnt->success();
        }
    }

    static void time_out(evutil_socket_t fd, short what, void* arg)
    {
        try {
            UdpThread* rtk = reinterpret_cast<UdpThread*>(arg);
            auto elapsed = std::chrono::steady_clock::now() - rtk->start_time();
            int64_t elapsed_num = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
            rtk->handler_waitconnenct(elapsed_num);
            rtk->handler_connected(elapsed_num);
            rtk->handler_accpeted(elapsed_num);
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    void UdpThread::handler_waitconnenct(int64_t elapsed_num)
    {
        std::lock_guard<std::mutex> lock(waitconnect_list_mtx_);
        auto iter = waitconnect_list_.begin();
        while (iter != waitconnect_list_.end()) {
            auto connector = utils::Singleton<net::ConnectReactorMgr>::instance().find(*iter);
            if (connector) {
                std::shared_ptr<protocol::UdpSocket> sock = connector->udpsock();
                if (sock->next_request_time < elapsed_num) {
                    if (sock->curr_request_num == sock->send_connection_attempt_count + 1
                        || sock->st == protocol::UdpSocket::CONNECTSUCCESS) {
                        if (sock->curr_request_num == sock->send_connection_attempt_count + 1) {
                            //尝试连接服务器失败
                            sock->st = protocol::UdpSocket::CONNECTFAIL;
                            auto cnt = sock->cnev.lock();
                            if (cnt)
                                cnt->close(net::CloseType::CONNECT_FAIL);
                        }
                        iter = waitconnect_list_.erase(iter);
                        continue;
                    } else {
                        sock->next_request_time = elapsed_num + sock->next_requset_interval;
                        sock->curr_request_num++;
                        char msg[MAXIMUM_MTU_SIZE] { 0 };
                        msg[0] = static_cast<char>(protocol::UdpMessageDefine::ID_ATTEMPT_CONNECT);
                        sock->sendto(msg, MAXIMUM_MTU_SIZE);
                    }
                }
            }
            ++iter;
        }
    }

    void UdpThread::handler_connected(int64_t elapsed_num)
    {
        std::lock_guard<std::mutex> lock(connect_udp_mtx_);
        if (connect_udp_layer_.empty() == false) {
            auto iter = connect_udp_layer_.begin();
            while (iter != connect_udp_layer_.end()) {
                std::shared_ptr<protocol::UdpSocket> sock = iter->second.lock();
                if (sock && sock->kcp()) {
                    ikcp_update(sock->kcp(), static_cast<uint32_t>(elapsed_num));
                    updata_elapsed_ = ikcp_check(sock->kcp(), static_cast<uint32_t>(elapsed_num)) - elapsed_num;
                } else {
                    iter = connect_udp_layer_.erase(iter);
                    continue;
                }
                ++iter;
            }
            struct timeval val {
                0, updata_elapsed_ * 1000
            };
            event_add(updata_timer_, &val);
        }
    }

    void UdpThread::handler_accpeted(int64_t elapsed_num)
    {
        std::lock_guard<std::mutex> lock(accept_udp_mtx_);
        if (accept_udp_layer_.empty() == false) {
            auto iter = accept_udp_layer_.begin();
            while (iter != accept_udp_layer_.end()) {
                std::shared_ptr<protocol::UdpSocket> sock = iter->second.lock();
                if (sock && sock->kcp()) {
                    ikcp_update(sock->kcp(), static_cast<uint32_t>(elapsed_num));
                    updata_elapsed_ = ikcp_check(sock->kcp(), static_cast<uint32_t>(elapsed_num)) - elapsed_num;
                } else {
                    iter = accept_udp_layer_.erase(iter);
                    continue;
                }
                ++iter;
            }
            struct timeval val {
                0, updata_elapsed_ * 1000
            };
            event_add(updata_timer_, &val);
        }
    };

    int UdpThread::run()
    {
        LOG_TRACE << "AcceptThread::run threadtype:" << (int)type();
        start_time_ = std::chrono::steady_clock::now();
        base_ = std::make_shared<events::EventBase>();
        base_->make_pair_buffer(pair_);
        pair_[0]->enabled(EV_WRITE);
        pair_[0]->disable(EV_READ);
        pair_[1]->enabled(EV_READ);
        pair_[1]->disable(EV_WRITE);
        pair_[1]->setcb(pipeReadcb, nullptr, nullptr, this);
        updata_timer_ = event_new(base_->base(), -1, EV_TIMEOUT | EV_PERSIST, time_out, this);
        struct timeval val {
            0, updata_elapsed_ * 1000
        };
        event_add(updata_timer_, &val);
        base_->loop(EVLOOP_NO_EXIT_ON_EMPTY);
        event_del(updata_timer_);
        pair_[0].reset();
        pair_[1].reset();
        return 0;
    }

    int UdpThread::stop()
    {
        base_->loopexit();
        return 0;
    }

    int UdpThread::notify(int ident, int targetid) const
    {
        char temp[sizeof(ident) + sizeof(targetid)];
        memcpy(temp, &ident, sizeof(ident));
        memcpy(temp + sizeof(ident), &targetid, sizeof(targetid));
        return pair_[0]->write(temp, sizeof(temp));
    }

    void UdpThread::add_waitconnect(int connectid)
    {
        std::lock_guard<std::mutex> lock(waitconnect_list_mtx_);
        waitconnect_list_.push_back(connectid);
    }

    void UdpThread::add_accept_udp(net::IPAddress& ip, std::weak_ptr<protocol::UdpSocket> sock)
    {
        std::lock_guard<std::mutex> lock(accept_udp_mtx_);
        if (accept_udp_layer_.find(ip) == accept_udp_layer_.end())
            accept_udp_layer_.insert({ ip, sock });
    }

    void UdpThread::remove_accept_udp(net::IPAddress& ip)
    {
        std::lock_guard<std::mutex> lock(accept_udp_mtx_);
        accept_udp_layer_.erase(ip);
    }

    void UdpThread::add_connect_udp(uint32_t guid, std::weak_ptr<protocol::UdpSocket> sock)
    {
        std::lock_guard<std::mutex> lock(connect_udp_mtx_);
        if (connect_udp_layer_.find(guid) == connect_udp_layer_.end())
            connect_udp_layer_.insert({ guid, sock });
    }

    void UdpThread::remove_connect_udp(uint32_t guid)
    {
        std::lock_guard<std::mutex> lock(connect_udp_mtx_);
        connect_udp_layer_.erase(guid);
    }

    std::weak_ptr<protocol::UdpSocket> UdpThread::find_accept_udp(net::IPAddress& ip)
    {
        std::lock_guard<std::mutex> lock(accept_udp_mtx_);
        auto it = accept_udp_layer_.find(ip);
        if (it != accept_udp_layer_.end()) {
            return it->second;
        }
        return std::weak_ptr<protocol::UdpSocket>();
    }
}
}
