#ifndef SHYNET_NET_CONNECTEVENT_H
#define SHYNET_NET_CONNECTEVENT_H

#include "shynet/events/eventhandler.h"
#include "shynet/net/ipaddress.h"
#include "shynet/protocol/filterproces.h"
#include <event2/dns.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace shynet {
namespace net {
    /**
     * @brief 接收服务器数据处理器
    */
    class ConnectEvent : public protocol::FilterProces {
    public:
        /**
         * @brief 构造
         * @param connect_addr 需要连接的服务器地址
         * @param pt 协议类型
         * @param type SOCK_STREAM,SOCK_DGRAM
         * @param enable_ssl 是否启用ssl
         * @param enable_check 是否启用检测与服务器连接状态
         * @param check_second 检测秒数
        */
        ConnectEvent(std::shared_ptr<net::IPAddress> connect_addr,
            FilterProces::ProtoType pt,
            __socket_type type = SOCK_STREAM,
            bool enable_ssl = false,
            bool enable_check = true, ssize_t check_second = 5);

        /**
         * @brief 构造
         * @param hostname 服务器主机名
         * @param port 端口
         * @param pt 协议类型
         * @param type SOCK_STREAM,SOCK_DGRAM
         * @param enable_ssl 是否启用ssl
         * @param enableHeart 是否启用检测与服务器连接状态
         * @param heartSecond 检测秒数
        */
        ConnectEvent(const char* hostname, short port,
            FilterProces::ProtoType pt,
            __socket_type type = SOCK_STREAM,
            bool enable_ssl = false,
            bool enable_check = true, ssize_t check_second = 5);
        ~ConnectEvent();

        /**
         * @brief 获取服务器数据处理器id
         * @return 服务器数据处理器id
        */
        int connectid() const;
        /**
         * @brief 设置服务器数据处理器id
         * @param id 服务器数据处理器id
        */
        void connectid(int id);
        /**
         * @brief 获取socket类型 SOCK_STREAM,SOCK_DGRAM
         * @return SOCK_STREAM,SOCK_DGRAM
        */
        __socket_type type() const { return type_; };
        /*
		* 获取连接的服务器地址
        * @return 连接的服务器地址
		*/
        std::shared_ptr<net::IPAddress> connect_addr() const;
        /*
		* 获取是否启用ssl
        * @return 是否启用ssl
		*/
        bool enable_ssl() const;
        /**
         * @brief 获取ssl上下文
         * @return ssl上下文
        */
        SSL_CTX* ctx() const;

        /**
         * @brief 连接成功回调
        */
        virtual void success();
        /**
         * @brief socket数据已经保存到管理io缓冲,可以读取回调
         * @return 返回SUCCESS正常没有操作,
         返回INITIATIVE_CLOSE服务器将关闭底层socket，并触发close(CLIENT_CLOSE)
         返回PASSIVE_CLOSE服务器将关闭底层socket，并触发close(SERVER_CLOSE)
        */
        virtual InputResult input();
        /**
         * @brief 指定数据已经完成发送到管理io缓冲回调
         * @return  返回0正常没有操作
         返回-1服务器将关闭底层socket，并触发close(CLIENT_CLOSE)
        */
        virtual int output() { return 0; };
        /**
         * @brief 服务器连接断开回调
         * @param active 断开原因 CLIENT_CLOSE,SERVER_CLOSE,TIMEOUT_CLOSE
        */
        virtual void close(CloseType active) = 0;
        /**
         * @brief 检测到与服务器没有心跳超时回调
         * @param active 断开原因 TIMEOUT_CLOSE
        */
        virtual void timerout(CloseType active) = 0;

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
        int fd() const
        {
            return iobuf()->fd();
        }

        /**
         * @brief 获取检测与服务器连接状态计时器id
         * @return 检测与服务器连接状态计时器id
        */
        int check_timeid() const;
        /**
         * @brief 设置检测与服务器连接状态计时器id
         * @param heart 检测与服务器连接状态计时器id
        */
        void set_check_timeid(int ht);

        /**
         * @brief 获取端口
         * @return 端口
        */
        short dnsport() const { return dnsport_; };
        /**
         * @brief 获取连接的主机名
         * @return 连接的主机名
        */
        std::string hostname() const { return hostname_; };

        /**
         * @brief 获取dns对象
         * @return dns对象
        */
        evdns_base* dnsbase() const { return dnsbase_; };
        /**
         * @brief 设置dns对象
         * @param base dns对象 
        */
        void set_dnsbase(evdns_base* base) { dnsbase_ = base; };

    private:
        /**
         * @brief 需要连接的服务器地址
        */
        std::shared_ptr<net::IPAddress> connect_addr_ = nullptr;
        /**
         * @brief 是否启用ssl
        */
        bool enable_ssl_ = false;
        /**
         * @brief ssl上下文
        */
        SSL_CTX* ctx_ = nullptr;
        /**
         * @brief 服务器数据处理器id
        */
        int conectid_ = -1;
        /**
         * @brief SOCK_STREAM,SOCK_DGRAM
        */
        __socket_type type_;
        /**
         * @brief 是否检测与服务器连接状态
        */
        bool enable_check_ = true;
        /**
         * @brief 检测秒数
        */
        ssize_t check_second_ = 5;
        /**
         * @brief 客户端检测与服务器连接状态超时处理器id
        */
        int check_timeid_;
        /**
         * @brief dns对象
        */
        evdns_base* dnsbase_ = nullptr;
        /**
         * @brief 端口
        */
        short dnsport_ = 0;
        /**
         * @brief 连接的主机名
        */
        std::string hostname_;
    };

}
}

#endif
