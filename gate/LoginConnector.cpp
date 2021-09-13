#include "gate/LoginConnector.h"
#include <unistd.h>
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/utils/IniConfig.h"
#include "frmpub/ReConnectTimer.h"
#include "frmpub/LuaCallBackTask.h"
#include "frmpub/protocc/gate.pb.h"
#include "gate/ConnectorMgr.h"
#include "gate/GateClientMgr.h"

namespace gate {
	LoginConnector::LoginConnector(std::shared_ptr<net::IPAddress> connect_addr) :
		frmpub::Connector(connect_addr, "LoginConnector") {
		pmb_ = {
			{
				protocc::ERRCODE,
				std::bind(&LoginConnector::errcode,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::REPEATLOGIN_CLIENT_GATE_S,
				std::bind(&LoginConnector::repeatlogin_client_gate_s,this,std::placeholders::_1,std::placeholders::_2)
			},
		};
	}
	LoginConnector::~LoginConnector() {
		if (active() == net::ConnectEvent::CloseType::CLIENT_CLOSE) {
			LOG_INFO << "连接login客户端主动关闭连接 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
		else if (active() == net::ConnectEvent::CloseType::SERVER_CLOSE) {
			LOG_INFO << "服务器login主动关闭连接 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
		else if (active() == net::ConnectEvent::CloseType::CONNECT_FAIL) {
			LOG_INFO << "连接服务器login失败 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
	}
	void LoginConnector::complete() {
		LOG_INFO << "连接服务器login成功 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";

		//通知lua的onConnect函数
		shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnConnectorTask<LoginConnector>>(shared_from_this()));

		//向登录服注册服务器信息
		protocc::register_gate_login_c msgc;
		protocc::ServerInfo* sif = msgc.mutable_sif();
		sif->set_ip(shynet::utils::Singleton<GateClientMgr>::instance().listen_addr().ip());
		sif->set_port(shynet::utils::Singleton<GateClientMgr>::instance().listen_addr().port());
		sif->set_st(protocc::ServerType::GATE);
		shynet::utils::IniConfig& ini = shynet::utils::Singleton<shynet::utils::IniConfig>::get_instance();
		int sid = ini.get<int, int>("gate", "sid", 1);
		sif->set_sid(sid);
		std::string name = ini.get<const char*, std::string>("gate", "name", "");
		sif->set_name(name);
		send_proto(protocc::REGISTER_GATE_LOGIN_C, &msgc);
	}
	int LoginConnector::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		if (obj != nullptr) {			
			//直接处理的登录服消息
			auto it = pmb_.find(obj->msgid());
			if (it != pmb_.end()) {
				return it->second(obj, enves);
			}
			else {
				if (enves->empty() == false) {
					return forward_login_client_c(obj, enves);
				}
				else {
					//通知lua的onMessage函数
					shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
						std::make_shared<frmpub::OnMessageTask<LoginConnector>>(shared_from_this(), obj, enves));
				}
			}
		}
		return 0;
	}
	void LoginConnector::close(net::ConnectEvent::CloseType active) {
		//通知lua的onClose函数
		shynet::utils::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnCloseTask>(fd()));

		shynet::utils::Singleton<ConnectorMgr>::instance().remove(login_connect_id_);
		Connector::close(active);
	}
	void LoginConnector::login_conncet_id(int v) {
		login_connect_id_ = v;
	}
	int LoginConnector::login_conncet_id() const {
		return login_connect_id_;
	}
	int LoginConnector::errcode(std::shared_ptr<protocc::CommonObject> data,
		std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		protocc::errcode err;
		if (err.ParseFromString(data->msgdata()) == true) {
			LOG_DEBUG << "错误码:" << err.code() << " 描述:" << err.desc();
			if (enves->empty() == false) {
				FilterData::Envelope& env = enves->top();
				enves->pop();
				std::shared_ptr<GateClient> client = shynet::utils::Singleton<GateClientMgr>::instance().find(env.fd);
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

	int LoginConnector::forward_login_client_c(std::shared_ptr<protocc::CommonObject> data,
		std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		if (enves->empty() == false) {
			FilterData::Envelope& env = enves->top();
			enves->pop();
			std::shared_ptr<GateClient> client = shynet::utils::Singleton<GateClientMgr>::instance().find(env.fd);
			if (client != nullptr) {
				if (data->msgid() == protocc::LOGIN_CLIENT_GATE_S) {
					protocc::login_client_gate_s msgc;
					if (msgc.ParseFromString(data->msgdata()) == true) {
						if (msgc.result() == 0) {
							client->accountid(msgc.aid());
							if (data->extend().empty() == false) {
								//从扩展信息中取出gamesid
								client->game_id(atoi(data->extend().c_str()));
							}
						}
					}
					else {
						std::stringstream stream;
						stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
						SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
					}
				}
				else if (data->msgid() == protocc::RECONNECT_CLIENT_GATE_S) {
					protocc::reconnect_client_gate_s msgc;
					if (msgc.ParseFromString(data->msgdata()) == true) {
						if (msgc.result() == 0) {
							client->accountid(msgc.aid());
							client->login_id(msgc.loginid());
							client->game_id(msgc.gameid());

							//通知游戏服玩家断线重连成功
							int conncetid = shynet::utils::Singleton<ConnectorMgr>::instance().sid_conv_connect_id(msgc.gameid());
							auto game = shynet::utils::Singleton<ConnectorMgr>::instance().game_connector(conncetid);
							if (game != nullptr) {
								game->send_proto(data.get());
								LOG_DEBUG << "通知游戏服玩家断线重连成功" << frmpub::Basic::msgname(data->msgid());
							}
						}
					}
					else {
						std::stringstream stream;
						stream << "消息" << frmpub::Basic::msgname(data->msgid()) << "解析错误";
						SEND_ERR(protocc::MESSAGE_PARSING_ERROR, stream.str());
					}
				}

				client->send_proto(data.get(), enves.get());
				LOG_DEBUG << "转发消息" << frmpub::Basic::msgname(data->msgid())
					<< "到client[" << client->remote_addr()->ip() << ":"
					<< client->remote_addr()->port() << "]"
					<< " client fd:" << env.fd;
			}
			else {
				std::stringstream stream;
				stream << "client fd:" << env.fd << " 已断开连接";
				SEND_ERR(protocc::CLIENT_CLOSEED, stream.str());
			}
		}
		else {
			SEND_ERR(protocc::NO_ROUTING_INFO, "转发消息没有路由信息");
		}
		return 0;
	}
	
	int LoginConnector::repeatlogin_client_gate_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves)
	{
		protocc::repeatlogin_client_gate_s msg;
		if (msg.ParseFromString(data->msgdata()) == true) {
			std::shared_ptr<GateClient> client = shynet::utils::Singleton<GateClientMgr>::instance().find(msg.aid());
			if (client != nullptr) {
				//顶掉之前登录的账号
				client->send_proto(data.get(), enves.get());
				client->close(true);
			}
			else {
				std::stringstream stream;
				stream << "client acccount:" << msg.aid() << " 已断开连接";
				SEND_ERR(protocc::CLIENT_CLOSEED, stream.str());
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
