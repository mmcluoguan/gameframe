#ifndef FRMPUB_CLIENT_H
#define FRMPUB_CLIENT_H

#include "frmpub/filterdata.h"
#include "frmpub/pingtimer.h"
#include "shynet/events/eventbuffer.h"
#include "shynet/net/acceptnewfd.h"

namespace frmpub {
/**
 * @brief 客户端连接
*/
class Client : public net::AcceptNewFd, public FilterData {
public:
    /**
         * @brief 构造
         * @param remote_addr 客户端连接地址
         * @param listen_addr 服务器监听地址
         * @param iobuf 管理io读写缓冲区
         * @param options 网络配置选项
        */
    Client(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf,
        NetConfigOptions options = {});
    ~Client();

    /**
    * @brief 客户端连接断开回调
    * @param active 断开原因 CLIENT_CLOSE,SERVER_CLOSE,TIMEOUT_CLOSE
    */
    void close(net::CloseType active) override;
    /**
         * @brief 检测到与客户端没有心跳超时回调
         * @param active 断开原因 TIMEOUT_CLOSE
        */
    void timerout(net::CloseType) override;
    /**
    * @brief socket数据已经保存到管理io缓冲,可以读取回调
    * @return 返回SUCCESS正常没有操作,
    返回INITIATIVE_CLOSE服务器将关闭底层socket，并触发close(CLIENT_CLOSE)
    返回PASSIVE_CLOSE服务器将关闭底层socket，并触发close(SERVER_CLOSE)
    */
    net::InputResult input(std::function<void(std::unique_ptr<char[]>, size_t)> cb) override;
    /**
     * @brief 消息数据封包处理
     * @param original_data 指定数据的指针
     * @param datalen 指定数据的指针的大小
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int message_handle(char* original_data, size_t datalen) override;

    /**
     * @brief 获取连接断开原因
     * @return 连接断开原因
    */
    net::CloseType active() const { return active_; }

    /**
     * @brief 获取连接的客户端的监听的服务信息
     * @return 连接的客户端的监听的服务信息
    */
    protocc::ServerInfo sif() const { return sif_; }
    /**
     * @brief 设置连接的客户端的监听的服务信息
     * @param v 连接的客户端的监听的服务信息
    */
    void set_sif(const protocc::ServerInfo& v) { sif_ = v; }

private:
    /**
     * @brief 连接的客户端的监听的服务信息
    */
    protocc::ServerInfo sif_;
    /**
     * @brief 连接断开原因
    */
    net::CloseType active_ = net::CloseType::SERVER_CLOSE;
    /**
     * @brief ping客户端计时器
    */
    std::weak_ptr<PingTimer> ping_timer_;
    /**
     * @brief 是否发送心跳
    */
    bool enable_ping_ = false;
    /**
     * @brief 发送心跳时间间隔(s)
    */
    ssize_t heartSecond_ = 5;
};
}

#endif
