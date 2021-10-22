#include "game/luawrapper.h"
#include "game/dbconnector.h"
#include "game/gameclient.h"
#include "game/gameserver.h"
#include "game/worldconnector.h"
#include "shynet/utils/idworker.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/singleton.h"

namespace game {
LuaWrapper::LuaWrapper()
{
}

LuaWrapper::~LuaWrapper()
{
}

void LuaWrapper::init(kaguya::State& state)
{
    frmpub::FrmLuaWrapper::init(state);

    state["newid"] = kaguya::function([]() {
        return shynet::utils::Singleton<shynet::utils::IdWorker>::get_instance().getid();
    });

    state["GameServer_CPP"].setClass(kaguya::UserdataMetatable<GameServer, net::ListenEvent>());

    state["GameClient_CPP"].setClass(kaguya::UserdataMetatable<GameClient, frmpub::Client>());

    state["DbConnector_CPP"].setClass(kaguya::UserdataMetatable<DbConnector, frmpub::Connector>());

    state["WorldConnector_CPP"].setClass(kaguya::UserdataMetatable<WorldConnector, frmpub::Connector>());

    //载入lua文件
    shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
    std::string luafile = ini.get<const char*, std::string>("game", "luafile", "lua/game/game_main.lua");
    state.dofile(luafile);
}
}
