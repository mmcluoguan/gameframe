#ifndef SHYNET_UTILS_SINGLETON_H
#define SHYNET_UTILS_SINGLETON_H

#include "shynet/utils/logger.h"
#include <memory>
#include <mutex>

namespace shynet {
namespace utils {
    /**
     * @brief 单例模式
     * @tparam T 类型
    */
    template <class T>
    class Singleton : public Nocopy {
    public:
        Singleton() = default;
        ~Singleton() = default;

        /**
         * @brief 获取实例
         * @tparam ...Args 实例化类型构造函数参数类型 
         * @param ...args 实例化类型构造函数参数
         * @return 实例
        */
        template <typename... Args>
        static T& instance(Args&&... args)
        {
            std::call_once(onceflag_,
                std::forward<void(Args && ...)>(&Singleton<T>::init),
                std::forward<Args>(args)...);
            return *instance_;
        };

        /**
         * @brief 获取实例,
         这个接口保证单例一定创建完成并且创建的单例没有被释放,否则抛出异常
         目的用于发现单例的创建是否在预期内
         * @return 实例
        */
        static T& get_instance()
        {
            if (instance_ == nullptr) {
                std::ostringstream err;
                err << T::kClassname << " 单例没有初始化或者已被释放";
                THROW_EXCEPTION(err.str());
            }
            return *instance_;
        }

    private:
        /**
         * @brief 调用单例构造函数
         * @tparam ...Args 实例化类型构造函数参数类型 
         * @param ...args 实例化类型构造函数参数
        */
        template <typename... Args>
        static void init(Args&&... args)
        {
            instance_.reset(new T(std::forward<Args>(args)...));
        }

    private:
        /**
         * @brief 保证多线程情况下单例也只实例化一次
        */
        inline static std::once_flag onceflag_;
        /**
         * @brief 单例对象指针
        */
        inline static std::unique_ptr<T> instance_;
    };
}
}

#endif
