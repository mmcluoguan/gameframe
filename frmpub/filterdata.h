#ifndef FRMPUB_FILTERDATA_H
#define FRMPUB_FILTERDATA_H

#include "3rd/rapidjson/document.h"
#include "frmpub/basic.h"
#include "shynet/protocol/filterproces.h"

namespace frmpub {
/**
 * @brief 消息数据封包过滤器
*/
class FilterData : public shynet::Nocopy {
public:
    /**
     * @brief 数据封包类型
    */
    enum class ProtoData {
        PROTOBUF = 0,
        JSON,
        NATIVE,
    };

    /**
     * @brief 消息路由信封
    */
    struct Envelope {
        uint32_t fd;
        /**
         * @brief 消息源地址
        */
        struct sockaddr_storage addr;
        void set_addr(const std::shared_ptr<net::IPAddress> ipaddr)
        {
            addr = *ipaddr->sockaddr();
        }

        net::IPAddress get_addr() const
        {
            return net::IPAddress(&addr);
        }
        void set_fd(uint32_t value)
        {
            fd = value;
        }
        uint32_t get_fd() const
        {
            return fd;
        }
    };

    /**
     * @brief 构造
     * @param pd 数据封包类型 
    */
    FilterData(ProtoData pd);
    ~FilterData() = default;

    /**
     * @brief 消息数据封包处理
     * @param original_data 指定数据的指针
     * @param datalen 指定数据的指针的大小
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int message_handle(char* original_data, size_t datalen);

    /**
     * @brief 处理protobuf数据封包
     * @param obj protobuf
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    virtual int input_handle(const std::shared_ptr<protocc::CommonObject> obj,
        std::shared_ptr<std::stack<Envelope>> enves)
    {
        return 0;
    };
    /**
     * @brief 处理json数据封包
     * @param obj json文档
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    virtual int input_handle(const std::shared_ptr<rapidjson::Document> obj,
        std::shared_ptr<std::stack<Envelope>> enves)
    {
        return 0;
    }
    /**
     * @brief 处理原始数据封包
     * @param original_data 指向数据的指针
     * @param datalen 指向数据的指针大小
     * @param enves 路由信息
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    virtual int input_handle(const char* original_data, size_t datalen,
        std::shared_ptr<std::stack<Envelope>> enves)
    {
        return 0;
    };

    /**
     * @brief  发送proto数据封包
     * @param data protobuf
     * @param enves 路由信息
     * @return 0成功 -1失败
    */
    int send_proto(protocc::CommonObject* data,
        std::stack<Envelope>* enves = nullptr) const;
    /**
     * @brief  发送proto数据封包
     * @param msgid msgid
     * @param data protobuf
     * @param enves 路由信息
     * @param extend 扩展字符串数据
     * @return 0成功 -1失败
    */
    int send_proto(int msgid, const google::protobuf::Message* data = nullptr,
        std::stack<Envelope>* enves = nullptr, const std::string* extend = nullptr) const;
    /**
     * @brief  发送proto数据封包
     * @param msgid msgid
     * @param data protobuf系列化后的字符串
     * @param enves 路由信息
     * @param extend 扩展字符串数据
     * @return 0成功 -1失败
    */
    int send_proto(int msgid, const std::string data,
        std::stack<Envelope>* enves = nullptr, const std::string* extend = nullptr) const;

    /**
     * @brief 发送json数据封包
     * @param doc json文档
     * @param enves 路由信息
     * @return 0成功 -1失败
    */
    int send_json(rapidjson::Document* doc,
        std::stack<Envelope>* enves = nullptr) const;
    /**
     * @brief 发送json数据封包
     * @param msgid msgid
     * @param data josn对象
     * @param enves 路由信息
     * @return 0成功 -1失败
    */
    int send_json(int msgid, rapidjson::Value* data = nullptr,
        std::stack<Envelope>* enves = nullptr) const;

    /**
     * @brief 发送服务器通用错误信息
     * @param code 错误码
     * @param desc 错误描述
     * @param enves 路由信息
     * @return 0成功 -1失败
    */
    int send_errcode(protocc::errnum code, const std::string& desc,
        std::stack<Envelope>* enves = nullptr);

protected:
    /**
     * @brief 自定义数据处理
     * @param original_data 
     * @param datalen 
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int native_handle(const char* original_data, size_t datalen);
    /**
     * @brief protobuf数据处理
     * @param original_data 
     * @param datalen 
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int protobuf_handle(const char* original_data, size_t datalen);
    /**
     * @brief json数据处理
     * @param original_data 
     * @param datalen 
     * @return 0成功 -1失败 失败将关闭对端连接
    */
    int json_handle(const char* original_data, size_t datalen);

    using ProtoMsgBind = std::function<int(std::shared_ptr<protocc::CommonObject> obj,
        std::shared_ptr<std::stack<Envelope>> enves)>;
    /**
     * @brief proto消息id与处理函数映射hash表
    */
    std::unordered_map<int, ProtoMsgBind> pmb_;

    using JsonMsgBind = std::function<int(std::shared_ptr<rapidjson::Document> doc,
        std::shared_ptr<std::stack<Envelope>> enves)>;
    /**
     * @brief josn消息id与处理函数映射hash表 
    */
    std::unordered_map<int, JsonMsgBind> jmb_;

    using NativeMsgBind = std::function<int(const char* original_data, size_t datalen,
        std::shared_ptr<std::stack<Envelope>> enves)>;
    /**
     * @brief  自定义消息id与处理函数映射hash表
    */
    std::unordered_map<int, NativeMsgBind> nmb_;
    /**
     * @brief 协议过滤器
    */
    protocol::FilterProces* filter_ = nullptr;

private:
    /**
     * @brief 数据封包类型
    */
    ProtoData pd_ = ProtoData::NATIVE;
};
}

#endif
