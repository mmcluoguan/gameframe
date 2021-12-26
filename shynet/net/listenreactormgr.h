#ifndef SHYNET_NET_LISTENREACTORMGR_H
#define SHYNET_NET_LISTENREACTORMGR_H

#include "shynet/net/listenevent.h"
#include "shynet/pool/threadpool.h"
#include <unordered_map>

namespace shynet {
namespace net {
    /**
     * @brief 侦听的服务器地址管理器
    */
    class ListenReactorMgr final : public Nocopy {
        friend class utils::Singleton<ListenReactorMgr>;

        /**
         * @brief 构造
        */
        ListenReactorMgr() = default;

    public:
        /**
         * @brief 类型名称
        */
        static constexpr const char* kClassname = "ListenReactorMgr";
        ~ListenReactorMgr() = default;

        /**
         * @brief 添加新的侦听的服务器地址发生有新连接处理器
         * @param v 侦听的服务器地址发生有新连接处理器
         * @return 服务器id
        */
        int add(std::shared_ptr<ListenEvent> v);
        /**
         * @brief 移除侦听的服务器地址发生有新连接处理器
         * @param k 服务器id
         * @return ture成功,false失败
        */
        bool remove(int k);

        /**
         * @brief 查找侦听的服务器地址发生有新连接处理器
         * @param k 服务器id
         * @return 找到返回侦听的服务器地址发生有新连接处理器,否则nullptr
        */
        std::shared_ptr<ListenEvent> find(int k);

    private:
        /**
         * @brief 通知侦听的服务器地址线程有新的服务器地址需要侦听
         * @param data 服务器id的地址
         * @param len 服务器id的地址长度
        */
        void notify(const void* data, size_t len);

    private:
        /**
         * @brief 互斥体
        */
        std::mutex les_mutex_;
        /*
		* 有新连接处理器的hash表
		*/
        std::unordered_map<int, std::shared_ptr<ListenEvent>> les_;
    };
}
}

#endif
