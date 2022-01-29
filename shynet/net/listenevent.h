#ifndef SHYNET_NET_LISTENEVENT_H
#define SHYNET_NET_LISTENEVENT_H

#include "shynet/events/eventhandler.h"
#include "shynet/net/acceptnewfd.h"
#include "shynet/net/ipaddress.h"
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace shynet {
namespace net {
    /**
     * @brief 侦听的服务器地址发生有新连接处理器
    */
    class ListenEvent : public events::EventHandler {
    public:
        /**
         * @brief 构造
         * @param listen_addr 侦听的服务器地址
         * @param type SOCK_STREAM,SOCK_DGRAM
         * @param enable_ssl 是否启用ssl
        */
        ListenEvent(std::shared_ptr<net::IPAddress> listen_addr,
            __socket_type type = SOCK_DGRAM,
            bool enable_ssl = false);
        ~ListenEvent();

        /**
         * @brief 获取服务器id
         * @return 服务器id
        */
        int serverid() const { return serverid_; }
        /**
         * @brief 设置服务器id
         * @param id 服务器id
        */
        void set_serverid(int id) { serverid_ = id; }
        /**
         * @brief 获取侦听的服务器地址
         * @return 侦听的服务器地址
        */
        std::shared_ptr<net::IPAddress> listenaddr() const { return listen_addr_; }
        /**
         * @brief 获取侦听的服务器socket文件描述符
         * @return 侦听的服务器socket文件描述符
        */
        int listenfd() const { return listenfd_; }
        /**
         * @brief 获取是否启用ssl
         * @return 是否启用ssl
        */
        bool enable_ssl() const { return enable_ssl_; }
        /**
         * @brief 获取ssl上下文
         * @return ssl上下文
        */
        SSL_CTX* ctx() const { return ctx_; }
        /**
         * @brief 获取socket类型 SOCK_STREAM,SOCK_DGRAM
         * @return SOCK_STREAM,SOCK_DGRAM
        */
        __socket_type type() const { return type_; };
        /**
         * @brief 侦听的服务器地址准备完成回调
         * @param listenfd socket文件描述符
        */
        void input(int listenfd) override;
        /**
         * @brief 关闭侦听的服务器
         * @return true成功,false失败
        */
        bool stop() const;

        /**
         * @brief 暂停侦听服务器地址
        */
        void pause();

        /**
         * @brief 重新开始侦听服务器地址
        */
        void resume();

        /**
         * @brief 创建新客户端连接实例
         * @param remoteAddr 客户端连接地址
         * @param iobuf 管理io缓冲
         * @return 新客户端连接实例
        */
        virtual std::weak_ptr<net::AcceptNewFd> accept_newfd(
            std::shared_ptr<net::IPAddress> remoteAddr,
            std::shared_ptr<events::EventBuffer> iobuf)
            = 0;

    private:
        /**
         * @brief 初始化tcp
        */
        void init_tcp();
        /**
         * @brief 初始化udp
        */
        void init_udp();
        /**
         * @brief 侦听的服务器socket文件描述符
        */
        int listenfd_ = 0;
        /**
         * @brief 侦听的服务器地址
        */
        int serverid_ = -1;
        /**
         * @brief SOCK_STREAM,SOCK_DGRAM
        */
        __socket_type type_;
        /**
         * @brief 侦听的服务器地址
        */
        std::shared_ptr<net::IPAddress> listen_addr_ = nullptr;
        /**
         * @brief 是否启用ssl
        */
        bool enable_ssl_ = false;
        /**
         * @brief ssl上下文
        */
        SSL_CTX* ctx_ = nullptr;
        /**
         * @brief 是否已经暂停侦听服务器地址
        */
        bool ispause_ = false;
    };
}
}

#endif
