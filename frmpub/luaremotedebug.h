#ifndef FRMPUB_LUAREMOTEDEBUG_H
#define FRMPUB_LUAREMOTEDEBUG_H

#include "shynet/utils/singleton.h"
#include <kaguya/kaguya.hpp>

namespace frmpub {
/*
	* lua远程调试
	*/
class LuaRemoteDebug final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<LuaRemoteDebug>;

    LuaRemoteDebug() = default;

public:
    /*
		* 启用调试
		*/
    LuaRemoteDebug& enable(const std::string& debugip);

    /*
		* 禁止调试
		*/
    void disenable();

    /*
		* 开始调试
		*/
    LuaRemoteDebug& start(kaguya::State& state);

    /*
		* 调试结束
		*/
    LuaRemoteDebug& stop(kaguya::State& state);

private:
    std::string debugip_;
    bool isinit_ = false;
    bool isstart_ = false;
};
}

#endif
