#ifndef WORLD_HTTPSERVER_H
#define WORLD_HTTPSERVER_H

#include "shynet/net/listenreactormgr.h"

namespace world {
/**
 * @brief http服务器
*/
class HttpServer final : public net::ListenEvent {
public:
    /**
     * @brief 构造
     * @param listen_addr 服务器监听地址 
    */
    explicit HttpServer(std::shared_ptr<net::IPAddress> listen_addr);
    ~HttpServer();

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
