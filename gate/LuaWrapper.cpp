#include "gate/luawrapper.h"
#include "gate/gameconnector.h"
#include "gate/gateclient.h"
#include "gate/gateserver.h"
#include "gate/loginconnector.h"
#include "gate/worldconnector.h"
#include "shynet/utils/iniconfig.h"

namespace gate {
LuaWrapper::LuaWrapper()
{
}

LuaWrapper::~LuaWrapper()
{
}

void LuaWrapper::init(kaguya::State& state)
{
    frmpub::FrmLuaWrapper::init(state);

    state["GateServer_CPP"].setClass(kaguya::UserdataMetatable<GateServer, net::ListenEvent>());

    state["GateClient_CPP"].setClass(kaguya::UserdataMetatable<GateClient, frmpub::Client>());

    state["WorldConnector_CPP"].setClass(kaguya::UserdataMetatable<WorldConnector, frmpub::Connector>());

    state["LoginConnector_CPP"].setClass(kaguya::UserdataMetatable<LoginConnector, frmpub::Connector>());

    state["GameConnector_CPP"].setClass(kaguya::UserdataMetatable<GameConnector, frmpub::Connector>());

    //载入lua文件
    shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
    std::string luafile = ini.get<const char*, std::string>("gate", "luafile", "lua/gate/gate_main.lua");
    state.dofile(luafile);
}
}
