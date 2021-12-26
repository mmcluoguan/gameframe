#ifndef FRMPUB_CLIENT_H
#define FRMPUB_CLIENT_H

#include "frmpub/filterdata.h"
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
         * @param enableHeart 是否启用心跳包检测客户端
         * @param heartSecond 心跳包检测秒数
         * @param pt 协议类型 SHY,HTTP,WEBSOCKET
         * @param pd 数据封包 PROTOBUF,JSON,NATIVE
        */
    Client(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf,
        bool enableHeart = false, ssize_t heartSecond = 5,
        protocol::FilterProces::ProtoType pt = protocol::FilterProces::ProtoType::SHY,
        FilterData::ProtoData pd = FilterData::ProtoData::PROTOBUF);
    ~Client() = default;

    /**
    * @brief 客户端连接断开回调
    * @param active 断开原因 CLIENT_CLOSE,SERVER_CLOSE,TIMEOUT_CLOSE
    */
    void close(net::CloseType active) override;
    /**
         * @brief 心跳包检测到客户端超时回调
         * @param active 断开原因
        */
    void timerout(net::CloseType) override;
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
};
}

#endif
