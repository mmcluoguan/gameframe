#include "animal/ConfirmOrderTimer.h"
#include "shynet/Utility.h"
#include "shynet/Logger.h"
#include "shynet/net/TimerReactorMgr.h"
#include "shynet/pool/MysqlPool.h"

namespace animal {
	ConfirmOrderTimer::ConfirmOrderTimer(const timeval val) :
		net::TimerEvent(val, EV_TIMEOUT | EV_PERSIST) {
		GOBJ++;
		LOG_TRACE << "ConfirmOrderTimer:" << GOBJ;
	}

	ConfirmOrderTimer::~ConfirmOrderTimer() {
		GOBJ--;
		LOG_TRACE << "~ConfirmOrderTimer:" << GOBJ;
	}

	void ConfirmOrderTimer::timeout() {
		try {
			std::string sql = shynet::Utility::str_format("select id,UNIX_TIMESTAMP(pay_time),UNIX_TIMESTAMP() from `order` WHERE state = 2");
			pool::MysqlPool& mysql = shynet::Singleton<pool::MysqlPool>::get_instance();
			mysqlx::SqlResult sr = mysql.fetch()->sql(sql).execute();
			for (auto item : sr.fetchAll()) {
				int64_t orderid = item.get(0);
				int rush_time = item.get(1);
				int now_time = item.get(2);
				if (now_time - rush_time >= 60 * 60) {
					sql = shynet::Utility::str_format("call confirm_order(%llu)", orderid);
					mysql.fetch()->sql(sql).execute();
				}
			}
		}
		catch (const mysqlx::Error& err) {
			LOG_WARN << err.what();
		}
	}
}
