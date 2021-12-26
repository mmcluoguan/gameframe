#ifndef DBVISIT_LUAWRAPPER_H
#define DBVISIT_LUAWRAPPER_H

#include "frmpub/frmluawrapper.h"

namespace dbvisit {
/**
 * @brief lua包装器
*/
class LuaWrapper : public frmpub::FrmLuaWrapper {
public:
    /**
     * @brief 构造
    */
    LuaWrapper() = default;
    ~LuaWrapper() = default;

    /**
    * @brief 初始化lua栈,导出c++到lua
    * @param state lua栈
    */
    virtual void init(kaguya::State& state) override;
};
}

#endif
