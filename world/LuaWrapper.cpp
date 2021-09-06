#include "world/LuaWrapper.h"
#include "shynet/IniConfig.h"
#include "shynet/Singleton.h"
#include "frmpub/LuaRemoteDebug.h"
#include "world/WorldServer.h"
#include "world/WorldClient.h"
#include "world/WorldClientMgr.h"
#include "world/HttpServer.h"
#include "world/HttpClient.h"
#include "world/HttpClientMgr.h"
#include "world/DbConnector.h"

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
		shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
		std::string luadebugip = ini.get<const char*, std::string>("world", "luadebugip", "");
		if (!luadebugip.empty()) {
			shynet::Singleton<frmpub::LuaRemoteDebug>::instance().init(luadebugip).start(state);
		}

		//载入lua文件
		std::string luafile = ini.get<const char*, std::string>("world", "luafile", "lua/world/world_main.lua");
		state.dofile(luafile);

		shynet::Singleton<frmpub::LuaRemoteDebug>::instance().stop(state);
	}
}
