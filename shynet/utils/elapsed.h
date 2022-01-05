#ifndef SHYNET_UTILS_ELAPSED_H
#define SHYNET_UTILS_ELAPSED_H

#include "shynet/utils/logger.h"
#include "shynet/utils/stuff.h"
#include <string_view>

namespace shynet {
namespace utils {
    class elapsed final {
    public:
        elapsed(std::string_view view)
        {
            beg_ = stuff::tick_msec();
            tag_ = view;
        }
        ~elapsed()
        {
            uint64_t elp = stuff::tick_msec() - beg_;
            if (elp >= 10) {
                LOG_WARN << "单任务执行时间超过10ms 执行时间:" << elp
                         << " 附带信息:" << tag_;
            }
        }

    private:
        std::string_view tag_;
        uint64_t beg_;
    };
}
}

#endif
