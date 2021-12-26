#ifndef LOGIN_LOGINSERVER_H
#define LOGIN_LOGINSERVER_H

#include "shynet/net/listenreactormgr.h"

namespace login {
/**
 * @brief game服务器
*/
class LoginServer final : public net::ListenEvent {
public:
    /**
     * @brief 构造
     * @param listen_addr 服务器监听地址 
    */
    explicit LoginServer(std::shared_ptr<net::IPAddress> listen_addr);
    ~LoginServer();

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
