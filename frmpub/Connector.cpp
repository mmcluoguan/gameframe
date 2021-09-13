#include "frmpub/Connector.h"
#include "shynet/net/TimerReactorMgr.h"
#include "shynet/net/ConnectReactorMgr.h"

namespace frmpub {
	Connector::Connector(std::shared_ptr<net::IPAddress> connect_addr,
		std::string name,
		bool enable_ssl,
		bool enable_ping,
		ssize_t heartSecond,
		protocol::FilterProces::ProtoType pt,
		FilterData::ProtoData pd)
		: net::ConnectEvent(connect_addr, pt, enable_ssl, false), FilterData(pd) {
		name_ = name;
		enable_ping_ = enable_ping;
		heartSecond_ = heartSecond;
		filter_ = this;
	}
	Connector::~Connector() {
		std::shared_ptr<PingTimer> pt = ping_timer_.lock();
		if (pt != nullptr) {
			pt->clean_connector();
		}
	}
	void Connector::success() {
		net::ConnectEvent::success();
		if (enable_ping_ == true) {
			std::shared_ptr<PingTimer> pt(new PingTimer({ heartSecond_,0L }, this));
			shynet::utils::Singleton<net::TimerReactorMgr>::instance().add(pt);
			ping_timer_ = pt;
		}
	}
	int Connector::input() {
		//有数据接收到，因此延迟心跳计时器时间
		std::shared_ptr<PingTimer> pt = ping_timer_.lock();
		if (pt != nullptr) {
			pt->val({ heartSecond_,0L });
		}
		return net::ConnectEvent::input();
	}

	int Connector::message_handle(char* original_data, size_t datalen) {
		return FilterData::message_handle(original_data, datalen);
	}

	void Connector::close(net::ConnectEvent::CloseType active) {
		active_ = active;
		shynet::utils::Singleton<net::ConnectReactorMgr>::instance().remove(connectid());
	}
	void Connector::timerout() {
		LOG_INFO << "服务器心跳超时 [ip:" << connect_addr()->ip() << ":" << connect_addr()->port() << "]";
		close(net::ConnectEvent::CloseType::TIMEOUT_CLOSE);
	}

	net::ConnectEvent::CloseType Connector::active() const {
		return active_;
	}
}
