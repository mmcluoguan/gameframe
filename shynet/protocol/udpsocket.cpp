#include "shynet/protocol/udpsocket.h"
#include "shynet/basic.h"
#include "shynet/pool/threadpool.h"
#include "shynet/task/acceptreadiotask.h"
#include "shynet/task/connectreadiotask.h"
#include "shynet/utils/singleton.h"
#include <event2/util.h>

namespace shynet {
namespace protocol {

    static int udp_output_client(const char* data, int size, ikcpcb* kcp, void* ctx)
    {
        UdpSocket* udp = (UdpSocket*)ctx;
        if (udp) {
            ssize_t ret = udp->sendto(data, size);
            (void)ret;
            //std::cout << " sendto:" << udp->guid() << " ret:" << ret << std::endl;
        }
        return 0;
    }

    static void inputReadcb(struct bufferevent* bev, void* ptr)
    {
        events::EventBuffer pbuf(bev);
        size_t total_length = pbuf.inputbuffer()->length();
        std::unique_ptr<char[]> complete_data(new char[total_length]);
        pbuf.inputbuffer()->remove(complete_data.get(), total_length);
        UdpSocket* sock = reinterpret_cast<UdpSocket*>(ptr);
        sock->send(complete_data.get(), static_cast<int>(total_length));
    }

    static void outputReadcb(struct bufferevent* bev, void* ptr)
    {
        events::EventBuffer pbuf(bev);
        UdpSocket* sock = reinterpret_cast<UdpSocket*>(ptr);
        if (sock->ident() == FilterProces::Identity::ACCEPTOR) {
            auto aptnewfd = sock->client.lock();
            if (aptnewfd) {
                std::shared_ptr<task::AcceptReadIoTask> io = std::make_shared<task::AcceptReadIoTask>(aptnewfd);
                utils::Singleton<pool::ThreadPool>::instance().appendwork(sock->guid(), io);
            }
        } else if (sock->ident() == FilterProces::Identity::CONNECTOR) {
            auto shconector = sock->cnev.lock();
            if (shconector) {
                std::shared_ptr<task::ConnectReadIoTask> io
                    = std::make_shared<task::ConnectReadIoTask>(shconector);
                utils::Singleton<pool::ThreadPool>::instance().appendwork(sock->guid(), io);
            }
        }
    }

    UdpSocket::UdpSocket(FilterProces::Identity ident)
    {
        ident_ = ident;
    }

    UdpSocket::~UdpSocket()
    {
        //发送断开连接包
        char msg[MAXIMUM_MTU_SIZE] { 0 };
        msg[0] = static_cast<char>(protocol::UdpMessageDefine::ID_CLOSE);
        int32_t id = htonl(guid_);
        memcpy(msg + sizeof(char), &id, sizeof(id));
        sendto(msg, MAXIMUM_MTU_SIZE);

        //从udp线程移除
        auto udpth = utils::Singleton<pool::ThreadPool>::instance().udpTh().lock();
        if (udpth) {
            if (ident_ == FilterProces::Identity::ACCEPTOR) {
                udpth->remove_accept_udp(addr);
            } else if (ident_ == FilterProces::Identity::CONNECTOR) {
                udpth->remove_connect_udp(guid_);
            }
        }

        if (kcp_ != nullptr) {
            ikcp_release(kcp_);
            kcp_ = nullptr;
        }
    }

    void UdpSocket::init_pair_buffer(std::shared_ptr<events::EventBase> base)
    {
        std::shared_ptr<events::EventBuffer> pair[2];
        base->make_pair_buffer(pair);
        input_buffer = pair[0];
        output_buffer = pair[1];
        input_buffer->set_fd(guid_);
        input_buffer->enabled(EV_WRITE);
        input_buffer->enabled(EV_READ);
        input_buffer->setcb(inputReadcb, nullptr, nullptr, this);
        output_buffer->set_fd(guid_);
        output_buffer->enabled(EV_READ);
        output_buffer->enabled(EV_WRITE);
        output_buffer->setcb(outputReadcb, nullptr, nullptr, this);
    }

    int UdpSocket::send(const char* data, size_t size)
    {
        int ret = -1;
        int waitnum = ikcp_waitsnd(kcp_);
        if (waitnum < sndwnd_size_ * 2) {
            ret = ikcp_send(kcp_, data, static_cast<int>(size));
            ikcp_flush(kcp_);
        } else {
            LOG_WARN << "网络拥堵,发送队列中等待的包数量:" << waitnum
                     << " 发送窗口大小:" << sndwnd_size_;
        }
        return ret;
    }

    ssize_t UdpSocket::sendto(const char* data, size_t size)
    {
        return ::sendto(fd, data, size, 0,
            reinterpret_cast<sockaddr*>(addr.sockaddr()),
            addr.socketlen());
    }

    int UdpSocket::recv(char* data, size_t size)
    {
        ikcp_input(kcp_, data, size);

        size_t buflen = recv_buffer_length_ + size;
        std::unique_ptr<char[]> complete_data(new char[buflen]);
        int ret = ikcp_recv(kcp_, complete_data.get(), int(buflen));
        if (ret > 0) {
            //std::cout << " ikcp_recv:" << guid() << " ret:" << ret << std::endl;
            recv_buffer_length_ = 0;
            input_buffer->write(complete_data.get(), ret);
        } else {
            recv_buffer_length_ += size;
            //std::cout << " ikcp_recv:" << guid() << " ret:" << ret << std::endl;
        }
        return ret;
    }

    ssize_t UdpSocket::recvfrom(char* data, size_t size)
    {
        socklen_t len = addr.socketlen();
        return ::recvfrom(fd, data, size, 0,
            reinterpret_cast<sockaddr*>(addr.sockaddr()),
            &len);
    }

    void UdpSocket::set_guid(uint32_t guid)
    {
        guid_ = guid;
        if (kcp_ != nullptr)
            ikcp_release(kcp_);
        kcp_ = ikcp_create(guid, this);
        //ikcp_wndsize(kcp_, sndwnd_size_, rcvwnd_size_);
        //ikcp_nodelay(kcp_, 1, 10, 2, 0);
        kcp_->output = udp_output_client;
    }
}
}
