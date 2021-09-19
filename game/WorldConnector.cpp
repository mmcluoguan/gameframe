#include "game/WorldConnector.h"
#include "game/ConnectorMgr.h"
#include "game/GameClientMgr.h"
#include "frmpub/LuaCallBackTask.h"
#include "frmpub/ReConnectTimer.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/utils/IniConfig.h"

namespace game {
	WorldConnector::WorldConnector(std::shared_ptr<net::IPAddress> connect_addr) :
		frmpub::Connector(connect_addr, "WorldConnector") {
		pmb_ = {
			{
				protocc::ERRCODE,
				std::bind(&WorldConnector::errcode,this,std::placeholders::_1,std::placeholders::_2)
			},
		};
	}
	WorldConnector::~WorldConnector() {
		if (active() == net::ConnectEvent::CloseType::CLIENT_CLOSE) {
			LOG_INFO << "连接world客户端主动关闭连接 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
		else if (active() == net::ConnectEvent::CloseType::SERVER_CLOSE) {
			LOG_INFO << "服务器world主动关闭连接 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
		else if (active() == net::ConnectEvent::CloseType::CONNECT_FAIL) {
			LOG_INFO << "连接服务器world失败 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
		if (active() != net::ConnectEvent::CloseType::CLIENT_CLOSE) {
			LOG_INFO << "3秒后开始重连";
			std::shared_ptr<frmpub::ReConnectTimer<WorldConnector>> reconnect(
				new frmpub::ReConnectTimer<WorldConnector>(connect_addr(), { 3L,0L }));
			shynet::utils::Singleton<net::TimerReactorMgr>::instance().add(reconnect);
		}
	}
	void WorldConnector::complete() {
		LOG_INFO << "连接服务器world成功 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		shynet::utils::Singleton<ConnectorMgr>::instance().add_worldctor(connectid());

		//通知lua的onConnect函数
		shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnConnectorTask<WorldConnector>>(shared_from_this()));

		//向世界服注册游戏服信息
		protocc::register_game_world_c msgc;
		protocc::ServerInfo* sif = msgc.mutable_sif();
		sif->set_ip(shynet::utils::Singleton<GameClientMgr>::instance().listen_addr().ip());
		sif->set_port(shynet::utils::Singleton<GameClientMgr>::instance().listen_addr().port());
		sif->set_st(protocc::ServerType::GAME);
		shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
		int sid = ini.get<int, int>("game", "sid", 1);
		sif->set_sid(sid);
		std::string name = ini.get<const char*, std::string>("game", "name", "");
		sif->set_name(name);
		send_proto(protocc::REGISTER_GAME_WORLD_C, &msgc);

	}

	int WorldConnector::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		if (obj != nullptr) {
			auto it = pmb_.find(obj->msgid());
			if (it != pmb_.end()) {
				return it->second(obj, enves);
			}
			else {
				//通知lua的onMessage函数
				shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
					std::make_shared<frmpub::OnMessageTask<WorldConnector>>(shared_from_this(), obj, enves));
			}
		}
		return 0;
	}

	void WorldConnector::close(net::ConnectEvent::CloseType active) {
		//通知lua的onClose函数
		shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnCloseTask>(fd()));

		shynet::utils::Singleton<ConnectorMgr>::instance().remove_worldctor(connectid());
		Connector::close(active);
	}

	int WorldConnector::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
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
}