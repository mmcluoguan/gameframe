#include "shynet/thread/connectthread.h"
#include "shynet/net/connectiobuffer.h"
#include "shynet/net/connectreactormgr.h"
#include "shynet/pool/threadpool.h"
#include "shynet/protocol/udpsocket.h"
#include "shynet/utils/logger.h"
#include "shynet/utils/stuff.h"

namespace shynet {
extern pthread_barrier_t g_barrier;
namespace thread {
    ConnectThread::ConnectThread(size_t index)
        : Thread(ThreadType::CONNECT, index)
    {
    }

    static void pipeReadcb(struct bufferevent* bev, void* ptr)
    {
        try {
            ConnectThread* cnt = reinterpret_cast<ConnectThread*>(ptr);
            cnt->process(bev);
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    void ConnectThread::process(bufferevent* bev)
    {
        events::EventBuffer pbuf(bev);
        int connectid = 0;
        do {
            size_t len = pbuf.read(&connectid, sizeof(connectid));
            if (len == 0 || len != sizeof(connectid)) {
                LOG_WARN << "ConnectThread没有足够的数据";
                break;
            } else {
                std::shared_ptr<net::ConnectEvent> connect = utils::Singleton<net::ConnectReactorMgr>::instance().find(connectid);
                if (connect != nullptr) {
                    if (connect->type() == SOCK_STREAM) {
                        tcp_connect(connect);
                    } else {
                        udp_connect(connect);
                    }
                }
            }
        } while (true);
    }

    void ConnectThread::tcp_connect(std::shared_ptr<net::ConnectEvent> connect)
    {
        struct sockaddr* address = (struct sockaddr*)connect->connect_addr()->sockaddr();

        std::shared_ptr<net::ConnectIoBuffer> buffer;
        if (connect->enable_ssl()) {
            buffer = std::make_shared<net::ConnectIoBuffer>(base_, true, connect->ctx());
        } else {
            buffer = std::make_shared<net::ConnectIoBuffer>(base_, false);
        }
        buffer->set_cnev(connect);
        connect->set_iobuf(buffer);
        if (connect->dnsport() != 0) {
            evdns_base* dnsbase = evdns_base_new(base_->base(), 1);
            if (dnsbase == nullptr) {
                THROW_EXCEPTION("call connect_hostname")
            }
            connect->set_dnsbase(dnsbase);
            if (bufferevent_socket_connect_hostname(buffer->buffer(), dnsbase, AF_UNSPEC,
                    connect->hostname().c_str(), connect->dnsport())
                == -1) {
                THROW_EXCEPTION("call bufferevent_socket_connect_hostname")
            }
        } else {
            if (bufferevent_socket_connect(buffer->buffer(), address, sizeof(sockaddr_storage)) == -1) {
                THROW_EXCEPTION("call bufferevent_socket_connect")
            }
        }
    }

    void ConnectThread::udp_connect(std::shared_ptr<net::ConnectEvent> connect)
    {
        auto udpth = utils::Singleton<pool::ThreadPool>::instance().udpTh().lock();
        if (udpth) {
            udpth->connect_server(connect);
        } else {
            THROW_EXCEPTION("udpthread 未创建");
        }
    }

    int ConnectThread::run()
    {
        try {
            LOG_TRACE << "ConnectThread::run threadtype:" << (int)type();

            base_ = std::make_shared<events::EventBase>();
            base_->make_pair_buffer(pair_);
            pair_[0]->enabled(EV_WRITE);
            pair_[0]->disable(EV_READ);
            pair_[1]->enabled(EV_READ);
            pair_[1]->disable(EV_WRITE);
            pair_[1]->setcb(pipeReadcb, nullptr, nullptr, this);
            pthread_barrier_wait(&g_barrier);
            base_->loop(EVLOOP_NO_EXIT_ON_EMPTY);
            pair_[0].reset();
            pair_[1].reset();
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
        return 0;
    }
    int ConnectThread::stop()
    {
        base_->loopexit();
        return 0;
    }

    int ConnectThread::notify(int connectid) const
    {
        return pair_[0]->write(&connectid, sizeof(connectid));
    }
}
}
