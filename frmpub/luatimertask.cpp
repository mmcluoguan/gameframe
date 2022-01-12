#include "frmpub/luatimertask.h"
#include "3rd/fmt/format.h"
#include "frmpub/luaremotedebug.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/thread/luathread.h"
#include "shynet/utils/elapsed.h"

namespace frmpub {
LuaTimerMgr::LuaTimerTask::LuaTimerTask(int timerid, std::string funname)
{
    timerid_ = timerid;
    funname_ = funname;
}

int LuaTimerMgr::LuaTimerTask::run(thread::Thread* tif)
{
    thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
    kaguya::State& state = *(lua->luaState());
    auto cb = [&]() {
        shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);
        state["onTimerOut"].call<void>(timerid_);
        shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
    };
#ifdef USE_DEBUG
    std::string str("lua线程计时单任务执行 ");
    str.append(funname_);
    shynet::utils::elapsed(str.c_str());
    cb();
#else
    cb();
#endif
    return 0;
}

LuaTimerMgr::LuaTimer::LuaTimer(const timeval val, bool repeat, std::string funname)
    : net::TimerEvent(val, EV_TIMEOUT | EV_PERSIST)
{
    repeat_ = repeat;
    funname_ = funname;
}

void LuaTimerMgr::LuaTimer::timeout()
{
    std::shared_ptr<LuaTimerTask> task = std::make_shared<LuaTimerTask>(timerid());
    shynet::utils::Singleton<shynet::lua::LuaEngine>::instance().append(task);
    if (repeat_ == false) {
        shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
    }
}
int LuaTimerMgr::add(const timeval val, bool repeat, std::string funname)
{
    std::shared_ptr<LuaTimer> timer = std::make_shared<LuaTimer>(val, repeat, funname);
    int id = shynet::utils::Singleton<net::TimerReactorMgr>::instance().add(timer);
    timerids_.insert(id);
    return id;
}

void LuaTimerMgr::remove(int timerid)
{
    timerids_.erase(timerid);
    shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid);
}
}
