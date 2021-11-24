#include "game/luawrapper.h"
#include "frmpub/luaremotedebug.h"
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

    //是否开启调试模式
    shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
    std::string luadebugip = ini.get<std::string>("game", "luadebugip");
    if (!luadebugip.empty()) {
        shynet::utils::Singleton<frmpub::LuaRemoteDebug>::instance().enable(luadebugip).start(state);
    }

    //载入lua文件
    std::string luafile = ini.get<std::string>("game", "luafile");
    state.dofile(luafile);

    shynet::utils::Singleton<frmpub::LuaRemoteDebug>::instance().stop(state);
}
}
