#include "world/LuaWrapper.h"
#include "world/DbConnector.h"
#include "world/HttpServer.h"
#include "world/HttpClient.h"
#include "world/HttpClientMgr.h"
#include "world/WorldServer.h"
#include "world/WorldClient.h"
#include "world/WorldClientMgr.h"
#include "frmpub/LuaRemoteDebug.h"
#include "shynet/utils/IniConfig.h"

//配置参数
extern const char* g_confname;

namespace world {
	LuaWrapper::LuaWrapper() {
	}

	LuaWrapper::~LuaWrapper() {
	}

	void LuaWrapper::init(kaguya::State& state) {
		frmpub::FrmLuaWrapper::init(state);

		state["WorldServer_CPP"].setClass(kaguya::UserdataMetatable<WorldServer, net::ListenEvent>()
		);

		state["WorldClient_CPP"].setClass(kaguya::UserdataMetatable<WorldClient, frmpub::Client>()
		);

		state["HttpServer_CPP"].setClass(kaguya::UserdataMetatable<HttpServer, net::ListenEvent>()
		);

		state["HttpClient_CPP"].setClass(kaguya::UserdataMetatable<HttpClient, frmpub::Client>()
		);

		state["DbConnector_CPP"].setClass(kaguya::UserdataMetatable<DbConnector, frmpub::Connector>()
		);

		//是否开启调试模式
		shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
		std::string luadebugip = ini.get<const char*, std::string>(g_confname, "luadebugip", "");
		if (!luadebugip.empty()) {
			shynet::utils::Singleton<frmpub::LuaRemoteDebug>::instance().init(luadebugip).start(state);
		}

		//载入lua文件
		std::string luafile = ini.get<const char*, std::string>("world", "luafile", "lua/world/world_main.lua");
		state.dofile(luafile);

		shynet::utils::Singleton<frmpub::LuaRemoteDebug>::instance().stop(state);
	}
}
