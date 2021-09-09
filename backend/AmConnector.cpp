#include "backend/AmConnector.h"
#include "shynet/events/Streambuff.h"
#include "shynet/net/ConnectReactorMgr.h"
#include "shynet/IniConfig.h"
#include "frmpub/ReConnectTimer.h"

namespace backend {
	AmConnector::AmConnector(std::shared_ptr<net::IPAddress> connect_addr) :
		frmpub::Connector(connect_addr, false, true, shynet::protocol::FilterProces::ProtoType::WEBSOCKET) {
		GOBJ++;
		LOG_TRACE << "AmConnector:" << GOBJ;

		pmb_ = {

		};
	}
	AmConnector::~AmConnector() {
		GOBJ--;
		LOG_TRACE << "~AmConnector:" << GOBJ;
		if (active() == net::ConnectEvent::CloseType::CLIENT_CLOSE) {
			LOG_INFO << "连接animal客户端主动关闭连接 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
		else if (active() == net::ConnectEvent::CloseType::SERVER_CLOSE) {
			LOG_INFO << "服务器animal主动关闭连接 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
		else if (active() == net::ConnectEvent::CloseType::CONNECT_FAIL) {
			LOG_INFO << "连接服务器animal失败 " << \
				"[ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		}
		if (active() != net::ConnectEvent::CloseType::CLIENT_CLOSE) {
			LOG_INFO << "3秒后开始重连";
			std::shared_ptr<frmpub::ReConnectTimer<AmConnector>> reconnect(
				new frmpub::ReConnectTimer<AmConnector>(connect_addr(), { 3L,0L }));
			shynet::Singleton<net::TimerReactorMgr>::instance().add(reconnect);
		}
	}
	void AmConnector::complete() {
		LOG_INFO << "连接服务器animal成功 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		AmConnector::am_id = connectid();
	}
	int AmConnector::input_handle(std::shared_ptr<protocc::CommonObject> obj, std::stack<Envelope>& enves) {
		if (obj != nullptr) {
			auto it = pmb_.find(obj->msgid());
			if (it != pmb_.end()) {
				return it->second(obj, enves);
			}
			else {
				LOG_WARN << "消息" << frmpub::Basic::internal_msgname(obj->msgid()) << " 没有处理函数";
			}
		}
		return 0;
	}

	int AmConnector::am_id = 0;

	std::shared_ptr<AmConnector> AmConnector::am_connector() {
		return std::dynamic_pointer_cast<AmConnector>(
			shynet::Singleton<net::ConnectReactorMgr>::instance().find(am_id));
	}
}
