#ifndef FRMPUB_LUAFOLDERTASK_H
#define FRMPUB_LUAFOLDERTASK_H

#include "shynet/task/notifytask.h"

namespace frmpub {
/**
 * @brief lua文件变化任务
*/
class LuaFolderTask : public shynet::task::NotifyTask {
public:
    /**
     * @brief 构造
     * @param path 监控lua路径(可以是相对或者绝对路径)
     * @param ischild 是否监控子目录
     * @param mask 监控掩码
    */
    LuaFolderTask(std::string path, bool ischild = true, uint32_t mask = IN_MODIFY | IN_CREATE);
    ~LuaFolderTask() = default;

    /**
    * @brief 监控目录发生事件回调
    * @param path 监控目录
    * @param mask 监控掩码
    * @return 0成功 -1失败
    */
    int notify_event(std::string path, uint32_t mask) override;
};
}

#endif
