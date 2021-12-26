#ifndef SHYNET_NET_ACCEPTNEWFD_H
#define SHYNET_NET_ACCEPTNEWFD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/net/acceptheartbeat.h"
#include "shynet/net/ipaddress.h"
#include "shynet/protocol/filterproces.h"

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
         * @param enableHeart 是否启用心跳包检测客户端
         * @param heartSecond 心跳包检测秒数
        */
        AcceptNewFd(std::shared_ptr<IPAddress> remoteAddr,
            std::shared_ptr<IPAddress> listenAddr,
            std::shared_ptr<events::EventBuffer> iobuf,
            FilterProces::ProtoType pt,
            bool enableHeart = true, ssize_t heartSecond = 5);
        ~AcceptNewFd() = default;

        /**
         * @brief socket数据已经保存到管理io缓冲,可以读取回调
         * @return 返回SUCCESS正常没有操作,
         返回INITIATIVE_CLOSE服务器将关闭底层socket，并触发close(SERVER_CLOSE)
         返回PASSIVE_CLOSE服务器将关闭底层socket，并触发close(CLIENT_CLOSE)
        */
        virtual InputResult input();
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
        virtual void close(CloseType active) = 0;
        /**
         * @brief 心跳包检测到客户端超时回调
         * @param active 断开原因
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
         * @brief 获取是否检测客户端心跳
         * @return 是否检测客户端心跳
        */
        bool enableHeart() const;
        /**
         * @brief 获取心跳包间隔时间(单位秒)
         * @return 心跳包间隔时间(单位秒)
        */
        ssize_t heart_second() const;

        /**
         * @brief 获取socket文件描述符
         * @return socket文件描述符
        */
        int fd() const { return iobuf()->fd(); }

        /**
         * @brief 获取心跳包计时器
         * @return 心跳包计时器
        */
        std::weak_ptr<AcceptHeartbeat> heart() const;
        /**
         * @brief 设置心跳包计时器
         * @param heart 心跳包计时器
        */
        void set_heart(std::weak_ptr<AcceptHeartbeat> heart);

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
         * @brief 是否检测客户端心跳
        */
        bool enable_heart_ = true;
        /**
         * @brief 心跳包检测秒数
        */
        ssize_t heart_second_ = 5;
        /**
         * @brief 心跳包计时器
        */
        std::weak_ptr<AcceptHeartbeat> heart_;
    };

}
}

#endif
