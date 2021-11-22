#ifndef DBVISIT_DBSERVER_H
#define DBVISIT_DBSERVER_H

#include "shynet/net/listenreactormgr.h"

namespace dbvisit {
/// <summary>
/// 数据库服务器
/// </summary>
class DbServer final : public net::ListenEvent {
public:
    explicit DbServer(std::shared_ptr<net::IPAddress> listen_addr);
    ~DbServer();

    /// <summary>
    /// 接收新的连接
    /// </summary>
    /// <param name="remoteAddr">新连接的地址</param>
    /// <param name="iobuf">新连接读写缓冲区</param>
    /// <returns>新的连接</returns>
    std::weak_ptr<net::AcceptNewFd> accept_newfd(
        std::shared_ptr<net::IPAddress> remoteAddr,
        std::shared_ptr<events::EventBuffer> iobuf) override;

private:
};
}

#endif
