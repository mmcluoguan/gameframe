#include "frmpub/luaremotedebug.h"
#include "shynet/utils/stringop.h"

namespace frmpub {
LuaRemoteDebug& LuaRemoteDebug::init(const std::string debugip)
{
    debugip_ = debugip;
    isinit_ = true;
    return *this;
}

LuaRemoteDebug& LuaRemoteDebug::start(kaguya::State& state)
{
    if (isinit_) {
        state(shynet::utils::StringOp::str_format("require('mobdebug').start('%s')", debugip_.c_str()));
    }
    return *this;
}

LuaRemoteDebug& LuaRemoteDebug::stop(kaguya::State& state)
{
    if (isinit_) {
        state("require('mobdebug').done()");
    }
    return *this;
}
}
