#include "gate/LuaWrapper.h"
#include "gate/GameConnector.h"
#include "gate/GateClient.h"
#include "gate/GateServer.h"
#include "gate/LoginConnector.h"
#include "gate/WorldConnector.h"
#include "shynet/utils/IniConfig.h"

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
