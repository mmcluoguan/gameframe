#ifndef SHYNET_THREAD_UDPTHREAD_H
#define SHYNET_THREAD_UDPTHREAD_H

#include "shynet/net/connectevent.h"
#include "shynet/protocol/udpsocket.h"
#include "shynet/thread/thread.h"
#include <list>
#include <mutex>

namespace shynet {
namespace thread {

    /**
     * @brief udp逻辑线程
    */
    class UdpThread : public Thread {
    public:
        /**
         * @brief 构造
         * @param index 线程在线程池中的索引
        */
        explicit UdpThread(size_t index);
        ~UdpThread() = default;

        /**
         * @brief 线程运行回调
         * @return 0成功 -1失败
        */
        int run() override;
        /**
         * @brief 安全终止线程
         * @return 0成功 -1失败
        */
        int stop() override;

        void add_waitconnect(std::weak_ptr<protocol::UdpSocket> sock);
        void add_accept_udp(net::IPAddress& ip, std::weak_ptr<protocol::UdpSocket> sock);
        void add_connect_udp(int64_t guid, std::weak_ptr<protocol::UdpSocket> sock);

        std::weak_ptr<protocol::UdpSocket> find_accept_udp(net::IPAddress& ip);

    private:
        /**
         * @brief 线程结束标识
        */
        bool stop_ = false;

        std::mutex waitconnect_list_mtx_;
        std::list<std::weak_ptr<protocol::UdpSocket>> waitconnect_list_;

        std::mutex accept_udp_mtx_;
        std::unordered_map<net::IPAddress, std::weak_ptr<protocol::UdpSocket>, net::IPAddressHash> accept_udp_layer_;

        std::mutex connect_udp_mtx_;
        std::unordered_map<int64_t, std::weak_ptr<protocol::UdpSocket>> connect_udp_layer_;
    };
}
}

#endif
