#include "shynet/protocol/filterproces.h"
#include "shynet/net/ipaddress.h"
#include "shynet/utils/logger.h"
#include <chrono>

namespace shynet {
namespace protocol {
    FilterProces::FilterProces(std::shared_ptr<events::EventBuffer> iobuf,
        ProtoType pt, Identity ident)
    {
        iobuf_ = iobuf;
        pt_ = pt;
        ident_ = ident;
        if (pt_ == ProtoType::SHY)
            tcpip_.reset(new protocol::Tcpip(this));
        else if (pt_ == ProtoType::HTTP)
            http_.reset(new protocol::Http(this));
        else if (pt_ == ProtoType::WEBSOCKET)
            websocket_.reset(new protocol::WebSocket(this));
    }

    net::InputResult FilterProces::process(std::function<void(std::unique_ptr<char[]>, size_t)> cb)
    {
        switch (pt_) {
        case ProtoType::SHY:
            return tcpip_->process(cb);
        case ProtoType::HTTP:
            return http_->process(cb);
        case ProtoType::WEBSOCKET:
            return websocket_->process(cb);
        default:
            LOG_WARN << "未知的协议类型";
            return net::InputResult::INITIATIVE_CLOSE;
        }
    }

    FilterProces::Identity FilterProces::ident() const { return ident_; }

    int FilterProces::send(const char* data, size_t len) const
    {
        switch (pt_) {
        case ProtoType::SHY: {
            return tcpip_->send(data, len, Tcpip::FrameType::Binary);
        }
        case ProtoType::HTTP: {
            if (ident_ == Identity::ACCEPTOR) {
                return http_->send_responses(data, len);
            } else if (ident_ == Identity::CONNECTOR) {
                return http_->send_requset(data, len);
            }
        }
        case ProtoType::WEBSOCKET: {
            return websocket_->send(data, len, protocol::WebSocket::FrameType::Binary);
        }
        default:
            LOG_WARN << "未知的协议类型";
            return -1;
        }
        return -1;
    }

    int FilterProces::send(const std::string& data) const
    {
        return send(data.c_str(), data.length());
    }

    int FilterProces::ping() const
    {
        uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        switch (pt_) {
        case ProtoType::SHY: {
            return tcpip_->send(&timestamp, sizeof(timestamp), Tcpip::FrameType::Ping);
        }
        case ProtoType::WEBSOCKET: {
            return websocket_->send(&timestamp, sizeof(timestamp), protocol::WebSocket::FrameType::Ping);
        }
        default:
            LOG_WARN << "未知的协议类型";
            return -1;
        }
        return -1;
    }

    int FilterProces::request_handshake() const
    {
        if (pt_ == ProtoType::WEBSOCKET) {
            return websocket_->request_handshake();
        } else {
            LOG_WARN << "使用的不是websoket协议";
            return -1;
        }
    }

}
}
