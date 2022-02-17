#ifndef SHYNET_PROTOCOL_WEBSOCKET_H
#define SHYNET_PROTOCOL_WEBSOCKET_H

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
     * @brief websocket协议处理器
    */
    class WebSocket : public Nocopy {
    public:
        /**
         * @brief 帧类型
        */
        enum class FrameType {
            /**
             * @brief 不完整的数据包
            */
            Continuation,
            /**
             * @brief 文本帧
            */
            Text,
            /**
             * @brief 二机制数据
            */
            Binary,
            /**
             * @brief 断开连接帧
            */
            Close = 8,
            /**
             * @brief ping帧
            */
            Ping = 9,
            /**
             * @brief pong帧
            */
            Pong = 10,
        };

        /**
         * @brief 状态阶段
        */
        enum class Status {
            /**
             * @brief 未连接阶段
            */
            Unconnect = 0,
            /**
             * @brief 握手阶段
            */
            Handsharked,
        };

        /**
         * @brief 构造
         * @param filter 协议过滤器
        */
        explicit WebSocket(FilterProces* filter);
        ~WebSocket() = default;

        /**
         * @brief 解析websocket数据流协议
         * @return 管理io缓冲数据处理结果
        */
        net::InputResult process(std::function<void(std::unique_ptr<char[]>, size_t)> cb);

        /**
         * @brief 解析请求websocket数据流
         * @param inputbuffer io缓冲
         * @param restore 用于数据包不完整时恢复原始io缓冲
         * @return 管理io缓冲数据处理结果
        */
        net::InputResult process_requset(std::shared_ptr<events::Streambuff> inputbuffer,
            std::shared_ptr<events::Streambuff> restore);
        /**
         * @brief 解析响应websocket数据流
         * @param inputbuffer io缓冲
         * @param restore 用于数据包不完整时恢复原始io缓冲
         * @return 管理io缓冲数据处理结果
        */
        net::InputResult process_responses(std::shared_ptr<events::Streambuff> inputbuffer,
            std::shared_ptr<events::Streambuff> restore);

        /**
         * @brief 解析websocket数据流协议
         * @param inputbuffer io缓冲
         * @param restore 用于数据包不完整时恢复原始io缓冲
         * @return 管理io缓冲数据处理结果
        */
        net::InputResult process_data(std::shared_ptr<events::Streambuff> inputbuffer,
            std::shared_ptr<events::Streambuff> restore,
            std::function<void(std::unique_ptr<char[]>, size_t)> cb);

        /**
         * @brief http协议升级
         * @param key 握手字符串
         * @return 0成功,-1失败
        */
        int upgrade(const char* key);
        /**
         * @brief 验证服务器发送的握手字符串与客户端自己生成的握手字符串是否相等
         * @param key 服务器发送的握手字符串
         * @return 0成功,-1失败
        */
        int verify(const char* key);
        /**
         * @brief 客户端请求与服务器握手
         * @return 0成功,-1失败
        */
        int request_handshake();

        /**
         * @brief 发送数据
         * @param data 指向数据的指针
         * @param len 指向数据的指针大小
         * @param op 帧类型
         * @return 0成功,-1失败 
        */
        int send1(const void* data, size_t len, FrameType op) const;
        /**
         * @brief 超过最大单包数据大小限制,分包发送
         * @param data 指向数据的指针
         * @param len 指向数据的指针大小
         * @param op 帧类型
         * @return 0成功,-1失败 
        */
        int send(const void* data, size_t len, FrameType op) const;
        /**
         * @brief 超过最大单包数据大小限制,分包发送
         * @param data 数据
         * @param op 帧类型
         * @return 0成功,-1失败 
        */
        int send(const std::string& data, FrameType op) const;

    private:
        /**
         * @brief 协议过滤器
        */
        FilterProces* filter_;
        /**
         * @brief http请求处理器
        */
        Request requset_;
        /**
         * @brief http响应处理器
        */
        Responses responses_;
        /**
         * @brief 握手字符串
        */
        const char* magic_key_ { "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" };
        /**
         * @brief 状态阶段
        */
        Status status_ = Status::Unconnect;
        /**
         * @brief 客户端生成的握手key
        */
        std::string request_key_;
        /**
         * @brief 缓存不完整的数据包
        */
        std::shared_ptr<events::Streambuff> total_original_data_;
    };
}
}

#endif
