#include "login/DbConnector.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/lua/LuaEngine.h"
#include "shynet/IniConfig.h"
#include "frmpub/ReConnectTimer.h"
#include "frmpub/LuaCallBackTask.h"
#include "frmpub/protocc/login.pb.h"
#include "login/ConnectorMgr.h"
#include "login/LoginClientMgr.h"
#include "login/LoginServer.h"

namespace login {
	DbConnector::DbConnector(std::shared_ptr<net::IPAddress> connect_addr) :
		frmpub::Connector(connect_addr, "DbConnector") {
		pmb_ = {
			{
				protocc::ERRCODE,
				std::bind(&DbConnector::errcode,this,std::placeholders::_1,std::placeholders::_2)
			},
			{
				protocc::REGISTER_LOGIN_DBVISIT_S,
				std::bind(&DbConnector::register_login_dbvisit_s,this,std::placeholders::_1,std::placeholders::_2)
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
		shynet::Singleton<ConnectorMgr>::instance().dbctor_id(connectid());

		//通知lua的onConnect函数
		shynet::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnConnectorTask<DbConnector>>(shared_from_this()));

		//向db服注册服务器信息
		protocc::register_login_dbvisit_c msgc;
		protocc::ServerInfo* sif = msgc.mutable_sif();
		shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
		std::string loginip = ini.get<const char*, std::string>("login", "ip", "127.0.0.1");
		short loginport = ini.get<short, short>("login", "port", short(24000));
		sif->set_ip(loginip);
		sif->set_port(loginport);
		sif->set_st(protocc::ServerType::LOGIN);
		int sid = ini.get<int, int>("login", "sid", 1);
		sif->set_sid(sid);
		std::string name = ini.get<const char*, std::string>("login", "name", "");
		sif->set_name(name);
		send_proto(protocc::REGISTER_LOGIN_DBVISIT_C, &msgc);
	}
	int DbConnector::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		if (obj != nullptr) {
			if (enves->empty() == false) {
				FilterData::Envelope& env = enves->top();
				enves->pop();
				std::shared_ptr<LoginClient> gate = shynet::Singleton<LoginClientMgr>::instance().find(env.fd);
				if (gate != nullptr) {
					gate->send_proto(obj.get(), enves.get());
					LOG_DEBUG << "转发消息" << frmpub::Basic::msgname(obj->msgid())
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
		}
		return 0;
	}

	void DbConnector::close(net::ConnectEvent::CloseType active) {
		//通知lua的onClose函数
		shynet::Singleton<lua::LuaEngine>::get_instance().append(
			std::make_shared<frmpub::OnCloseTask>(fd()));

		shynet::Singleton<ConnectorMgr>::instance().dbctor_id(0);
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

	int DbConnector::register_login_dbvisit_s(std::shared_ptr<protocc::CommonObject> data, std::shared_ptr<std::stack<FilterData::Envelope>> enves) {
		protocc::register_login_dbvisit_s msgc;
		if (msgc.ParseFromString(data->msgdata()) == true) {
			shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
			if (msgc.result() == 0) {
				static bool oc = true;
				if (oc == true) {
					LOG_DEBUG << "开启登录服服务器监听";
					std::string loginip = ini.get<const char*, std::string>("login", "ip", "127.0.0.1");
					short loginport = ini.get<short, short>("login", "port", short(23000));
					std::shared_ptr<net::IPAddress> loginaddr(new net::IPAddress(loginip.c_str(), loginport));
					std::shared_ptr<LoginServer> loginserver(new LoginServer(loginaddr));
					shynet::Singleton<net::ListenReactorMgr>::instance().add(loginserver);

					LOG_DEBUG << "开始连接世界服";
					std::string regip = ini.get<const char*, std::string>("world", "ip", "127.0.0.1");
					short regport = ini.get<short, short>("world", "port", short(22000));
					shynet::Singleton<net::ConnectReactorMgr>::instance().add(
						std::shared_ptr<WorldConnector>(
							new WorldConnector(std::shared_ptr<net::IPAddress>(
								new net::IPAddress(regip.c_str(), regport)))));
					oc = false;
				}
			}
			else {
				shynet::IniConfig& ini = shynet::Singleton<shynet::IniConfig>::get_instance();
				int sid = ini.get<int, int>("login", "sid", 1);
				LOG_WARN << frmpub::Basic::connectname(protocc::ServerType::LOGIN) << " sid:" << sid << " 已存在";
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
