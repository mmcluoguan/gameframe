#include "world/DbConnector.h"
#include "shynet/events/Streambuff.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/IniConfig.h"
#include "frmpub/ReConnectTimer.h"
#include "frmpub/LuaCallBackTask.h"
#include "frmpub/protocc/world.pb.h"
#include "world/WorldClientMgr.h"
#include "world/WorldServer.h"
#include "world/HttpServer.h"

namespace world {
	DbConnector::DbConnector(std::shared_ptr<net::IPAddress> connect_addr) :
		frmpub::Connector(connect_addr, "DbConnector") {
		pmb_ = {
			{
				protocc::ERRCODE,
				std::bind(&DbConnector::errcode,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::REGISTER_WORLD_DBVISIT_S,
				std::bind(&DbConnector::register_world_dbvisit_s,this,std::placeholders::_1,std::placeholders::_2)
			},
		};
	}
	DbConnector::~DbConnector() {
		if (active() == net::ConnectEvent::CloseType::CLIENT_CLOSE) {
			LOG_INFO << "连接dbvisit客户端主动关闭连接 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
		else if (active() == net::ConnectEvent::CloseType::SERVER_CLOSE) {
			LOG_INFO << "服务器dbvisit主动关闭连接 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
		else if (active() == net::ConnectEvent::CloseType::CONNECT_FAIL) {
			LOG_INFO << "连接服务器dbvisit失败 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
		if (active() != net::ConnectEvent::CloseType::CLIENT_CLOSE) {
			LOG_INFO << "3秒后开始重连";
			std::shared_ptr<frmpub::ReConnectTimer<DbConnector>> reconnect(
				new frmpub::ReConnectTimer<DbConnector>(connect_addr(), { 3L,0L }));
			shynet::Singleton<net::TimerReactorMgr>::instance().add(reconnect);
		}
	}
	void DbConnector::complete() {
		LOG_INFO << "连接服务器dbvisit成功 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		DbConnector::db_id = connectid();

		//通知lua的onConnect函数
		shynet::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnConnectorTask<DbConnector>>(shared_from_this()));

		//向db服世界服务器信息
		protocc::register_world_dbvisit_c msgc;
		protocc::ServerInfo* sif = msgc.mutable_sif();
		shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
		std::string registerip = ini.get<const char*, std::string>("world", "ip", "127.0.0.1");
		short registerport = ini.get<short, short>("world", "port", short(24000));
		sif->set_ip(registerip);
		sif->set_port(registerport);
		sif->set_st(protocc::ServerType::WORLD);
		int sid = ini.get<int, int>("world", "sid", 1);
		sif->set_sid(sid);
		std::string name = ini.get<const char*, std::string>("world", "name", "");
		sif->set_name(name);
		send_proto(protocc::REGISTER_WORLD_DBVISIT_C, &msgc);
	}
	int DbConnector::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		if (obj != nullptr) {
			auto it = pmb_.find(obj->msgid());
			if (it != pmb_.end()) {
				return it->second(obj, enves);
			}
			else {
				//通知lua的onMessage函数
				shynet::Singleton<lua::LuaEngine>::get_instance().append(
					std::make_shared<frmpub::OnMessageTask<DbConnector>>(shared_from_this(), obj, enves));
			}
		}
		return 0;
	}

	void DbConnector::close(net::ConnectEvent::CloseType active) {
		//通知lua的onClose函数
		shynet::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnCloseTask>(fd()));

		DbConnector::db_id = 0;
		Connector::close(active);
	}

	int DbConnector::db_id = 0;

	std::shared_ptr<DbConnector> DbConnector::db_connector() {
		return std::dynamic_pointer_cast<DbConnector>(
			shynet::Singleton<net::ConnectReactorMgr>::instance().find(db_id));
	}

	int DbConnector::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		protocc::errcode err;
		if (err.ParseFromString(data->msgdata()) == true) {
			LOG_DEBUG << "错误码:" << err.code() << " 描述:" << err.desc();
		}
		else {
			std::stringstream stream;
			stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
			SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
		}
		return 0;
	}

	int DbConnector::register_world_dbvisit_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		protocc::register_world_dbvisit_s msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
			if (msgc.result() == 0) {
				static bool oc = true;
				if (oc == true) {
					LOG_DEBUG << "开启世界服务器监听";
					std::string regip = ini.get<const char*, std::string>("world", "ip", "127.0.0.1");
					short regport = ini.get<short, short>("world", "port", short(22000));
					std::shared_ptr<net::IPAddress> regaddr(new net::IPAddress(regip.c_str(), regport));
					std::shared_ptr<WorldServer> regserver(new WorldServer(regaddr));
					shynet::Singleton<net::ListenReactorMgr>::instance().add(regserver);

					LOG_DEBUG << "开启http后台服务器监听";
					std::string httpip = ini.get<const char*, std::string>("http", "ip", "127.0.0.1");
					short httpport = ini.get<short, short>("http", "port", short(26000));
					std::shared_ptr<net::IPAddress> httpaddr(new net::IPAddress(httpip.c_str(), httpport));
					std::shared_ptr<HttpServer> httpserver(new HttpServer(httpaddr));
					shynet::Singleton<net::ListenReactorMgr>::instance().add(httpserver);
					oc = false;
				}
			}
			else {
				shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
				int sid = ini.get<int, int>("world", "sid", 1);
				LOG_WARN << frmpub::Basic::connectname(protocc::ServerType::WORLD) << " sid:" << sid << " 已存在";
				return -1;
			}
		}
		else {
			std::stringstream stream;
			stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
			SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
		}
		return 0;
	}
}
