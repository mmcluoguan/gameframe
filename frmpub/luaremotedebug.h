#ifndef FRMPUB_LUAREMOTEDEBUG_H
#define FRMPUB_LUAREMOTEDEBUG_H

#include "3rd/kaguya/kaguya.hpp"
#include "shynet/utils/singleton.h"

namespace frmpub {
/**
 * @brief lua远程调试器
*/
class LuaRemoteDebug final : public shynet::Nocopy {
    friend class shynet::utils::Singleton<LuaRemoteDebug>;

    LuaRemoteDebug() = default;

public:
    /**
     * @brief 启用调试
     * @param debugip 远程服务器地址
     * @return 
    */
    LuaRemoteDebug& enable(const std::string& debugip);

    /*
	* @brief 禁用调试
	*/
    void disenable();

    /*
	* @brief 开始调试
	*/
    LuaRemoteDebug& start(kaguya::State& state);

    /*
	* @brief 调试结束
	*/
    LuaRemoteDebug& stop(kaguya::State& state);

private:
    /**
     * @brief 远程服务器地址
    */
    std::string debugip_;
    /**
     * @brief 是否初始化
    */
    bool isinit_ = false;
    /**
     * @brief 是否开始调试
    */
    bool isstart_ = false;
};
}

#endif
