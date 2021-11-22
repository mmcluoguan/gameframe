#ifndef WORLD_LUAWRAPPER_H
#define WORLD_LUAWRAPPER_H

#include "frmpub/frmluawrapper.h"

namespace world {
class LuaWrapper : public frmpub::FrmLuaWrapper {
public:
    LuaWrapper();
    ~LuaWrapper();

    virtual void init(kaguya::State& state) override;
};
}

#endif
