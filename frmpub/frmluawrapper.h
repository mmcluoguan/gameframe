#ifndef FRMPUB_FRMLUAWRAPPER_H
#define FRMPUB_FRMLUAWRAPPER_H

#include "shynet/lua/luawrapper.h"

namespace frmpub {
/**
 * @brief lua包装器
*/
class FrmLuaWrapper : public shynet::lua::LuaWrapper {
public:
    FrmLuaWrapper() = default;
    ~FrmLuaWrapper() = default;

    /**
    * @brief 初始化lua栈,导出c++到lua
    * @param state lua栈
    */
    void init(kaguya::State& state) override;
};
}

#endif
