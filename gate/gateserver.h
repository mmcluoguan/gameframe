#ifndef GATE_GATESERVER_H
#define GATE_GATESERVER_H

#include "shynet/net/listenreactormgr.h"

namespace gate {
/**
 * @brief gate服务器
*/
class GateServer final : public net::ListenEvent {
public:
    /**
     * @brief 构造
     * @param listen_addr 服务器监听地址 
    */
    explicit GateServer(std::shared_ptr<net::IPAddress> listen_addr);
    ~GateServer();

    /**
     * @brief 接收新的连接
     * @param remoteAddr 新连接的地址
     * @param iobuf 新连接读写缓冲区
     * @return 新的连接
    */
    std::weak_ptr<net::AcceptNewFd> accept_newfd(std::shared_ptr<net::IPAddress> remoteAddr,
        std::shared_ptr<events::EventBuffer> iobuf) override;

private:
};
}

#endif
