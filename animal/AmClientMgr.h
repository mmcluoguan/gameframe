#pragma once
#include <memory>
#include <mutex>
#include <unordered_map>
#include "shynet/Singleton.h"
#include "animal/AmClient.h"

namespace animal {
	class AmClientMgr final : public shynet::Nocopy {
		friend class shynet::Singleton<AmClientMgr>;
	public:
		AmClientMgr();
		~AmClientMgr();

		void add(int k, std::shared_ptr<AmClient> v);
		bool remove(int k);
		std::shared_ptr<AmClient> find(int k);
		std::shared_ptr<AmClient> find(int64_t phone);
		std::unordered_map<int, std::shared_ptr<AmClient>> clis() const;

		const net::IPAddress& listen_addr() const;
		void listen_addr(const net::IPAddress& addr);
	public:
		std::unordered_map<int, std::string> config_data;
	private:
		net::IPAddress listen_addr_;
		mutable std::mutex clis_mutex_;
		std::unordered_map<int, std::shared_ptr<AmClient>> clis_;
	};
}
