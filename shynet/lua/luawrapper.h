#ifndef SHYNET_LUA_LUAWRAPPER_H
#define SHYNET_LUA_LUAWRAPPER_H

#include "3rd/kaguya/kaguya.hpp"

namespace shynet {
namespace lua {
    /**
     * @brief lua包装器
    */
    class LuaWrapper {
    public:
        /**
         * @brief 构造
        */
        LuaWrapper() = default;
        virtual ~LuaWrapper() = default;

        /**
         * @brief 初始化lua栈,导出c++到lua
         * @param state lua栈
        */
        virtual void init(kaguya::State& state);
    };
}
}

#endif
