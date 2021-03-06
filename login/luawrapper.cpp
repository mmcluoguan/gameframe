#include "login/luawrapper.h"
#include "frmpub/luaremotedebug.h"
#include "login/dbconnector.h"
#include "login/loginclient.h"
#include "login/loginclientmgr.h"
#include "login/loginserver.h"
#include "login/worldconnector.h"
#include "shynet/utils/iniconfig.h"

//配置参数
extern const char* g_conf_node;

namespace login {

void LuaWrapper::init(kaguya::State& state)
{
    frmpub::FrmLuaWrapper::init(state);

    state["LoginServer_CPP"].setClass(kaguya::UserdataMetatable<LoginServer, net::ListenEvent>());

    state["LoginClient_CPP"].setClass(kaguya::UserdataMetatable<LoginClient, frmpub::Client>());

    state["DbConnector_CPP"].setClass(kaguya::UserdataMetatable<DbConnector, frmpub::Connector>());

    state["WorldConnector_CPP"].setClass(kaguya::UserdataMetatable<WorldConnector, frmpub::Connector>());

    //是否开启调试模式
    shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
    std::string luadebugip = ini.get<std::string>(g_conf_node, "luadebugip");
    if (!luadebugip.empty()) {
        shynet::utils::Singleton<frmpub::LuaRemoteDebug>::instance().enable(luadebugip).start(state);
    }

    //载入lua文件
    std::string luafile = ini.get<std::string>(g_conf_node, "luafile");
    state.dofile(luafile);

    shynet::utils::Singleton<frmpub::LuaRemoteDebug>::instance().stop(state);
}
}
