#ifndef SHYNET_UTILS_SINGLETON_H
#define SHYNET_UTILS_SINGLETON_H

#include "shynet/utils/Logger.h"
#include <memory>
#include <mutex>

namespace shynet {
namespace utils {
    template <class T>
    class Singleton : public Nocopy {
    public:
        Singleton()
        {
        }
        ~Singleton()
        {
        }

        template <typename... Args>
        static T& instance(Args&&... args)
        {
            std::call_once(onceflag_,
                std::forward<void(Args && ...)>(&Singleton<T>::init),
                std::forward<Args>(args)...);
            return *instance_;
        };

        static T& get_instance()
        {
            if (instance_ == nullptr) {
                std::ostringstream err;
                err << T::kClassname << " 单例没有初始化";
                THROW_EXCEPTION(err.str());
            }
            return *instance_;
        }

    private:
        template <typename... Args>
        static void init(Args&&... args)
        {
            instance_.reset(new T(std::forward<Args>(args)...));
        }

    private:
        static std::once_flag onceflag_;
        static std::unique_ptr<T> instance_;
    };

    template <class T>
    std::unique_ptr<T> Singleton<T>::instance_;
    template <class T>
    std::once_flag Singleton<T>::onceflag_;
}
}

#endif
