#ifndef SHYNET_UTILS_OBJECTCOUNT_H
#define SHYNET_UTILS_OBJECTCOUNT_H

#include <cstddef>

namespace shynet {
namespace utils {
    template <class T>
    class ObjectCount {
    protected:
        ObjectCount()
        {
            ++count_;
        }

        ObjectCount(const ObjectCount<T>&)
        {
            ++count_;
        }

        ObjectCount(ObjectCount<T>&&)
        {
            ++count_;
        }

        ~ObjectCount()
        {
            --count_;
        }

    public:
        //对象数量
        static std::size_t live()
        {
            return count_;
        }

    private:
        inline static std::size_t count_ = 0;
    };
}
}

#endif
