#ifndef GAME_LUAWRAPPER_H
#define GAME_LUAWRAPPER_H

#include "frmpub/frmluawrapper.h"

namespace game {
class LuaWrapper : public frmpub::FrmLuaWrapper {
public:
    LuaWrapper();
    ~LuaWrapper();

    virtual void init(kaguya::State& state) override;
};
}

#endif
