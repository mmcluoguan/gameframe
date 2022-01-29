#ifndef FRMPUB_CONNECTOR_H
#define FRMPUB_CONNECTOR_H

#include "frmpub/filterdata.h"
#include "frmpub/pingtimer.h"
#include "shynet/net/connectevent.h"

namespace frmpub {
/**
 * @brief 服务器连接器
*/
class Connector : public net::ConnectEvent, public FilterData {
public:
    /**
     * @brief 
     * @param connect_addr 需要连接的服务器地址
     * @param name 连接器名称
     * @param type SOCK_STREAM SOCK_DGRAM
     * @param enable_ssl 是否启用ssl
     * @param enable_ping 是否发送心跳
     * @param heartSecond 发送心跳时间间隔(s)
     * @param pt 协议类型 SHY,HTTP,WEBSOCKET
     * @param pd 数据封包类型 PROTOBUF,JSON,NATIVE
    */
    Connector(std::shared_ptr<net::IPAddress> connect_addr,
        std::string name = "Connector",
        __socket_type type = SOCK_DGRAM,
        bool enable_ssl = false,
        bool enable_ping = false,
        ssize_t heartSecond = 3,
        protocol::FilterProces::ProtoType pt = protocol::FilterProces::ProtoType::SHY,
        FilterData::ProtoData pd = FilterData::ProtoData::PROTOBUF);
    ~Connector();

    /**
    * @brief 服务器连接断开回调
    * @param active 断开原因 CLIENT_CLOSE,SERVER_CLOSE,TIMEOUT_CLOSE
    */
    void close(net::CloseType active) override;
    /**
    * @brief 检测到与服务器没有心跳超时回调
    * @param active 断开原因 TIMEOUT_CLOSE
    */
    void timerout(net::CloseType active) override;
    /**
    * @brief 连接成功回调
    */
    void success() override;
    /**
    * @brief socket数据已经保存到管理io缓冲,可以读取回调
    * @return 返回SUCCESS正常没有操作,
    返回INITIATIVE_CLOSE服务器将关闭底层socket，并触发close(CLIENT_CLOSE)
    返回PASSIVE_CLOSE服务器将关闭底层socket，并触发close(SERVER_CLOSE)
    */
    net::InputResult input() override;
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
    net::CloseType active() const;

    /*
		* 获取连接器名称
		*/
    std::string name() const { return name_; }

private:
    /**
     * @brief 连接断开原因
    */
    net::CloseType active_ = net::CloseType::CLIENT_CLOSE;
    /**
     * @brief ping服务器计时器
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
    /**
     * @brief 连接器名称
    */
    std::string name_ { "Connector" };
};
}

#endif
