#ifndef DBVISIT_DBSERVER_H
#define DBVISIT_DBSERVER_H

#include "shynet/net/listenreactormgr.h"

namespace dbvisit {
/**
 * @brief 数据库服务器
*/
class DbServer final : public net::ListenEvent {
public:
    /**
     * @brief 构造
     * @param listen_addr 服务器监听地址 
    */
    explicit DbServer(std::shared_ptr<net::IPAddress> listen_addr);
    ~DbServer();

    /**
     * @brief 接收新的连接
     * @param remoteAddr 新连接的地址
     * @param iobuf 新连接读写缓冲区
     * @return 新的连接
    */
    std::weak_ptr<net::AcceptNewFd> accept_newfd(
        std::shared_ptr<net::IPAddress> remoteAddr,
        std::shared_ptr<events::EventBuffer> iobuf) override;

private:
};
}

#endif
