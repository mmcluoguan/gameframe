#ifndef FRMPUB_FRMLUAWRAPPER_H
#define FRMPUB_FRMLUAWRAPPER_H

#include "shynet/lua/LuaWrapper.h"

namespace frmpub {
/*
	* 导出c++到lua
	*/
class FrmLuaWrapper : public shynet::lua::LuaWrapper {
public:
    FrmLuaWrapper();
    ~FrmLuaWrapper();

    virtual void init(kaguya::State& state) override;
};
}

#endif
