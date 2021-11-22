#ifndef DBVISIT_LUAWRAPPER_H
#define DBVISIT_LUAWRAPPER_H

#include "frmpub/frmluawrapper.h"

namespace dbvisit {
class LuaWrapper : public frmpub::FrmLuaWrapper {
public:
    LuaWrapper();
    ~LuaWrapper();

    virtual void init(kaguya::State& state) override;
};
}

#endif
