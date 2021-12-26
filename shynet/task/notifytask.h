#ifndef SHYNET_TASK_NOTIFYTASK_H
#define SHYNET_TASK_NOTIFYTASK_H

#include "shynet/basic.h"
#include <sys/inotify.h>
#include <unordered_map>

namespace shynet {
namespace task {
    /**
     * @brief 目录监控任务
    */
    class NotifyTask : public Nocopy {
    public:
        /**
         * @brief 构造
         * @param path 监控路径(可以是相对或者绝对路径)
         * @param ischild 是否监控子目录
         * @param mask 监控掩码
        */
        explicit NotifyTask(std::string path, bool ischild = false, uint32_t mask = IN_ALL_EVENTS);
        ~NotifyTask() = default;

        /**
         * @brief 监控目录发生事件回调
         * @param path 监控目录
         * @param mask 监控掩码
         * @return 0成功 -1失败
        */
        virtual int notify_event(std::string path, uint32_t mask) = 0;

        /**
         * @brief 获取监控路径(可以是相对或者绝对路径)
         * @return 监控路径(可以是相对或者绝对路径)
        */
        std::string path() const { return path_; }
        /**
         * @brief 获取是否监控子目录
         * @return 是否监控子目录
        */
        bool ischild() const { return ischild_; }

        /**
         * @brief 获取监控掩码
         * @return 监控掩码
        */
        uint32_t mask() const { return mask_; }

        /**
         * @brief 获取监控任务fd关联的目录的hash表
         * @return 监控任务fd关联的目录的hash表
        */
        std::unordered_map<int, std::string>& fdptahs() { return fdptahs_; }

    private:
        /**
         * @brief 监控路径(可以是相对或者绝对路径)
        */
        std::string path_;
        /**
         * @brief 是否监控子目录
        */
        bool ischild_ = false;
        /**
         * @brief 监控掩码
        */
        uint32_t mask_;
        /**
         * @brief 监控任务fd关联的目录的hash表
        */
        std::unordered_map<int, std::string> fdptahs_;
    };
}
}

#endif
