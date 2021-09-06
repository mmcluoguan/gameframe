#include "login/LuaWrapper.h"
#include "shynet/IniConfig.h"
#include "shynet/Singleton.h"
#include "login/LoginServer.h"
#include "login/LoginClient.h"
#include "login/LoginClientMgr.h"
#include "login/DbConnector.h"
#include "login/WorldConnector.h"

namespace login {
	LuaWrapper::LuaWrapper() {
	}

	LuaWrapper::~LuaWrapper() {
	}

	void LuaWrapper::init(kaguya::State& state) {
		frmpub::FrmLuaWrapper::init(state);

		state["LoginServer_CPP"].setClass(kaguya::UserdataMetatable<LoginServer, net::ListenEvent>()
		);

		state["LoginClient_CPP"].setClass(kaguya::UserdataMetatable<LoginClient, frmpub::Client>()
		);

		state["DbConnector_CPP"].setClass(kaguya::UserdataMetatable<DbConnector, frmpub::Connector>()
		);

		state["WorldConnector_CPP"].setClass(kaguya::UserdataMetatable<WorldConnector, frmpub::Connector>()
		);

		//载入lua文件
		shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
		std::string luafile = ini.get<const char*, std::string>("login", "luafile", "lua/login/login_main.lua");
		state.dofile(luafile);
	}
}
