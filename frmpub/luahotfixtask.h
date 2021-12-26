#ifndef FRMPUB_LUAHOTFIXTASK_H
#define FRMPUB_LUAHOTFIXTASK_H

#include "shynet/lua/luatask.h"
#include "shynet/thread/thread.h"

namespace frmpub {
/**
 * @brief lua热更新任务
*/
class LuaHotFixTask : public shynet::luatask::LuaTask {
public:
    /**
     * @brief 构造
     * @param filepath 发生变化的lua文件
    */
    LuaHotFixTask(std::string filepath);
    ~LuaHotFixTask() = default;

    /**
    * @brief 运行lua热更新任务
    * @param tif 线程信息
    * @return 0成功,-1失败,返回失败lua线程会结束
    */
    int run(thread::Thread* tif) override;

private:
    /**
     * @brief 发生变化的lua文件
    */
    std::string filepath_;
};
}

#endif
