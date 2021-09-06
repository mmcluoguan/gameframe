#include "gate/DbConnector.h"
#include <unistd.h>
#include "shynet/events/Streambuff.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/IniConfig.h"
#include "frmpub/ReConnectTimer.h"
#include "frmpub/LuaCallBackTask.h"
#include "frmpub/protocc/gate.pb.h"
#include "gate/ConnectorMgr.h"
#include "gate/GateClientMgr.h"
#include "gate/GateServer.h"

namespace gate {
	DbConnector::DbConnector(std::shared_ptr<net::IPAddress> connect_addr) :
		frmpub::Connector(connect_addr, "DbConnector") {

		pmb_ = {
			{
				protocc::ERRCODE,
				std::bind(&DbConnector::errcode,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::REGISTER_GATE_DBVISIT_S,
				std::bind(&DbConnector::register_gate_dbvisit_s,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::REPEATLOGIN_CLIENT_GATE_S,
				std::bind(&DbConnector::repeatlogin_client_gate_s,this,std::placeholders::_1,std::placeholders::_2)
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
		shynet::Singleton<ConnectorMgr>::instance().db_id(connectid());

		//通知lua的onConnect函数
		shynet::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnConnectorTask<DbConnector>>(shared_from_this()));

		//向db服注册服务器信息
		protocc::register_gate_dbvisit_c msgc;
		protocc::ServerInfo* sif = msgc.mutable_sif();
		shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
		std::string gateip = ini.get<const char*, std::string>("gate", "ip", "127.0.0.1");
		short gateport = ini.get<short, short>("gate", "port", short(25000));
		sif->set_ip(gateip);
		sif->set_port(gateport);
		sif->set_st(protocc::ServerType::GATE);
		int sid = ini.get<int, int>("gate", "sid", 1);
		sif->set_sid(sid);
		std::string name = ini.get<const char*, std::string>("gate", "name", "");
		sif->set_name(name);
		send_proto(protocc::REGISTER_GATE_DBVISIT_C, &msgc);
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

		shynet::Singleton<ConnectorMgr>::instance().db_id(0);
		Connector::close(active);
	}

	int DbConnector::errcode(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		protocc::errcode err;
		if (err.ParseFromString(data->msgdata()) == true) {
			LOG_DEBUG << "错误码:" << err.code() << " 描述:" << err.desc();
			if (enves->empty() == false) {
				FilterData::Envelope& env = enves->top();
				enves->pop();
				std::shared_ptr<GateClient> client = shynet::Singleton<GateClientMgr>::instance().find(env.fd);
				if (client != nullptr) {
					client->send_proto(data.get(), enves.get());
				}
				else {
					std::stringstream stream;
					stream << "client fd:" << env.fd << " 已断开连接";
					SEND_ERR(protocc::CLIENT_CLOSEED, stream.str());
				}
			}
		}
		else {
			std::stringstream stream;
			stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
			SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
		}
		return 0;
	}

	int DbConnector::register_gate_dbvisit_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		protocc::register_gate_dbvisit_s msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
			if (msgc.result() == 0) {
				static bool oc = true;
				if (oc == true) {
					LOG_DEBUG << "开启网关服服务器监听";
					std::string gateip = ini.get<const char*, std::string>("gate", "ip", "127.0.0.1");
					short gateport = ini.get<short, short>("gate", "port", short(25000));
					std::shared_ptr<net::IPAddress> gateaddr(new net::IPAddress(gateip.c_str(), gateport));
					std::shared_ptr<GateServer> gateserver(new GateServer(gateaddr));
					shynet::Singleton<net::ListenReactorMgr>::instance().add(gateserver);

					LOG_DEBUG << "开始连接世界服";
					std::string worldip = ini.get<const char*, std::string>("world", "ip", "127.0.0.1");
					short worldport = ini.get<short, short>("world", "port", short(22000));
					std::shared_ptr<net::IPAddress> registeraddr(new net::IPAddress(worldip.c_str(), worldport));
					shynet::Singleton<net::ConnectReactorMgr>::instance().add(
						std::shared_ptr<WorldConnector>(
							new WorldConnector(std::shared_ptr<net::IPAddress>(
								new net::IPAddress(worldip.c_str(), worldport)))));
					oc = false;
				}
			}
			else {
				shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
				int sid = ini.get<int, int>("gate", "sid", 1);
				LOG_WARN << frmpub::Basic::connectname(protocc::ServerType::GATE) << " sid:" << sid << " 已存在";
				return 0;
			}
		}
		else {
			std::stringstream stream;
			stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
			SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
		}
		return 0;
	}
	int DbConnector::repeatlogin_client_gate_s(std::shared_ptr<protocc::CommonObject> data,
		std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		protocc::repeatlogin_client_gate_s msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			auto cli = shynet::Singleton<GateClientMgr>::instance().find(msgc.aid());
			if (cli) {
				cli->send_proto(data.get());
				cli->close(true);
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
