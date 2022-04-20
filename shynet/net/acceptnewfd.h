#ifndef SHYNET_NET_ACCEPTNEWFD_H
#define SHYNET_NET_ACCEPTNEWFD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/net/ipaddress.h"
#include "shynet/protocol/filterproces.h"
#include "shynet/protocol/udpsocket.h"

namespace shynet {
namespace net {
    /**
     * @brief 接收客户端数据处理器
    */
    class AcceptNewFd : public protocol::FilterProces {
    public:
        /**
         * @brief 构造
         * @param remoteAddr 客户端连接地址
         * @param listenAddr 服务器监听地址
         * @param iobuf 管理io读写缓冲区
         * @param pt 协议类型 SHY,HTTP,WEBSOCKET
         * @param enable_check 是否启用检测与服务器连接状态
         * @param check_second 检测秒数
        */
        AcceptNewFd(std::shared_ptr<IPAddress> remoteAddr,
            std::shared_ptr<IPAddress> listenAddr,
            std::shared_ptr<events::EventBuffer> iobuf,
            FilterProces::ProtoType pt,
            bool enable_check = true, ssize_t check_second = 5);
        ~AcceptNewFd() = default;

        /**
         * @brief socket数据已经保存到管理io缓冲,可以读取回调
         * @param cb 解析出网络封包回调
         * @return 返回SUCCESS正常没有操作,
         返回INITIATIVE_CLOSE服务器将关闭底层socket，并触发close(SERVER_CLOSE)
         返回PASSIVE_CLOSE服务器将关闭底层socket，并触发close(CLIENT_CLOSE)
        */
        virtual InputResult input(std::function<void(std::unique_ptr<char[]>, size_t)> cb);
        /**
         * @brief 指定数据已经完成发送到管理io缓冲回调
         * @return  返回0正常没有操作
         返回-1服务器将关闭底层socket，并触发close(SERVER_CLOSE)
        */
        virtual int output() { return 0; };
        /**
         * @brief 客户端连接断开回调
         * @param active 断开原因 CLIENT_CLOSE,SERVER_CLOSE,TIMEOUT_CLOSE
        */
        void close(CloseType active) override = 0;
        /**
         * @brief 检测到与客户端没有心跳超时回调
         * @param active 断开原因 TIMEOUT_CLOSE
        */
        virtual void timerout(CloseType active) = 0;

        /**
         * @brief 获取连接的客户端地址
         * @return 连接的客户端地址
        */
        std::shared_ptr<IPAddress> remote_addr() const;
        /**
         * @brief 获取监听的服务器地址
         * @return 监听的服务器地址
        */
        std::shared_ptr<IPAddress> listen_addr() const;
        /**
         * @brief 获取是否启用检测与服务器连接状态
         * @return 是否启用检测与服务器连接状态
        */
        bool enable_check() const;
        /**
         * @brief 获取检测秒数
         * @return 检测秒数
        */
        ssize_t check_second() const;

        /**
         * @brief 获取socket文件描述符
         * @return socket文件描述符
        */
        uint32_t fd() const { return udpsocket_ ? udpsocket_->guid() : iobuf()->fd(); }

        /**
         * @brief 获取检测与客户端连接状态计时器id
         * @return 检测与客户端连接状态计时器id
        */
        int check_timeid() const;
        /**
         * @brief 设置检测与客户端连接状态计时器id
         * @param heart 检测与客户端连接状态计时器id
        */
        void set_check_timeid(int heart);
        /**
         * @brief 获取udpsocket
         * @return udpsocket
        */
        std::shared_ptr<protocol::UdpSocket> udpsock() const { return udpsocket_; };
        /**
         * @brief 设置udpsocket
         * @param udp udpsocket
        */
        void set_udpsock(std::shared_ptr<protocol::UdpSocket> udp) { udpsocket_ = udp; };

    private:
        /**
         * @brief 连接的客户端地址
        */
        std::shared_ptr<IPAddress> remote_addr_ = nullptr;
        /**
         * @brief 监听的服务器地址
        */
        std::shared_ptr<IPAddress> listen_addr_ = nullptr;
        /**
         * @brief 是否检测与服务器连接状态
        */
        bool enable_check_ = true;
        /**
         * @brief 检测秒数
        */
        ssize_t check_second_ = 5;
        /**
         * @brief 服务器检测与客户端连接状态计时器id
        */
        int check_timeid_;
        /**
         * @brief udpsocket
        */
        std::shared_ptr<protocol::UdpSocket> udpsocket_;
    };

}
}

#endif
