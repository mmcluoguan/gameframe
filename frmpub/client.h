#ifndef FRMPUB_CLIENT_H
#define FRMPUB_CLIENT_H

#include "frmpub/filterdata.h"
#include "shynet/events/eventbuffer.h"
#include "shynet/net/acceptnewfd.h"

namespace frmpub {
/// <summary>
/// 客户端连接
/// </summary>
class Client : public net::AcceptNewFd, public FilterData {
public:
    /// <summary>
    /// 客户端连接
    /// </summary>
    /// <param name="remote_addr">客户端连接地址</param>
    /// <param name="listen_addr">服务器监听地址</param>
    /// <param name="iobuf">io读写缓冲区</param>
    /// <param name="enableHeart">是否检测心跳</param>
    /// <param name="heartSecond">心跳检测秒数</param>
    /// <param name="pt">协议类型</param>
    /// <param name="pd">数据封包类型</param>
    Client(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf,
        bool enableHeart = false, ssize_t heartSecond = 5,
        protocol::FilterProces::ProtoType pt = protocol::FilterProces::ProtoType::SHY,
        FilterData::ProtoData pd = FilterData::ProtoData::PROTOBUF);
    ~Client();

    /*
		* 连接断开
		active=true服务器主动断开,false客户端主动断开
		*/
    void close(bool active) override;
    /// <summary>
    /// 心跳超时
    /// </summary>
    void timerout() override;
    /*
		* 消息数据封包处理
		*/
    int message_handle(char* original_data, size_t datalen) override;

    /*
		* true服务器主动断开,false客户端主动断开
		*/
    bool active() const
    {
        return active_;
    }

    /*
		* 获取或设置连接的客户端的监听的服务信息
		*/
    protocc::ServerInfo sif() const
    {
        return sif_;
    }
    void set_sif(const protocc::ServerInfo& v)
    {
        sif_ = v;
    }

private:
    protocc::ServerInfo sif_;
    bool active_ = false;
};
}

#endif
