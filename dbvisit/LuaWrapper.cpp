#include "dbvisit/LuaWrapper.h"
#include "dbvisit/DbClientMgr.h"
#include "dbvisit/DbClient.h"
#include "dbvisit/DbServer.h"
#include "frmpub/LuaRemoteDebug.h"
#include "shynet/utils/IniConfig.h"
#include "shynet/utils/Singleton.h"

//配置参数
extern const char* g_confname;

namespace dbvisit {
	LuaWrapper::LuaWrapper() {
	}

	LuaWrapper::~LuaWrapper() {
	}

	void LuaWrapper::init(kaguya::State& state) {
		frmpub::FrmLuaWrapper::init(state);

		state["DbServer_CPP"].setClass(kaguya::UserdataMetatable<DbServer, net::ListenEvent>()
		);

		state["DbClient_CPP"].setClass(kaguya::UserdataMetatable<DbClient, frmpub::Client>()
		);

		//是否开启调试模式
		shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
		std::string luadebugip = ini.get<const char*, std::string>(g_confname, "luadebugip", "");
		if (!luadebugip.empty()){
			shynet::utils::Singleton<frmpub::LuaRemoteDebug>::instance().init(luadebugip).start(state);
		}

		//载入lua文件
		std::string luafile = ini.get<const char*, std::string>(g_confname, "luafile", "lua/dbvisit/db_main.lua");
		state.dofile(luafile);

		shynet::utils::Singleton<frmpub::LuaRemoteDebug>::instance().stop(state);
	}
}