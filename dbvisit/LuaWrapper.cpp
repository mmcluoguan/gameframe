#include "dbvisit/LuaWrapper.h"
#include "shynet/IniConfig.h"
#include "shynet/Singleton.h"
#include "frmpub/LuaRemoteDebug.h"
#include "dbvisit/DbServer.h"
#include "dbvisit/DbClient.h"
#include "dbvisit/DbClientMgr.h"

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
		shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
		std::string luadebugip = ini.get<const char*, std::string>("dbvisit", "luadebugip", "");
		if (!luadebugip.empty()){
			shynet::Singleton<frmpub::LuaRemoteDebug>::instance().init(luadebugip).start(state);
		}

		//载入lua文件
		std::string luafile = ini.get<const char*, std::string>("dbvisit", "luafile", "lua/dbvisit/db_main.lua");
		state.dofile(luafile);

		shynet::Singleton<frmpub::LuaRemoteDebug>::instance().stop(state);
	}
}
