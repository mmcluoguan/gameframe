#ifndef WORLD_HTTPCLIENT_H
#define WORLD_HTTPCLIENT_H

#include "frmpub/client.h"
#include "shynet/events/eventbuffer.h"

namespace world {
/**
 * @brief http客户端连接
*/
class HttpClient : public frmpub::Client, public std::enable_shared_from_this<HttpClient> {
public:
    /**
     * @brief 构造
     * @param remote_addr game客户端地址
     * @param listen_addr 服务器监听地址
     * @param iobuf 管理io读写缓冲区
    */
    HttpClient(std::shared_ptr<net::IPAddress> remote_addr,
        std::shared_ptr<net::IPAddress> listen_addr,
        std::shared_ptr<events::EventBuffer> iobuf);
    ~HttpClient();

    /**
     * @brief 处理protobuf数据封包
     * @param obj protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int input_handle(std::shared_ptr<rapidjson::Document> doc, std::shared_ptr<std::stack<FilterData::Envelope>> enves) override;

    /**
     * @brief 客户端连接与http服务器断开连接回调
     * @param active 连接断开原因
    */
    void close(net::CloseType active) override;

private:
    /**
     * @brief 接收http客户端发来的错误信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int errcode(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收http客户端发来的获取区服列表信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int getgamelist_admin_world_c(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收http客户端发来的通知消息信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int noticeserver_admin_world_c(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);

    /**
     * @brief 接收http客户端发来的发送邮件信息
     * @param data protobuf对象
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int sysemail_admin_world_c(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<FilterData::Envelope>> enves);
};
}

#endif
