#include "dbvisit/luawrapper.h"
#include "dbvisit/dbclient.h"
#include "dbvisit/dbclientmgr.h"
#include "dbvisit/dbserver.h"
#include "frmpub/luaremotedebug.h"
#include "shynet/utils/iniconfig.h"
#include "shynet/utils/singleton.h"

//配置参数
extern const char* g_conf_node;

namespace dbvisit {

void LuaWrapper::init(kaguya::State& state)
{
    frmpub::FrmLuaWrapper::init(state);

    state["DbServer_CPP"].setClass(kaguya::UserdataMetatable<DbServer, net::ListenEvent>());

    state["DbClient_CPP"].setClass(kaguya::UserdataMetatable<DbClient, frmpub::Client>());

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
