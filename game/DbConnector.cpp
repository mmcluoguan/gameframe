#include "game/DbConnector.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/IniConfig.h"
#include "shynet/lua/LuaEngine.h"
#include "frmpub/LuaCallBackTask.h"
#include "frmpub/ReConnectTimer.h"
#include "frmpub/protocc/game.pb.h"
#include "game/ConnectorMgr.h"
#include "game/GameClientMgr.h"
#include "game/GameServer.h"

namespace game {
	DbConnector::DbConnector(std::shared_ptr<net::IPAddress> connect_addr) :
		frmpub::Connector(connect_addr, "DbConnector") {

		pmb_ = {
			{
				protocc::ERRCODE,
				std::bind(&DbConnector::errcode,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::REGISTER_GAME_DBVISIT_S,
				std::bind(&DbConnector::register_game_dbvisit_s,this,std::placeholders::_1,std::placeholders::_2)
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
		shynet::Singleton<ConnectorMgr>::instance().add_dbctor(connectid());

		//通知lua的onConnect函数
		shynet::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnConnectorTask<DbConnector>>(shared_from_this()));

		//向db服游戏服务器信息
		protocc::register_game_dbvisit_c msgc;
		protocc::ServerInfo* sif = msgc.mutable_sif();
		shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
		std::string gameip = ini.get<const char*, std::string>("game", "ip", "127.0.0.1");
		short gameport = ini.get<short, short>("game", "port", short(24000));
		sif->set_ip(gameip);
		sif->set_port(gameport);
		sif->set_st(protocc::ServerType::GAME);
		int sid = ini.get<int, int>("game", "sid", 1);
		sif->set_sid(sid);
		std::string name = ini.get<const char*, std::string>("game", "name", "");
		sif->set_name(name);
		send_proto(protocc::REGISTER_GAME_DBVISIT_C, &msgc);
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

		shynet::Singleton<ConnectorMgr>::instance().remove_dbctor(connectid());
		Connector::close(active);
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

	int DbConnector::register_game_dbvisit_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		protocc::register_game_dbvisit_s msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			if (msgc.result() == 0) {				
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
