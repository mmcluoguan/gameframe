#include "login/WorldConnector.h"
#include "login/ConnectorMgr.h"
#include "login/LoginClientMgr.h"
#include "frmpub/LuaCallBackTask.h"
#include "frmpub/ReConnectTimer.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/utils/IniConfig.h"

namespace login {
	WorldConnector::WorldConnector(std::shared_ptr<net::IPAddress> connect_addr) :
		frmpub::Connector(connect_addr, "WorldConnector") {
		pmb_ = {
			{
				protocc::ERRCODE,
				std::bind(&WorldConnector::errcode,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::LOGIN_CLIENT_GATE_S,
				std::bind(&WorldConnector::login_client_gate_s,this,std::placeholders::_1,std::placeholders::_2)
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

		//向世界服注册登录服信息
		protocc::register_login_world_c msgc;
		protocc::ServerInfo* sif = msgc.mutable_sif();
		sif->set_ip(shynet::utils::Singleton<LoginClientMgr>::instance().listen_addr().ip());
		sif->set_port(shynet::utils::Singleton<LoginClientMgr>::instance().listen_addr().port());
		sif->set_st(protocc::ServerType::LOGIN);
		shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
		int sid = ini.get<int, int>("login", "sid", 1);
		sif->set_sid(sid);
		std::string name = ini.get<const char*, std::string>("login", "name", "");
		sif->set_name(name);
		send_proto(protocc::REGISTER_LOGIN_WORLD_C, &msgc);

	}
	int WorldConnector::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		if (obj != nullptr) {
			auto it = pmb_.find(obj->msgid());
			if (it != pmb_.end()) {
				return it->second(obj, enves);
			}
			else {		
				if (enves->empty() == false) {
					return forward_world_gate_c(obj, enves);
				}
				else {
					//通知lua的onMessage函数
					shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
						std::make_shared<frmpub::OnMessageTask<WorldConnector>>(shared_from_this(), obj, enves));
				}
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

	int WorldConnector::forward_world_gate_c(std::shared_ptr<protocc::CommonObject> data,
		std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		if (enves->empty() == false) {
			FilterData::Envelope& env = enves->top();
			enves->pop();
			std::shared_ptr<LoginClient> gate = shynet::utils::Singleton<LoginClientMgr>::instance().find(env.fd);
			if (gate != nullptr) {
				gate->send_proto(data.get(), enves.get());
				LOG_DEBUG << "转发消息" << frmpub::Basic::msgname(data->msgid())
					<< "到gate[" << gate->remote_addr()->ip() << ":"
					<< gate->remote_addr()->port() << "]"
					<< " gate fd:" << env.fd;
			}
			else {
				std::stringstream stream;
				stream << "gate fd:" << env.fd << " 已断开连接";
				SEND_ERR(protocc::GATE_NOT_EXIST, stream.str());
			}
		}
		else {
			SEND_ERR(protocc::NO_ROUTING_INFO, "转发消息没有路由信息");
		}
		return 0;
	}

	int WorldConnector::login_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
		std::shared_ptr<std::stack<FilterData::Envelope>> enves) {

		protocc::login_client_gate_s msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			std::shared_ptr<DbConnector> db = shynet::utils::Singleton<ConnectorMgr>::instance().db_connector();
			if (db != nullptr) {
				//更新world选择的gameid到db
				protocc::updata_to_dbvisit_c updata;
				updata.set_cache_key("account_" + msgc.aid());
				auto fields = updata.add_fields();
				fields->set_key("game_sid");
				fields->set_value(data->extend());

				db->send_proto(protocc::UPDATA_TO_DBVISIT_C, &updata);
				LOG_DEBUG << "发送更新数据消息" << frmpub::Basic::msgname(data->msgid())
					<< "到dbvisit[" << db->connect_addr()->ip() << ":"
					<< db->connect_addr()->port() << "]";
			}
			else {
				SEND_ERR_EX(protocc::DBVISIT_NOT_EXIST, "没有可选的db连接", enves.get());
			}
		}
		else {
			std::stringstream stream;
			stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
			SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
		}
		return forward_world_gate_c(data, enves);
	}
}
