#ifndef SHYNET_NET_CONNECTREACTORMGR_H
#define SHYNET_NET_CONNECTREACTORMGR_H

#include "shynet/net/connectevent.h"
#include "shynet/utils/singleton.h"
#include <unordered_map>

namespace shynet {
namespace net {
    /**
     * @brief 连接的服务器地址管理器
    */
    class ConnectReactorMgr final : public Nocopy {
        friend class utils::Singleton<ConnectReactorMgr>;
        ConnectReactorMgr() = default;

    public:
        /**
         * @brief 类型名称
        */
        static constexpr const char* kClassname = "ConnectReactorMgr";
        ~ConnectReactorMgr() = default;

        /**
         * @brief 添加新的接收服务器数据处理器
         * @param v 新的接收服务器数据处理器
         * @return 客户端id
        */
        int add(std::shared_ptr<ConnectEvent> v);
        /**
         * @brief 移除接收服务器数据处理器
         * @param k 客户端id
         * @return ture成功,false失败
        */
        bool remove(int k);

        /**
         * @brief 查找接收服务器数据处理器
         * @param k 客户端id
         * @return 找到返回接收服务器数据处理器,否则nullptr
        */
        std::shared_ptr<ConnectEvent> find(int k);
        /**
         * @brief 查找接收服务器数据处理器
         * @param ip 地址字符串
         * @param port 端口
         * @return 找到返回接收服务器数据处理器,否则nullptr
        */
        std::shared_ptr<ConnectEvent> find(const std::string ip, unsigned short port);

    private:
        /**
         * @brief 通知连接服务器的线程有新的连接请求
         * @param data 客户端id的地址
         * @param len 客户端id地址大小
        */
        void notify(const void* data, size_t len);

    private:
        /**
         * @brief 互斥体
        */
        std::mutex cnt_mutex_;
        /**
         * @brief 接收服务器数据处理器的hash表
        */
        std::unordered_map<int, std::shared_ptr<ConnectEvent>> cnts_;
    };
}
}

#endif
