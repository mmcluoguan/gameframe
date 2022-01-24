#include "shynet/thread/udpthread.h"
#include <atomic>
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
        std::atomic<int64_t> sleepms = 10;
        auto start_time = std::chrono::steady_clock::now();
        while (!stop_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepms));
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            {
                std::lock_guard<std::mutex> lock(waitconnect_map_mtx_);
                for (auto&& [key, sock] : waitconnect_map_) {
                    int64_t t = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                    if (sock->next_request_time < t) {
                        //发包
                        sock->next_request_time = t + 500;
                    }
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

    void UdpThread::add(int32_t key, net::IPAddress* addr)
    {
        std::lock_guard<std::mutex> lock(waitconnect_map_mtx_);
        if (waitconnect_map_.find(key) == waitconnect_map_.end()) {
            waitconnect_map_.insert({ key, std::make_shared<UdpSocket>(key, addr) });
        }
    }

    int udp_output_client(const char* data, int size, ikcpcb* kcp, void* ctx)
    {
        UdpThread::UdpSocket* udp = (UdpThread::UdpSocket*)ctx;
        if (udp) {
            udp->sendto(data, size);
        }
        return 0;
    }

    UdpThread::UdpSocket::UdpSocket(int32_t key, net::IPAddress* addr)
    {
        key_ = key;
        addr_ = *addr;
        fd_ = socket(addr->family(), SOCK_DGRAM, IPPROTO_IP);
        if (fd_ == -1) {
            THROW_EXCEPTION("call socket");
        }
        if (evutil_make_socket_nonblocking(fd_) < 0) {
            evutil_closesocket(fd_);
            THROW_EXCEPTION("call evutil_make_socket_nonblocking");
        }
        if (evutil_make_socket_closeonexec(fd_) < 0) {
            evutil_closesocket(fd_);
            THROW_EXCEPTION("call evutil_make_socket_closeonexec");
        }
        kcp_ = ikcp_create(key, this);
        kcp_->output = udp_output_client;
    }
    UdpThread::UdpSocket::~UdpSocket()
    {
        if (fd_ != -1)
            evutil_closesocket(fd_);
        if (kcp_ != nullptr)
            ikcp_release(kcp_);
    }
    int UdpThread::UdpSocket::send(const char* data, size_t size)
    {
        return ikcp_send(kcp_, data, size);
    }
    int UdpThread::UdpSocket::sendto(const char* data, size_t size)
    {
        return ::sendto(fd_, data, size, 0,
            reinterpret_cast<const sockaddr*>(addr_.sockaddr()),
            addr_.socketlen());
    }
}
}
