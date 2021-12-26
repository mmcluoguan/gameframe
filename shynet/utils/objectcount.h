#ifndef SHYNET_UTILS_OBJECTCOUNT_H
#define SHYNET_UTILS_OBJECTCOUNT_H

#include <cstddef>

namespace shynet {
namespace utils {
    /**
     * @brief 对象实例化计数
     * @tparam T 对象类型
    */
    template <class T>
    class ObjectCount {
    protected:
        /**
         * @brief 构造
        */
        ObjectCount()
        {
            ++count_;
        }

        /**
         * @brief 复制构造
         * @param  
        */
        ObjectCount(const ObjectCount<T>&)
        {
            ++count_;
        }

        /**
         * @brief 移动构造
         * @param  
        */
        ObjectCount(ObjectCount<T>&&)
        {
            ++count_;
        }

        ~ObjectCount()
        {
            --count_;
        }

    public:
        /**
         * @brief 获取已创建的对象数量
         * @return 已创建的对象数量
        */
        static std::size_t live()
        {
            return count_;
        }

    private:
        /**
         * @brief 已创建的对象数量
        */
        inline static std::size_t count_ = 0;
    };
}
}

#endif
