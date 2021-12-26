#include "frmpub/luatimertask.h"
#include "frmpub/luaremotedebug.h"
#include "shynet/lua/luaengine.h"
#include "shynet/net/timerreactormgr.h"
#include "shynet/thread/luathread.h"

namespace frmpub {
LuaTimerMgr::LuaTimerTask::LuaTimerTask(int timerid)
{
    timerid_ = timerid;
}

int LuaTimerMgr::LuaTimerTask::run(thread::Thread* tif)
{
    thread::LuaThread* lua = dynamic_cast<thread::LuaThread*>(tif);
    kaguya::State& state = *(lua->luaState());
    shynet::utils::Singleton<LuaRemoteDebug>::instance().start(state);
    state["onTimerOut"].call<void>(timerid_);
    shynet::utils::Singleton<LuaRemoteDebug>::instance().stop(state);
    return 0;
}

LuaTimerMgr::LuaTimer::LuaTimer(const timeval val, bool repeat)
    : net::TimerEvent(val, EV_TIMEOUT | EV_PERSIST)
{
    repeat_ = repeat;
}

void LuaTimerMgr::LuaTimer::timeout()
{
    std::shared_ptr<LuaTimerTask> task = std::make_shared<LuaTimerTask>(timerid());
    shynet::utils::Singleton<shynet::lua::LuaEngine>::get_instance().append(task);
    if (repeat_ == false) {
        shynet::utils::Singleton<net::TimerReactorMgr>::instance().remove(timerid());
    }
}
int LuaTimerMgr::add(const timeval val, bool repeat)
{
    std::shared_ptr<LuaTimer> timer = std::make_shared<LuaTimer>(val, repeat);
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
