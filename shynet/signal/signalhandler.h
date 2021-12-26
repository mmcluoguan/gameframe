#ifndef SHYNET_SIGNAL_SIGINTHANDLER_H
#define SHYNET_SIGNAL_SIGINTHANDLER_H

#include "shynet/events/eventhandler.h"
#include "shynet/utils/singleton.h"
#include <functional>
#include <unordered_map>

namespace shynet {
namespace signal {

    /**
     * @brief 系统信号处理器
    */
    class SignalHandler {

        friend class utils::Singleton<SignalHandler>;
        /**
         * @brief 构造
        */
        SignalHandler() = default;

    public:
        /**
         * @brief 类型名称
        */
        static constexpr const char* kClassname = "SignalHandler";
        /**
         * @brief 处理回调
        */
        using callback = std::function<void(std::shared_ptr<events::EventBase>, int)>;

        ~SignalHandler() = default;

        /**
         * @brief 添加信号处理回调
         * @param base 反应堆
         * @param sig 系统信号
         * @param cb 处理回调
        */
        void add(std::shared_ptr<events::EventBase> base,
            int sig,
            callback cb);

    private:
        /**
         * @brief 系统信号事件
        */
        class SignalEvent : public events::EventHandler {

        public:
            /**
             * @brief 构造
             * @param base 反应堆
             * @param sig 系统信号
             * @param cb 处理回调
            */
            SignalEvent(std::shared_ptr<events::EventBase> base,
                int sig, callback cb);
            ~SignalEvent() = default;

        private:
            /**
             * @brief 系统信号处理
             * @param signal 系统信号
            */
            void signal(int signal) override;
            /**
             * @brief 处理回调函数
            */
            callback cb_;
        };

    private:
        /**
         * @brief 系统信号发生对应的处理回调函数hash表
        */
        std::unordered_map<int, std::shared_ptr<SignalEvent>> sigevents_;
    };
}
}
#endif
