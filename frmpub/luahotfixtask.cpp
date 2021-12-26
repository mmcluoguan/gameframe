#include "frmpub/luahotfixtask.h"
#include "frmpub/luaremotedebug.h"
#include "shynet/thread/luathread.h"
#include "shynet/utils/singleton.h"
#include "shynet/utils/stringop.h"
#include <chrono>
#include <thread>

namespace frmpub {
LuaHotFixTask::LuaHotFixTask(std::string filepath)
{
    filepath_ = filepath;
}

int LuaHotFixTask::run(thread::Thread* tif)
{
    thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
    kaguya::State& state = *(lua->luaState());
    std::string suffix_str = filepath_.substr(filepath_.find_last_of('.') + 1);
    std::string name_str = filepath_.substr(0, filepath_.find_last_of('.'));
    if (suffix_str.compare("pb") == 0) {
        shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);

        std::string script = "local pb = require('pb');pb.clear('%s');pb.loadfile('%s')";
        script = shynet::utils::stringop::str_format(script, filepath_.c_str(), filepath_.c_str());
        state(script);

        shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
        LOG_DEBUG << filepath_ << " 重新加载";
    } else if (suffix_str.compare("lua") == 0 || suffix_str.compare("so") == 0) {
        shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);

        std::string script = "require('%s');local hotfix=require('hotfix');hotfix:reloadmodule('%s')";
        script = shynet::utils::stringop::str_format(script, name_str.c_str(), name_str.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); //休眠1毫秒，为了让修改的文件加载时是最新的
        state(script);

        shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
        LOG_DEBUG << filepath_ << " 重新加载";
    }
    return 0;
}

}
