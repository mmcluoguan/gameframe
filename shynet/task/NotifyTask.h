#ifndef SHYNET_TASK_NOTIFYTASK_H
#define SHYNET_TASK_NOTIFYTASK_H

#include "shynet/basic.h"
#include <sys/inotify.h>
#include <unordered_map>

namespace shynet {
namespace task {
    /*
		* 目录监控任务
		*/
    class NotifyTask : public Nocopy {
    public:
        /*
			* path 监控路径(可以是相对或者绝对路径)
			* ischild 是否监控子目录
			* mask 监控掩码
			*/
        explicit NotifyTask(std::string path, bool ischild = false, uint32_t mask = IN_ALL_EVENTS);
        ~NotifyTask();

        /*
			* 监控目录发生事件
			*/
        virtual int notify_event(std::string path, uint32_t mask) = 0;

        /*
			* 监控路径(可以是相对或者绝对路径)
			*/
        std::string path() const
        {
            return path_;
        }

        /*
			* 是否监控子目录
			*/
        bool ischild() const
        {
            return ischild_;
        }

        /*
			* 监控掩码
			*/
        uint32_t mask() const
        {
            return mask_;
        }

        /*
			* 监控任务fd关联的目录
			*/
        std::unordered_map<int, std::string>& fdptahs()
        {
            return fdptahs_;
        }

    private:
        std::string path_;
        bool ischild_ = false;
        uint32_t mask_;
        std::unordered_map<int, std::string> fdptahs_;
    };
}
}

#endif
