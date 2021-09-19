#pragma once
#include "shynet/net/TimerEvent.h"
#include <unordered_map>

namespace dbvisit {
	class DataTimer : public net::TimerEvent {
	public:
		DataTimer(const std::string& cachekey, const struct timeval val);
		~DataTimer();

		/// <summary>
		/// 计时器线程超时回调
		/// </summary>
		void timeout() override;

		/// <summary>
		/// 修改缓存的数据
		/// </summary>
		/// <param name="data">新数据</param>
		void modify_cache_fields(const std::unordered_map<std::string, std::string>& data);
	private:
		std::string cachekey_;
		std::unordered_map<std::string, std::string> fields_;
	};
}
