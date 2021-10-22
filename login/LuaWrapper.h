#ifndef LOGIN_LUAWRAPPER_H
#define LOGIN_LUAWRAPPER_H

#include "frmpub/frmluawrapper.h"

namespace login {
class LuaWrapper : public frmpub::FrmLuaWrapper {
public:
    LuaWrapper();
    ~LuaWrapper();

    virtual void init(kaguya::State& state) override;
};
}

#endif
