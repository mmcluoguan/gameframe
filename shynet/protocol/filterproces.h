#ifndef SHYNET_PROTOCOL_FILTERPROCES_H
#define SHYNET_PROTOCOL_FILTERPROCES_H

#include "shynet/events/eventbuffer.h"
#include "shynet/net/ipaddress.h"
#include "shynet/protocol/http.h"
#include "shynet/protocol/tcpip.h"
#include "shynet/protocol/websocket.h"

namespace shynet {
namespace protocol {
    /**
     * @brief 协议过滤器
    */
    class FilterProces : public Nocopy {
    public:
        /**
         * @brief 身份标识
        */
        enum class Identity {
            /**
             * @brief 接收身份
            */
            ACCEPTOR,
            /**
             * @brief 连接身份
            */
            CONNECTOR,
        };

        /**
         * @brief 协议类型
        */
        enum class ProtoType {
            /**
             * @brief 自定义协议 {/r/n + uint32 + len}
            */
            SHY,
            /**
             * @brief http协议
            */
            HTTP,
            /**
             * @brief websocket协议
            */
            WEBSOCKET,
        };

        /**
         * @brief 构造
         * @param iobuf 管理io缓冲
         * @param pt 协议类型
         * @param ident 身份标识
        */
        FilterProces(std::shared_ptr<events::EventBuffer> iobuf, ProtoType pt, Identity ident);
        ~FilterProces() = default;

        /**
         * @brief 连接服务器完成回调
        */
        virtual void complete() {};

        /**
         * @brief 消息数据封包处理回调
         * @param original_data 指向存储数据的指针
         * @param datalen  指向存储数据的大小
         * @return 0成功 -1失败,失败将关闭对端连接
        */
        virtual int message_handle(char* original_data, size_t datalen) = 0;

        /**
         * @brief 发送数据
         * @param data 指向数据的指针
         * @param datalen 指向数据的指针大小
         * @return 0成功 -1失败
        */
        int send(const char* data, size_t datalen) const;
        /**
         * @brief 发送数据
         * @param data 数据
         * @return 0成功 -1失败
        */
        int send(const std::string& data) const;

        /**
         * @brief 发送ping包
         * @return  0成功 -1失败
        */
        int ping() const;

        /**
         * @brief 获取协议类型
         * @return 协议类型
        */
        ProtoType pt() const { return pt_; }

        /**
         * @brief 获取身份标识
         * @return 身份标识
        */
        Identity ident() const;

        /**
         * @brief 单包接收缓冲区最大值
        */
        const size_t max_reve_buf_size = 4 * 1024 * 1024;
        /**
         * @brief 单包最大值(超过时发送分包)
        */
        const size_t max_single_buf_size = 1024 * 64;

        /**
         * @brief 获取管理io缓冲
         * @param iobuf 管理io缓冲 
        */
        std::shared_ptr<events::EventBuffer> iobuf() const
        {
            return iobuf_;
        }
        /**
         * @brief 设置管理io缓冲
         * @param iobuf 管理io缓冲 
        */
        void set_iobuf(std::shared_ptr<events::EventBuffer> iobuf)
        {
            iobuf_ = iobuf;
        }

    protected:
        /**
         * @brief 解析数据流协议
         * @return 管理io缓冲数据处理结果
        */
        net::InputResult process();

        /**
         * @brief 处理websocket握手
         * @return 0成功 -1失败
        */
        int request_handshake() const;

    private:
        /**
         * @brief 管理io缓冲
        */
        std::shared_ptr<events::EventBuffer> iobuf_;
        /**
         * @brief 协议类型 
        */
        ProtoType pt_ = ProtoType::SHY;
        /**
         * @brief 身份标识
        */
        Identity ident_;
        /**
         * @brief 自定义协议处理器
        */
        std::unique_ptr<Tcpip> tcpip_;
        /**
         * @brief http协议处理器
        */
        std::unique_ptr<Http> http_;
        /**
         * @brief websocket协议处理器
        */
        std::unique_ptr<WebSocket> websocket_;
    };
}
}

#endif
