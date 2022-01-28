#include "shynet/thread/udpthread.h"
#include "shynet/utils/logger.h"
#include <event2/util.h>

namespace shynet {
extern pthread_barrier_t g_barrier;
namespace thread {
    UdpThread::UdpThread(size_t index)
        : Thread(ThreadType::UDP, index)
    {
    }

    int UdpThread::run()
    {
        int64_t sleepms = 10;
        auto start_time = std::chrono::steady_clock::now();
        while (!stop_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepms));
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            int64_t elapsed_num = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
            //处理待连接列表
            {
                std::lock_guard<std::mutex> lock(waitconnect_list_mtx_);
                auto iter = waitconnect_list_.begin();
                while (iter != waitconnect_list_.end()) {
                    std::shared_ptr<protocol::UdpSocket> sock = (*iter).lock();
                    if (sock) {
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
                    } else {
                        iter = waitconnect_list_.erase(iter);
                        continue;
                    }
                    ++iter;
                }
            }
            //处理已连接的udp连接
            {
                std::lock_guard<std::mutex> lock(connect_udp_mtx_);
                auto iter = connect_udp_layer_.begin();
                while (iter != connect_udp_layer_.end()) {
                    std::shared_ptr<protocol::UdpSocket> sock = iter->second.lock();
                    if (sock && sock->kcp()) {
                        ikcp_update(sock->kcp(), static_cast<uint32_t>(elapsed_num));
                        sleepms = ikcp_check(sock->kcp(), static_cast<uint32_t>(elapsed_num)) - elapsed_num;
                    } else {
                        iter = connect_udp_layer_.erase(iter);
                        continue;
                    }
                    ++iter;
                }
            }
            //处理连接udp服务器的client
            {
                std::lock_guard<std::mutex> lock(accept_udp_mtx_);
                auto iter = accept_udp_layer_.begin();
                while (iter != accept_udp_layer_.end()) {
                    std::shared_ptr<protocol::UdpSocket> sock = iter->second.lock();
                    if (sock && sock->kcp()) {
                        ikcp_update(sock->kcp(), static_cast<uint32_t>(elapsed_num));
                        sleepms = ikcp_check(sock->kcp(), static_cast<uint32_t>(elapsed_num)) - elapsed_num;
                    } else {
                        iter = accept_udp_layer_.erase(iter);
                        continue;
                    }
                    ++iter;
                }
            }
        }
        return 0;
    }
    int UdpThread::stop()
    {
        stop_ = true;
        return 0;
    }

    void UdpThread::add_waitconnect(std::weak_ptr<protocol::UdpSocket> sock)
    {
        std::lock_guard<std::mutex> lock(waitconnect_list_mtx_);
        waitconnect_list_.push_back(sock);
    }

    void UdpThread::add_accept_udp(net::IPAddress& ip, std::weak_ptr<protocol::UdpSocket> sock)
    {
        std::lock_guard<std::mutex> lock(accept_udp_mtx_);
        if (accept_udp_layer_.find(ip) == accept_udp_layer_.end())
            accept_udp_layer_.insert({ ip, sock });
    }

    void UdpThread::add_connect_udp(int64_t guid, std::weak_ptr<protocol::UdpSocket> sock)
    {
        std::lock_guard<std::mutex> lock(connect_udp_mtx_);
        if (connect_udp_layer_.find(guid) == connect_udp_layer_.end())
            connect_udp_layer_.insert({ guid, sock });
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
