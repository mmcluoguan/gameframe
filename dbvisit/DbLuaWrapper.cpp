#include "dbvisit/DbLuaWrapper.h"
#include "shynet/IniConfig.h"
#include "shynet/Singleton.h"
#include "dbvisit/DbServer.h"
#include "dbvisit/DbClient.h"
#include "dbvisit/DbClientMgr.h"

namespace dbvisit {
	DbLuaWrapper::DbLuaWrapper() {
		GOBJ++;
		LOG_TRACE << "DbLuaWrapper:" << GOBJ;
	}

	DbLuaWrapper::~DbLuaWrapper() {
		GOBJ--;
		LOG_TRACE << "DbLuaWrapper:" << GOBJ;
	}

	void DbLuaWrapper::init(kaguya::State& state) {
		frmpub::FrmLuaWrapper::init(state);

		state["DbServer"].setClass(kaguya::UserdataMetatable<DbServer, net::ListenEvent>());

		state["DbClient"].setClass(kaguya::UserdataMetatable<DbClient, frmpub::Client>());

		state["DbClientMgr"].setClass(kaguya::UserdataMetatable<DbClientMgr>()
			.addFunction("find", &DbClientMgr::find));

		//注册实例
		state["dbClientMgr"] = &shynet::Singleton<DbClientMgr>::instance();

		//载入lua文件
		shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
		std::string luafile = ini.get<const char*, std::string>("dbvisit", "luafile", "lua/dbvisit/db_main.lua");
		state.dofile(luafile);
	}
}
