#ifndef GATE_LUAWRAPPER_H
#define GATE_LUAWRAPPER_H

#include "frmpub/FrmLuaWrapper.h"

namespace gate {
class LuaWrapper : public frmpub::FrmLuaWrapper {
public:
    LuaWrapper();
    ~LuaWrapper();

    virtual void init(kaguya::State& state) override;
};
}

#endif
