#include "frmpub/luaremotedebug.h"
#include "shynet/utils/stringop.h"

namespace frmpub {
LuaRemoteDebug& LuaRemoteDebug::enable(const std::string& debugip)
{
    debugip_ = debugip;
    isinit_ = true;
    return *this;
}

void LuaRemoteDebug::disenable()
{
    isinit_ = false;
}

LuaRemoteDebug& LuaRemoteDebug::start(kaguya::State& state)
{
    if (isinit_ && isstart_ == false) {
        state(shynet::utils::stringop::str_format("require('mobdebug').start('%s')", debugip_.c_str()));
        isstart_ = true;
    }
    return *this;
}

LuaRemoteDebug& LuaRemoteDebug::stop(kaguya::State& state)
{
    if (isinit_ && isstart_) {
        state("require('mobdebug').done()");
        isstart_ = false;
    }
    return *this;
}
}
