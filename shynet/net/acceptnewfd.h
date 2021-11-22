#ifndef SHYNET_NET_ACCEPTNEWFD_H
#define SHYNET_NET_ACCEPTNEWFD_H

#include "shynet/events/eventbuffer.h"
#include "shynet/net/acceptheartbeat.h"
#include "shynet/net/ipaddress.h"
#include "shynet/protocol/filterproces.h"

namespace shynet {
namespace net {
    /// <summary>
    /// 客户端连接
    /// </summary>
    class AcceptNewFd : public protocol::FilterProces {
    public:
        /// <summary>
        /// 客户端连接
        /// </summary>
        /// <param name="remoteAddr">客户端连接地址</param>
        /// <param name="listenAddr">服务器监听地址</param>
        /// <param name="iobuf">io读写缓冲区</param>
        /// <param name="pt">协议类型</param>
        /// <param name="enableHeart">是否检测客户端心跳</param>
        /// <param name="heartSecond">心跳检测秒数</param>
        AcceptNewFd(std::shared_ptr<IPAddress> remoteAddr,
            std::shared_ptr<IPAddress> listenAddr,
            std::shared_ptr<events::EventBuffer> iobuf,
            FilterProces::ProtoType pt,
            bool enableHeart = true, ssize_t heartSecond = 5);
        ~AcceptNewFd();

        /// <summary>
        /// 可以读数据回调
        /// </summary>
        /// <returns>
        /// 返回0成功
        /// 返回-1服务器将关闭底层socket，并触发close(true)
        /// 返回-2服务器将关闭底层socket，并触发close(false)
        /// </returns>
        virtual int input();
        /// <summary>
        /// 指定数据已经完成发送到底层socket回调
        /// </summary>
        /// <returns>
        /// 返回0成功
        /// 返回-1服务器将关闭底层socket，并触发close(true)
        /// 返回-2服务器将关闭底层socket，并触发close(false)
        /// </returns>
        virtual int output() { return 0; };
        /// <summary>
        /// 断开连接
        /// </summary>
        /// <param name="active">true服务器主动断开,false客户端主动断开</param>
        virtual void close(bool active) = 0;
        /// <summary>
        /// 心跳超时回调
        /// </summary>
        virtual void timerout() = 0;

        /// <summary>
        /// 连接的客户端地址
        /// </summary>
        /// <returns></returns>
        std::shared_ptr<IPAddress> remote_addr() const;
        /// <summary>
        /// 监听的服务器地址
        /// </summary>
        /// <returns></returns>
        std::shared_ptr<IPAddress> listen_addr() const;
        /// <summary>
        /// 是否检测客户端心跳
        /// </summary>
        /// <returns></returns>
        bool enableHeart() const;
        /// <summary>
        /// 心跳间隔时间(单位秒)
        /// </summary>
        /// <returns></returns>
        ssize_t heart_second() const;

        int fd() const
        {
            return iobuf()->fd();
        }

        /*
			* 心跳计时器
			*/
        std::weak_ptr<AcceptHeartbeat> heart() const;
        void set_heart(std::weak_ptr<AcceptHeartbeat> heart);

    private:
        std::shared_ptr<IPAddress> remote_addr_ = nullptr;
        std::shared_ptr<IPAddress> listen_addr_ = nullptr;
        bool enable_heart_ = true;
        ssize_t heart_second_ = 5;
        std::weak_ptr<AcceptHeartbeat> heart_;
    };

}
}

#endif
