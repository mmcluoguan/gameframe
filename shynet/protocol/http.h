#ifndef SHYNET_PROTOCOL_HTTP_H
#define SHYNET_PROTOCOL_HTTP_H

#include "shynet/events/streambuff.h"
#include "shynet/protocol/request.h"
#include "shynet/protocol/responses.h"

namespace shynet {
namespace protocol {
    /**
     * @brief 协议过滤器
    */
    class FilterProces;

    /**
     * @brief http协议处理器
    */
    class Http : public Nocopy {
    public:
        /**
         * @brief 构造
         * @param filter 协议过滤器
        */
        explicit Http(FilterProces* filter);
        ~Http() = default;

        /**
         * @brief 解析http数据流协议
         * @return 管理io缓冲数据处理结果
        */
        net::InputResult process();
        /**
         * @brief 解析请求http数据流
         * @param inputbuffer io缓冲
         * @param restore 用于数据包不完整时恢复原始io缓冲
         * @return 管理io缓冲数据处理结果
        */
        net::InputResult process_requset(std::shared_ptr<events::Streambuff> inputbuffer,
            std::shared_ptr<events::Streambuff> restore);

        /**
         * @brief 解析响应http数据流
         * @param inputbuffer io缓冲
         * @param restore 用于数据包不完整时恢复原始io缓冲
         * @return 管理io缓冲数据处理结果
        */
        net::InputResult process_responses(std::shared_ptr<events::Streambuff> inputbuffer,
            std::shared_ptr<events::Streambuff> restore);

        /**
         * @brief 发送响应http数据
         * @param data 指向数据的指针
         * @param len 指向数据的指针大小
         * @param server_name 服务器名称
         * @param content_type mime类型
         * @param server_heads http头
         * @param status_code 状态码
         * @param status_desc 状态描述
         * @param version http版本
         * @return 0成功,-1失败
        */
        int send_responses(const void* data, size_t len,
            std::string server_name = "",
            std::string content_type = "application/json",
            std::map<std::string, std::string>* server_heads = nullptr,
            int status_code = 200,
            std::string status_desc = "OK",
            std::string version = "HTTP/1.1") const;

        /**
         * @brief 发送响应http数据
         * @param data 数据
         * @param server_name 服务器名称
         * @param content_type mime类型 
         * @param server_heads http头 
         * @param status_code 状态码
         * @param status_desc 状态描述
         * @param version http版本
         * @return 0成功,-1失败
        */
        int send_responses(const std::string& data,
            std::string server_name = "",
            std::string content_type = "application/json",
            std::map<std::string, std::string>* server_heads = nullptr,
            int status_code = 200,
            std::string status_desc = "OK",
            std::string version = "HTTP/1.1") const;

        /**
         * @brief 发送请求http数据
         * @param data 指向数据的指针
         * @param len 指向数据的指针大小
         * @param host 主机名称
         * @param path url路径
         * @param version http版本
         * @return 0成功,-1失败
        */
        int send_requset(const void* data, size_t len,
            std::string host = "",
            std::string path = "/",
            std::string version = "HTTP/1.1") const;

        /**
         * @brief 发送请求http数据
         * @param data 数据
         * @param host 主机名称
         * @param path url路径
         * @param version http版本
         * @return  0成功,-1失败
        */
        int send_requset(const std::string& data,
            std::string host = "",
            std::string path = "/",
            std::string version = "HTTP/1.1") const;

    private:
    private:
        FilterProces* filter_;
        Request requset_;
        Responses responses_;
    };
}
}

#endif
