#ifndef SHYNET_PROTOCOL_TCPIP_H
#define SHYNET_PROTOCOL_TCPIP_H

#include "shynet/basic.h"
#include "shynet/events/streambuff.h"

namespace shynet {
namespace protocol {
    /**
     * @brief 协议过滤器
    */
    class FilterProces;

    /**
     * @brief 自定义协议处理器
    */
    class Tcpip : public Nocopy {
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
         * @brief 构造
         * @param filter 协议过滤器
        */
        explicit Tcpip(FilterProces* filter);
        ~Tcpip() = default;

        /**
         * @brief 解析自定义数据流协议
         * @return 管理io缓冲数据处理结果
        */
        net::InputResult process();

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
         * @brief 缓存不完整的数据包
        */
        std::unique_ptr<char[]> total_original_data_;
        /**
         * @brief 缓存不完整的数据包当前偏移位置
        */
        size_t total_postion_ = 0;
    };
}
}

#endif
