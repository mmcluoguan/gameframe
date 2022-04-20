#include "shynet/net/connectiobuffer.h"
#include "shynet/net/connectheartbeat.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/pool/threadpool.h"
#include "shynet/task/connectreadiotask.h"
#include "shynet/utils/stuff.h"

namespace shynet {
namespace net {
    static void ioreadcb(struct bufferevent* bev, void* ptr)
    {
        try {
            ConnectIoBuffer* p = reinterpret_cast<ConnectIoBuffer*>(ptr);
            p->io_readcb();
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    static void iowritecb(struct bufferevent* bev, void* ptr)
    {
        try {
            ConnectIoBuffer* p = reinterpret_cast<ConnectIoBuffer*>(ptr);
            p->io_writecb();
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    static void ioeventcb(struct bufferevent* bev, short events, void* ptr)
    {
        try {
            ConnectIoBuffer* p = reinterpret_cast<ConnectIoBuffer*>(ptr);
            p->io_eventcb(events);
        } catch (const std::exception& err) {
            utils::stuff::print_exception(err);
        }
    }

    ConnectIoBuffer::ConnectIoBuffer(std::shared_ptr<events::EventBase> base, bool enable_ssl, SSL_CTX* ctx)
        : events::EventBuffer(base)
    {
        if (enable_ssl) {
            iobuf_ = std::make_shared<events::EventBufferSsl>(base, -1,
                BUFFEREVENT_SSL_CONNECTING,
                BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE, ctx);
        } else {
            iobuf_ = std::make_shared<events::EventBuffer>(base, -1,
                BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
        }
        set_buffer(iobuf_->buffer());

        iobuf_->setcb(ioreadcb, iowritecb, ioeventcb, this);
        iobuf_->enabled(EV_READ | EV_WRITE | EV_PERSIST);
    }

    std::weak_ptr<ConnectEvent> ConnectIoBuffer::cnev() const
    {
        return cnev_;
    }

    void ConnectIoBuffer::set_cnev(std::weak_ptr<ConnectEvent> cnev)
    {
        cnev_ = cnev;
    }

    void ConnectIoBuffer::io_readcb()
    {
        std::shared_ptr<ConnectEvent> shconector = cnev_.lock();
        if (shconector != nullptr) {
            if (shconector->enable_check()) {
                //延迟检测与服务器连接状态的计时处理器时间
                auto heart = utils::Singleton<TimerReactorMgr>::instance().find(shconector->check_timeid());
                if (heart) {
                    heart->set_val({ shconector->check_second(), 0L });
                }
            }
            net::InputResult ret = shconector->input([shconector](std::unique_ptr<char[]> data, size_t len) {
                std::shared_ptr<task::ConnectReadIoTask> io
                    = std::make_shared<task::ConnectReadIoTask>(shconector, std::move(data), len);
                utils::Singleton<pool::ThreadPool>::instance().appendwork(shconector->iobuf()->fd(), io);
            });
            if (ret == net::InputResult::INITIATIVE_CLOSE) {
                shconector->close(net::CloseType::CLIENT_CLOSE);
            } else if (ret == net::InputResult::PASSIVE_CLOSE) {
                shconector->close(net::CloseType::SERVER_CLOSE);
            }
        }
    }

    void ConnectIoBuffer::io_writecb()
    {
        std::shared_ptr<ConnectEvent> shconector = cnev_.lock();
        if (shconector != nullptr) {
            int ret = shconector->output();
            if (ret == -1) {
                shconector->close(net::CloseType::CLIENT_CLOSE);
            }
        }
    }

    void ConnectIoBuffer::io_eventcb(short events)
    {
        std::shared_ptr<ConnectEvent> shconector = cnev_.lock();
        if (shconector != nullptr) {
            if (events & BEV_EVENT_CONNECTED) {
                if (shconector->enable_check()) {
                    //设置检测与服务器连接状态计时处理器
                    std::shared_ptr<ConnectHeartbeat> ht
                        = std::make_shared<ConnectHeartbeat>(
                            shconector, timeval { shconector->check_second(), 0L });
                    int id = utils::Singleton<TimerReactorMgr>::instance().add(ht);
                    shconector->set_check_timeid(id);
                }
                shconector->success();
            } else if (events & BEV_EVENT_EOF) {
                shconector->close(net::CloseType::SERVER_CLOSE);
            } else if (events & (BEV_EVENT_ERROR | BEV_EVENT_READING | BEV_EVENT_WRITING | BEV_EVENT_TIMEOUT)) {
                if (shconector->dnsbase() != nullptr)
                    LOG_WARN << evutil_gai_strerror(bufferevent_socket_get_dns_error(iobuf_->buffer()));
                LOG_WARN << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
                shconector->close(net::CloseType::CONNECT_FAIL);
            }
        }
    }

}
}
