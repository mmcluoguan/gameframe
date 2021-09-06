#include "game/LuaWrapper.h"
#include <random>
#include "shynet/IniConfig.h"
#include "shynet/Singleton.h"
#include "shynet/IdWorker.h"
#include "game/GameServer.h"
#include "game/GameClient.h"
#include "game/DbConnector.h"
#include "game/WorldConnector.h"

namespace game {
	LuaWrapper::LuaWrapper() {
	}

	LuaWrapper::~LuaWrapper() {
	}

	void LuaWrapper::init(kaguya::State& state) {
		frmpub::FrmLuaWrapper::init(state);

		state["newid"] = kaguya::function([]() {
			return shynet::Singleton<shynet::IdWorker>::get_instance().getid();
			});

		state["random"] = kaguya::function([](int min, int max) {
			std::default_random_engine random(time(nullptr));
			std::uniform_int_distribution<int> num_random(1, 20);
			return num_random(random);
			});

		state["GameServer_CPP"].setClass(kaguya::UserdataMetatable<GameServer, net::ListenEvent>()
		);

		state["GameClient_CPP"].setClass(kaguya::UserdataMetatable<GameClient, frmpub::Client>()
		);

		state["DbConnector_CPP"].setClass(kaguya::UserdataMetatable<DbConnector, frmpub::Connector>()
		);

		state["WorldConnector_CPP"].setClass(kaguya::UserdataMetatable<WorldConnector, frmpub::Connector>()
		);

		//载入lua文件
		shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
		std::string luafile = ini.get<const char*, std::string>("game", "luafile", "lua/game/game_main.lua");
		state.dofile(luafile);
	}
}
