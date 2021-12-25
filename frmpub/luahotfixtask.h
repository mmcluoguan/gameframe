#ifndef FRMPUB_LUAHOTFIXTASK_H
#define FRMPUB_LUAHOTFIXTASK_H

#include "shynet/lua/luatask.h"
#include "shynet/thread/thread.h"

namespace frmpub {
/*
	* lua热更新任务
	*/
class LuaHotFixTask : public shynet::luatask::LuaTask {
public:
    LuaHotFixTask(std::string filepath);
    ~LuaHotFixTask();

    virtual int run(thread::Thread* tif);

private:
    std::string filepath_;
};
}

#endif
